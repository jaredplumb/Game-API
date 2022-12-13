#include "GSound.h"
#if __BIONIC__
#include "GSystem.h"



struct GSound::Private {
};



// Startup and Shutdown could be re-used in other classes such as GMusic to allow for complete self-encapsulation
void GSound::Startup () {
}

void GSound::Shutdown () {
}



// These are required to be here to satisfy the hidden struct that is pointed to by the unique_ptr
GSound::GSound (): _data(new Private) {}
GSound::GSound (const Resource& resource): _data(new Private) { New(resource); }
GSound::GSound (const GString& resource): _data(new Private) { New(resource); }
GSound::~GSound () {}



bool GSound::New (const Resource& resource) {
	return true;
}



void GSound::Play () {
}



void GSound::Stop () {
}



void GSound::Pause () {
}



bool GSound::IsPlaying () {
	return false;
}







bool GSound::Resource::New (const GString& name) {
	bufferSize = GSystem::ResourceSize(name + ".snd");
	if(bufferSize <= sizeof(Resource)) {
		bufferSize = 0;
		return false;
	}
	buffer = new uint8_t[bufferSize];
	if(!GSystem::ResourceRead(name + ".snd", buffer, bufferSize)) {
		delete [] buffer;
		buffer = nullptr;
		bufferSize = 0;
		return false;
	}
	return true;
}



bool GSound::Resource::NewFromFile (const GString& path) {
	bufferSize = GSystem::ResourceSizeFromFile(path);
	if(bufferSize <= 0) {
		bufferSize = 0;
		return false;
	}
	buffer = new uint8_t[bufferSize];
	if(!GSystem::ResourceReadFromFile(path, buffer, bufferSize)) {
		delete [] buffer;
		buffer = nullptr;
		bufferSize = 0;
		return false;
	}
	return true;
}



bool GSound::Resource::Write (const GString& name) {
	const int64_t resourceSize = sizeof(bufferSize) + bufferSize * sizeof(uint8_t);
	std::unique_ptr<uint8_t[]> resourceBuffer(new uint8_t[resourceSize]);
	int64_t offset = 0;
	*((int64_t*)(resourceBuffer.get() + offset)) = bufferSize;
	offset += sizeof(bufferSize);
	memcpy(resourceBuffer.get() + offset, buffer, bufferSize);
	return GSystem::ResourceWrite(name + ".snd", resourceBuffer.get(), resourceSize);
}



#endif // __BIONIC__
