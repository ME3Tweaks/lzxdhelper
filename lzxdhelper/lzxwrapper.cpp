#define _WIN32_WINNT 0x0501
#include <windows.h>
#include "lzxmain.h"
BOOL WINAPI DllMain(HINSTANCE hin, DWORD reason, LPVOID lpvReserved) { return TRUE; }

#define LZX_EXPORT __declspec(dllexport)
extern "C" {
	LZX_EXPORT int LZXDecompress(unsigned char* src, unsigned int src_len, unsigned char* dst, unsigned int* dst_len)
	{
		//int status =
		return appDecompressLZX(src, src_len, dst, *dst_len);
		//if (status == SZ_OK)
		//	*dst_len = len;

		//return status;
	}
}

// This wrapper won't support compression
//SEVENZIP_EXPORT int SevenZipCompress(int compression_level, unsigned char* src, unsigned int src_len, unsigned char* dst, unsigned int* dst_len)
//{
//	size_t len = *dst_len, propsSize = LZMA_PROPS_SIZE;
//
//	int status = LzmaCompress(&dst[LZMA_PROPS_SIZE], &len, src, src_len, dst, &propsSize, compression_level, 1 << 16, -1, -1, -1, -1, -1);
//	if (status == SZ_OK)
//		*dst_len = len + LZMA_PROPS_SIZE;
//
//	return status;
//}