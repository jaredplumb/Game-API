#ifndef _P_ARCHIVE_H_
#define _P_ARCHIVE_H_

#include "GTypes.h"

class PArchive {
public:
	static const uint8 VERSION = 4;
	static uint_t Compress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize);
	static uint_t Decompress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize);
	static uint_t GetBufferBounds (uint_t srcSize);
};

#endif // _P_ARCHIVE_H_
