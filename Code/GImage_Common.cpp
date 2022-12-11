#include "GImage.h"



bool GImage::New (const GColor& color) {
	Resource resource;
	resource.width = 4;
	resource.height = 4;
	resource.bufferSize = resource.width * resource.height * 4;
	resource.buffer = new uint8_t[resource.bufferSize];
	for(int i = 0; i < resource.bufferSize; i += 4) {
		resource.buffer[i + 0] = color.GetRed();
		resource.buffer[i + 1] = color.GetGreen();
		resource.buffer[i + 2] = color.GetBlue();
		resource.buffer[i + 3] = color.GetAlpha();
	}
	return New(resource);
}



bool GImage::Resource::New (const GString& name) {
	int64_t resourceSize = GSystem::ResourceSize(name + ".img");
	if(resourceSize <= sizeof(Resource))
		return false;
	
	std::unique_ptr<uint8_t[]> resourceBuffer(new uint8_t[resourceSize]);
	if(!GSystem::ResourceRead(name + ".img", resourceBuffer.get(), resourceSize))
		return false;
	
	int64_t offset = 0;
	width = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(width);
	height = *((int32_t*)(resourceBuffer.get() + offset));
	offset += sizeof(height);
	bufferSize = *((int64_t*)(resourceBuffer.get() + offset));
	offset += sizeof(bufferSize);
	if(bufferSize <= 0 || bufferSize * sizeof(uint8_t) > resourceSize - offset)
		return false;
	
	buffer = new uint8_t[bufferSize];
	memcpy(buffer, resourceBuffer.get() + offset, bufferSize * sizeof(uint8_t));
	return true;
}



bool GImage::Resource::Write (const GString& name) {
	int64_t resourceSize = sizeof(width) + sizeof(height) + sizeof(bufferSize) + bufferSize * sizeof(uint8_t);
	std::unique_ptr<uint8_t[]> resourceBuffer(new uint8_t[resourceSize]);
	int64_t offset = 0;
	*((int32_t*)(resourceBuffer.get() + offset)) = width;
	offset += sizeof(width);
	*((int32_t*)(resourceBuffer.get() + offset)) = height;
	offset += sizeof(height);
	*((int64_t*)(resourceBuffer.get() + offset)) = bufferSize;
	offset += sizeof(bufferSize);
	memcpy(resourceBuffer.get() + offset, buffer, bufferSize * sizeof(uint8_t));
	return GSystem::ResourceWrite(name + ".img", resourceBuffer.get(), resourceSize);
}
