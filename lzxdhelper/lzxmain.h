#include <cstring>
#include "mspack/mspack.h"
#include "mspack/lzx.h"

#define BLOCK_MAGIC				0xAE

typedef unsigned char			byte;

struct CBlockHeader
{
	byte			magic;
	byte			offset;
	byte			align;
	int				blockSize;
};

struct mspack_file
{
	byte* buf;
	int			bufSize;
	int			pos;
	int			rest;
};

// Align integer or pointer of any type
template<class T> inline T Align(const T ptr, int alignment)
{
	return (T)(((size_t)ptr + alignment - 1) & ~(alignment - 1));
}

// Using size_t typecasts - that's platform integer type
template<class T> inline T OffsetPointer(const T ptr, int offset)
{
	return (T)((size_t)ptr + offset);
}

static int mspack_read(mspack_file* file, void* buffer, int bytes)
{
	if (!file->rest)
	{
		// read block header
		if (file->buf[file->pos] == 0xFF)
		{
			// [0]   = FF
			// [1,2] = uncompressed block size
			// [3,4] = compressed block size
			file->rest = (file->buf[file->pos + 3] << 8) | file->buf[file->pos + 4];
			file->pos += 5;
		}
		else
		{
			// [0,1] = compressed size
			file->rest = (file->buf[file->pos + 0] << 8) | file->buf[file->pos + 1];
			file->pos += 2;
		}
		if (file->rest > file->bufSize - file->pos)
			file->rest = file->bufSize - file->pos;
	}
	if (bytes > file->rest) bytes = file->rest;
	if (bytes <= 0) return 0;

	// copy block data
	memcpy(buffer, file->buf + file->pos, bytes);
	file->pos += bytes;
	file->rest -= bytes;

	return bytes;
}

static int mspack_write(mspack_file* file, void* buffer, int bytes)
{
	memcpy(file->buf + file->pos, buffer, bytes);
	file->pos += bytes;
	return bytes;
}

void* appMalloc(int size, int alignment = 8, bool noInit = false)
{
	// Allocate memory
	void* block = malloc(size + sizeof(CBlockHeader) + (alignment - 1));
	if (!block)
	{
		// OUT OF MEMORY!
		return NULL; // Will crash
	}

	// Initialize the allocated block
	void* ptr = Align(OffsetPointer(block, sizeof(CBlockHeader)), alignment);
	if (size > 0 && !noInit)
		memset(ptr, 0, size);

	// Prepare block header
	CBlockHeader* hdr = (CBlockHeader*)ptr - 1;
	byte offset = (byte*)ptr - (byte*)block;
	hdr->magic = BLOCK_MAGIC;
	hdr->offset = offset - 1;
	hdr->align = alignment - 1;
	hdr->blockSize = size;
	return ptr;
}

static void* mspack_alloc(mspack_system* self, size_t bytes)
{
	return appMalloc(bytes);
}

void appFree(void* ptr)
{
	CBlockHeader* hdr = (CBlockHeader*)ptr - 1;
	hdr->magic--;		// modify to any value
	int offset = hdr->offset + 1;
	void* block = OffsetPointer(ptr, -offset);
	free(block);
}

static void mspack_free(void* ptr)
{
	appFree(ptr);
}

static void mspack_copy(void* src, void* dst, size_t bytes)
{
	memcpy(dst, src, bytes);
}

static struct mspack_system lzxSys =
{
	NULL,				// open
	NULL,				// close
	mspack_read,
	mspack_write,
	NULL,				// seek
	NULL,				// tell
	NULL,				// message
	mspack_alloc,
	mspack_free,
	mspack_copy
};

static int appDecompressLZX(byte* CompressedBuffer, int CompressedSize, byte* UncompressedBuffer, int UncompressedSize)
{
	// setup streams
	mspack_file src, dst;
	src.buf = CompressedBuffer;
	src.bufSize = CompressedSize;
	src.pos = 0;
	src.rest = 0;
	dst.buf = UncompressedBuffer;
	dst.bufSize = UncompressedSize;
	dst.pos = 0;
	// prepare decompressor
	lzxd_stream* lzxd = lzxd_init(&lzxSys, &src, &dst, 17, 0, 256 * 1024, UncompressedSize);

	// decompress
	int r = lzxd_decompress(lzxd, UncompressedSize);
	// free resources
	lzxd_free(lzxd);
	return r;
}
