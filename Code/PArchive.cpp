#include "PArchive.h"
#include "PArchive_zlib.h"

enum _eCompressType {
	_COMPRESS_TYPE_ZLIB = 0,
};

static uint_t _ZLIBCompress (uint8* srcBuffer, uint_t srcSize, uint8* dstBuffer, uint_t dstSize, int level) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (unsigned int)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (unsigned int)dstSize;
	
	int error = deflateInit(&stream, level);
	if(error != Z_OK) return 0;
	
	error = deflate(&stream, Z_FINISH);
	if(error != Z_STREAM_END) return 0;
	
	dstSize = stream.total_out;
	
	error = deflateEnd(&stream);
	if(error != Z_OK) return 0;
	
	return dstSize;
}

static uint_t _ZLIBDecompress (uint8* srcBuffer, uint_t srcSize, uint8* dstBuffer, uint_t dstSize) {
	z_stream stream;
	memset(&stream, 0, sizeof(stream));
	stream.next_in = srcBuffer;
	stream.avail_in = (unsigned int)srcSize;
	stream.next_out = dstBuffer;
	stream.avail_out = (unsigned int)dstSize;
	
	int error = inflateInit(&stream);
	if(error != Z_OK) return 0;
	
	error = inflate(&stream, Z_FINISH);
	if(error != Z_STREAM_END) return 0;
	
	dstSize = stream.total_out;
	
	error = inflateEnd(&stream);
	if(error != Z_OK) return 0;
	
	return dstSize;
}

uint_t PArchive::Compress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize) {
	if(dstSize < 2)
		return 0;
	
	// Set the header data
	((uint8*)dstBuffer)[0] = VERSION;
	((uint8*)dstBuffer)[1] = (uint8)_COMPRESS_TYPE_ZLIB;
	dstBuffer = ((uint8*)dstBuffer) + 2;
	dstSize -= 2;
	
	// Max zlib compression
	dstSize = _ZLIBCompress((uint8*)srcBuffer, srcSize, (uint8*)dstBuffer, dstSize, 9);
	
	// Return the actual size of the compressed data
	if(dstSize == 0)
		return 0;
	return dstSize + 2;
}

uint_t PArchive::Decompress (const void* srcBuffer, uint_t srcSize, void* dstBuffer, uint_t dstSize) {
	if(srcSize < 2)
		return 0;
	
	// Check the version
	if(((uint8*)srcBuffer)[0] != VERSION) {
		//ERROR("Could not decompress archive of version %d!", ((uint8*)srcBuffer)[0]);
		return 0;
	}
	
	// Get the remainder of the header data
	_eCompressType compressType = (_eCompressType)((uint8*)srcBuffer)[1];
	srcBuffer = ((uint8*)srcBuffer) + 2;
	srcSize -= 2;
	
	// Decompress depending on the type
	switch(compressType) {
		case _COMPRESS_TYPE_ZLIB:
			return _ZLIBDecompress((uint8*)srcBuffer, srcSize, (uint8*)dstBuffer, dstSize);
		default:
			//ERROR("Could not decompress unknown compression type (%d)!", (int)compressType);
			return 0;
	}
	
	return 0;
}

uint_t PArchive::GetBufferBounds (uint_t srcSize) {
	// This is the size a dstBuffer needs to be to hold the headers when no compression happens.
	// This is taken from the function compressBound in the zlib library.
	// The last + number is what is needed for header values from GArchive.
	return srcSize + (srcSize >> 12) + (srcSize >> 14) + 11 + 2;
}
