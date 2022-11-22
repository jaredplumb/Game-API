#include "GSound.h"
#if defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>


static ALCdevice *		_AL_DEVICE		= NULL;
static ALCcontext *		_AL_CONTEXT		= NULL;


struct GSound::_PrivateData {
	ALuint buffer;
	ALuint source;
};


// Startup and Shutdown could be re-used in other classes such as GMusic to allow for complete self-encapsulation
void GSound::Startup () {
	
	// If already open and created do nothing
	if(_AL_DEVICE && _AL_CONTEXT)
		return;
	
	// Use a context and device if they already exist
	_AL_CONTEXT = alcGetCurrentContext();
	if(_AL_CONTEXT) {
		_AL_DEVICE = alcGetContextsDevice(_AL_CONTEXT);
		if(_AL_DEVICE)
			return;
	}
	
	// Open and create a new OpenAL devine and context
	_AL_DEVICE = alcOpenDevice(NULL);
	if(_AL_DEVICE) {
		_AL_CONTEXT = alcCreateContext(_AL_DEVICE, NULL);
		if(_AL_CONTEXT) {
			alcMakeContextCurrent(_AL_CONTEXT);
		} else {
			GSystem::Print("Could not create OpenAL context! (%s)\n", alGetString(alGetError()));
			alcCloseDevice(_AL_DEVICE);
			_AL_DEVICE = NULL;
			return;
		}
	} else {
		GSystem::Print("Could not open OpenAL device! (%s)\n", alGetString(alGetError()));
		return;
	}
	
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
	
	if(_AL_CONTEXT) {
		alcDestroyContext(_AL_CONTEXT);
		_AL_CONTEXT = NULL;
	}
	
	if(_AL_DEVICE) {
		alcCloseDevice(_AL_DEVICE);
		_AL_DEVICE = NULL;
	}
}

GSound::GSound ()
:	_data(NULL)
{
}

GSound::GSound (const Resource& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load sound from resource!\n");
}

GSound::GSound (const GString& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load sound \"%s\"!\n", (const char*)resource);
}

GSound::~GSound () {
	Delete();
}

bool GSound::New (const Resource& resource) {
	Startup();
	
	Delete();
	
	// Not a WAV file because there is no header
	if(resource.bufferSize < 44)
		return false;
	
	_data = new _PrivateData;
	_data->buffer = 0;
	_data->source = 0;
	
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

bool GSound::New (const GString& resource) {
	return New(Resource(resource));
}

void GSound::Delete () {
	if(_data) {
		if(_data->source != 0)
			alDeleteSources(1, &_data->source);
		if(_data->buffer != 0)
			alDeleteBuffers(1, &_data->buffer);
		delete _data;
		_data = NULL;
	}
}

void GSound::Play () {
	if(_data && _data->source != 0)
		alSourcePlay(_data->source);
}

void GSound::Stop () {
	if(_data && _data->source != 0)
		alSourceRewind(_data->source);
}

void GSound::Pause () {
	if(_data && _data->source != 0)
		alSourcePause(_data->source);
}

bool GSound::IsPlaying () {
	if(_data && _data->source != 0) {
		ALint state;
		alGetSourcei(_data->source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
	return false;
}

GSound::Resource::Resource ()
:	bufferSize(0)
,	buffer(NULL)
{
}

GSound::Resource::Resource (const GString& resource)
:	bufferSize(0)
,	buffer(NULL)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load sound resource \"%s\"!\n", (const char*)resource);
}

GSound::Resource::~Resource () {
	Delete();
}

bool GSound::Resource::New (const GString& resource) {
	if(NewFromPackage(resource))
		return true;
	if(NewFromFile(resource))
		return true;
	return false;
}

bool GSound::Resource::NewFromFile (const GString& resource) {
	Delete();
	
	GFile file(resource);
	if(file.IsOpen() == false)
		return false;
	bufferSize = file.GetSize();
	buffer = new uint8_t[bufferSize];
	return file.Read(buffer, bufferSize);
}

bool GSound::Resource::NewFromPackage (const GString& resource) {
	Delete();
	
	uint64_t archiveSize = GPackage::GetSize(resource + ".sound");
	if(archiveSize == 0)
		return false;
	
	uint8_t* archiveBuffer = new uint8_t[archiveSize];
	if(GPackage::Read(resource + ".sound", archiveBuffer, archiveSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	uint64_t headerSize = sizeof(bufferSize);
	memcpy(this, archiveBuffer, headerSize);
	
	buffer = new uint8_t[bufferSize];
	archiveSize = GArchive::Decompress(archiveBuffer + headerSize, archiveSize - headerSize, buffer, bufferSize);
	
	if(archiveSize != bufferSize) {
		delete [] archiveBuffer;
		return false;
	}
	
	delete [] archiveBuffer;
	return true;
}

void GSound::Resource::Delete () {
	bufferSize = 0;
	if(buffer) {
		delete [] buffer;
		buffer = NULL;
	}
}

bool GSound::Resource::WriteToPackage (GPackage& package, const GString& name) {
	uint64_t headerSize = sizeof(bufferSize);
	uint64_t archiveSize = GArchive::GetBufferBounds(headerSize + sizeof(uint8_t) * bufferSize);
	
	uint8_t* archiveBuffer = new uint8_t[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	archiveSize = GArchive::Compress(buffer, sizeof(uint8_t) * bufferSize, archiveBuffer + headerSize, archiveSize - headerSize);
	
	if(archiveSize == 0) {
		delete [] archiveBuffer;
		return false;
	}
	
	if(package.Write(name + ".sound", archiveBuffer, archiveSize + headerSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	delete [] archiveBuffer;
	return true;
}

#endif // defined(__APPLE__)
