#include "GSound.h"
#if __APPLE__
#include "GSystem.h"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>



static ALCdevice* AL_DEVICE = nullptr;
static ALCcontext* AL_CONTEXT = nullptr;



struct GSound::Private {
	ALuint buffer;
	ALuint source;
	inline Private (): buffer(0), source(0) {}
	inline ~Private () { if(source) alDeleteSources(1, &source); if(buffer) alDeleteBuffers(1, &buffer); buffer = 0; source = 0; }
};



// Startup and Shutdown could be re-used in other classes such as GMusic to allow for complete self-encapsulation
void GSound::Startup () {
	if(AL_DEVICE && AL_CONTEXT)
		return;
	
	// If a context and device already exist, use them and return
	AL_CONTEXT = alcGetCurrentContext();
	if(AL_CONTEXT) {
		AL_DEVICE = alcGetContextsDevice(AL_CONTEXT);
		if(AL_DEVICE)
			return;
	}
	
	AL_DEVICE = alcOpenDevice(nullptr);
	if(AL_DEVICE == nullptr) {
		GSystem::Debug("Could not open OpenAL device! (%s)\n", alGetString(alGetError()));
		return;
	}
	
	AL_CONTEXT = alcCreateContext(AL_DEVICE, nullptr);
	if(AL_CONTEXT == nullptr) {
		GSystem::Debug("Could not create OpenAL context! (%s)\n", alGetString(alGetError()));
		alcCloseDevice(AL_DEVICE);
		AL_DEVICE = nullptr;
		return;
	}
	
	alcMakeContextCurrent(AL_CONTEXT);
	
	// Set the default listen position and volume
	ALfloat listenerPos[] = {0.0, 0.0, 1.0};
	ALfloat listenerVel[] = {0.0, 0.0, 0.0};
	ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alListenerf(AL_GAIN, 1.0f);
	
	// Add a shutdown callback to automatically shutdown at close
	GSystem::NewShutdownCallback(Shutdown);
}

void GSound::Shutdown () {
	if(AL_CONTEXT)
		alcDestroyContext(AL_CONTEXT);
	if(AL_DEVICE)
		alcCloseDevice(AL_DEVICE);
	AL_CONTEXT = nullptr;
	AL_DEVICE = nullptr;
}



// These are required to be here to satisfy the hidden struct that is pointed to by the unique_ptr
GSound::GSound (): _data(new Private) {}
GSound::GSound (const Resource& resource): _data(new Private) { New(resource); }
GSound::GSound (const GString& resource): _data(new Private) { New(resource); }
GSound::~GSound () {}



bool GSound::New (const Resource& resource) {
	Startup();
	if(_data->source)
		alDeleteSources(1, &_data->source);
	if(_data->buffer)
		alDeleteBuffers(1, &_data->buffer);
	_data->buffer = 0;
	_data->source = 0;
	
	// Not a WAV file because there is no header
	if(resource.bufferSize < 44)
		return false;
	
	ALsizei offset = 22; // Ignore up to the channels
	
	ALint channels = (resource.buffer[offset + 1] << 8);
	channels |= resource.buffer[offset];
	offset += 2;
	
	ALsizei frequency = resource.buffer[offset + 3] << 24;
	frequency |= resource.buffer[offset + 2] << 16;
	frequency |= resource.buffer[offset + 1] << 8;
	frequency |= resource.buffer[offset];
	offset += 4;

	offset += 6; // Ignore up to bit size

	ALint bits = resource.buffer[offset + 1] << 8;
	bits |= resource.buffer[offset];
	offset += 2;
	
	ALenum format = 0;
	if(bits == 8) {
		if(channels == 1)
			format = AL_FORMAT_MONO8;
		else if(channels == 2)
			format = AL_FORMAT_STEREO8;
	} else if(bits == 16) {
		if(channels == 1)
			format = AL_FORMAT_MONO16;
		else if(channels == 2)
			format = AL_FORMAT_STEREO16;
	}
	
	offset = 44; // Offset to the actual data
	
	alGenBuffers(1, &_data->buffer);
	if(_data->buffer == 0) {
		GSystem::Debug("Could not create OpenAL audio buffer! (%s)\n", alGetString(alGetError()));
		return false;
	}
	
	alBufferData(_data->buffer, format, resource.buffer + offset, (ALsizei)resource.bufferSize - offset, frequency);
	//if ((error = alGetError()) != AL_NO_ERROR)
	//	printf("%s\n", alGetString(error));
	
	alGenSources(1, &_data->source);
	if(_data->source == 0) {
		GSystem::Debug("Could not create OpenAL audio source! (%s)\n", alGetString(alGetError()));
		return false;
	}
	
	alSourcei(_data->source, AL_BUFFER, _data->buffer);
	
	return true;
}



void GSound::Play () {
	if(_data->source != 0)
		alSourcePlay(_data->source);
}



void GSound::Stop () {
	if(_data->source != 0)
		alSourceRewind(_data->source);
}



void GSound::Pause () {
	if(_data->source != 0)
		alSourcePause(_data->source);
}



bool GSound::IsPlaying () {
	if(_data->source != 0) {
		ALint state;
		alGetSourcei(_data->source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
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



#endif // __APPLE__
