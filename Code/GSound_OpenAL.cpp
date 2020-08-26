#include "GSound.h"
#if PLATFORM_MACOSX || PLATFORM_IOS || PLATFORM_WEB



struct GSound::_PrivateData {
	// Commented out for future reference if OpenAL becomes deprecated
	//AVAudioPlayer* player;
	ALuint buffer;
	ALuint source;
	
};



static ALCdevice *			_alDevice						= NULL;
static ALCcontext *			_alContext						= NULL;




//static void _AudioSessionInterruptionListener (void * inClientData, UInt32 inInterruptionState) {
//	switch(inInterruptionState) {
//	case kAudioSessionBeginInterruption:
//		if(_alContext) alcMakeContextCurrent(NULL);
//		break;
//	case kAudioSessionEndInterruption:
//		AudioSessionSetActive(true);
//		if(_alContext) alcMakeContextCurrent(_alContext);
//		break;
//	}
//}





class _GSoundEngine {
public:
	
	
	static void Startup () {
		
		// Startup the Audio Session //
		
		//AudioSessionInitialize(NULL, NULL, _AudioSessionInterruptionListener, NULL);
		
		//UInt32 iPodIsPlaying;
		//UInt32 size = sizeof(iPodIsPlaying);
		//AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &iPodIsPlaying);
		
		//UInt32 category = (iPodIsPlaying) ? kAudioSessionCategory_AmbientSound : kAudioSessionCategory_SoloAmbientSound;
		//AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
		
		//AudioSessionSetActive(true);
		
		static bool once = false;
		if(once)
			return;
		once = true;
		
		// Startup OpenAL //
		
		int major, minor;
		alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
		alcGetIntegerv(NULL, ALC_MINOR_VERSION, 1, &minor);

		//assert(major == 1);

		printf("ALC version: %i.%i\n", major, minor);
		printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
		
		
		// Open the device using the defaults
		_alDevice = alcOpenDevice(NULL);
		if(_alDevice) {
			// Create an audio context using the defaults
			_alContext = alcCreateContext(_alDevice, NULL);
			if(_alContext) {
				// Set the current context, this can take a few seconds
				alcMakeContextCurrent(_alContext);
			} else {
				GConsole::Print("Could not create OpenAL context! (%x)\n", alGetError());
				alcCloseDevice(_alDevice);
				_alDevice = NULL;
			}
		} else {
			GConsole::Print("Could not open OpenAL device! (%x)\n", alGetError());
		}
		
		printf("OpenAL version: %s\n", alGetString(AL_VERSION));
		printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
		printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));
		
		ALfloat listenerPos[] = {0.0, 0.0, 1.0};
		ALfloat listenerVel[] = {0.0, 0.0, 0.0};
		ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

		alListenerfv(AL_POSITION, listenerPos);
		alListenerfv(AL_VELOCITY, listenerVel);
		alListenerfv(AL_ORIENTATION, listenerOri);
		
		ALfloat volume;
		alGetListenerf(AL_GAIN, &volume);
		assert(volume == 1.0);
		alListenerf(AL_GAIN, 0.0);
		alGetListenerf(AL_GAIN, &volume);
		assert(volume == 0.0);
		alListenerf(AL_GAIN, 1.0f); // reset gain to default
		
	}
	
	~_GSoundEngine () {
		if(_alContext) {
			alcDestroyContext(_alContext);
			_alContext = NULL;
		}
		
		if(_alDevice) {
			alcCloseDevice(_alDevice);
			_alDevice = NULL;
		}
	}
	
	
};
static _GSoundEngine _ENGINE;



GSound::GSound ()
:	_data(NULL)
{
}

GSound::GSound (const Resource& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GConsole::Debug("ERROR: Could not load sound from resource!\n");
}

GSound::GSound (const GString& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GConsole::Debug("ERROR: Could not load sound \"%s\"!\n", (const char*)resource);
}

GSound::~GSound () {
	Delete();
}

bool GSound::New (const Resource& resource) {
	_GSoundEngine::Startup();
	
	Delete();
	
	_data = new _PrivateData;
	_data->buffer = 0;
	_data->source = 0;
	
	// TODO: Make sure the resource.buffer has at least X amount of bytes before continuing
	
	alGetError();
	
	unsigned offset = 12; // ignore the RIFF header
	offset += 8; // ignore the fmt header
	offset += 2; // ignore the format type
	
	unsigned channels = resource.buffer[offset + 1] << 8;
	channels |= resource.buffer[offset];
	offset += 2;
	printf("Channels: %u\n", channels);
	
	ALsizei frequency = resource.buffer[offset + 3] << 24;
	frequency |= resource.buffer[offset + 2] << 16;
	frequency |= resource.buffer[offset + 1] << 8;
	frequency |= resource.buffer[offset];
	offset += 4;
	printf("Frequency: %u\n", frequency);

	offset += 6; // ignore block size and bps

	unsigned bits = resource.buffer[offset + 1] << 8;
	bits |= resource.buffer[offset];
	offset += 2;
	printf("Bits: %u\n", bits);
	
	ALenum format = 0;
	if(bits == 8)
	{
	  if(channels == 1)
		format = AL_FORMAT_MONO8;
	  else if(channels == 2)
		format = AL_FORMAT_STEREO8;
	}
	else if(bits == 16)
	{
	  if(channels == 1)
		format = AL_FORMAT_MONO16;
	  else if(channels == 2)
		format = AL_FORMAT_STEREO16;
	}
	
	offset += 8; // ignore the data chunk

	printf("Start offset: %d\n", offset);

	alGenBuffers(1, &_data->buffer);
	assert(_data->buffer != 0);
	alBufferData(_data->buffer, format, resource.buffer + offset, (ALsizei)resource.bufferSize - (ALsizei)offset, frequency);
	ALenum error;
	if ((error = alGetError()) != AL_NO_ERROR)
		printf("%s\n", alGetString(error));
	
	ALint val;
	alGetBufferi(_data->buffer, AL_FREQUENCY, &val);
	assert(val == frequency);
	alGetBufferi(_data->buffer, AL_SIZE, &val);
	assert(val == (ALsizei)resource.bufferSize - offset);
	alGetBufferi(_data->buffer, AL_BITS, &val);
	assert(val == bits);
	alGetBufferi(_data->buffer, AL_CHANNELS, &val);
	assert(val == channels);
	
	alGenSources(1, &_data->source);
	assert(alIsSource(_data->source));
	
	alSourcei(_data->source, AL_BUFFER, _data->buffer);
	
	ALint state;
	alGetSourcei(_data->source, AL_SOURCE_STATE, &state);
	assert(state == AL_INITIAL); // TODO: Use this to get the state of the current audio (playing, stopped, etc)
	
	//alSourcef(_data->source, AL_REFERENCE_DISTANCE, 50.0f);
	//alSourcef(_source, AL_GAIN, (float)sound.volume / 255.0f);
	
	
	// Commented out for future reference if OpenAL becomes deprecated
	//NSError* error;
	//data->player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:resource.buffer length:resource.bufferSize] error:&error];
	//if(error != nil || _data->player == nil)
	//	return false;
	//[_data->player prepareToPlay];
	
	return true;
}

bool GSound::New (const GString& resource) {
	return New(Resource(resource));
}

void GSound::Delete () {
	if(_data) {
		delete _data;
		_data = NULL;
	}
}

void GSound::Play () {
	
	alSourcePlay(_data->source);
	ALint state;
	alGetSourcei(_data->source, AL_SOURCE_STATE, &state);
	assert(state == AL_PLAYING);
	
	
	
	// Commented out for future reference if OpenAL becomes deprecated
	//if(_data != NULL && _data->player != nil)
	//	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
	//		[_data->player play];
	//		[_data->player prepareToPlay];
	//	});
}

void GSound::Stop () {
	// Commented out for future reference if OpenAL becomes deprecated
	//if(_data != NULL && _data->player != nil)
	//	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
	//		[_data->player stop];
	//		[_data->player prepareToPlay];
	//	});
}

void GSound::Pause () {
	// Commented out for future reference if OpenAL becomes deprecated
	//if(_data != NULL && _data->player != nil)
	//	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
	//		[_data->player pause];
	//		[_data->player prepareToPlay];
	//	});
}

bool GSound::IsPlaying () {
	return false;
	
	// Commented out for future reference if OpenAL becomes deprecated
	//return _data != NULL ? [_data->player isPlaying] : false;
}

void GSound::Startup () {
	_GSoundEngine::Startup();
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
		GConsole::Debug("ERROR: Could not load sound resource \"%s\"!\n", (const char*)resource);
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
	//NSData* data = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:resource]];
	//if(data == nil)
	//	return false;
	//bufferSize = [data length];
	//buffer = new uint8[bufferSize];
	//[data getBytes:buffer length:bufferSize];
	//return true;
	
	// Maybe IMG_Load for Web
	
	GFile file(resource);
	if(file.IsOpen() == false)
		return false;
	bufferSize = file.GetSize();
	buffer = new uint8[bufferSize];
	return file.Read(buffer, bufferSize);
}

bool GSound::Resource::NewFromPackage (const GString& resource) {
	Delete();
	
	uint64 archiveSize = GPackage::GetSize(resource + ".sound");
	if(archiveSize == 0)
		return false;
	
	uint8* archiveBuffer = new uint8[archiveSize];
	if(GPackage::Read(resource + ".sound", archiveBuffer, archiveSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	uint64 headerSize = sizeof(bufferSize);
	memcpy(this, archiveBuffer, headerSize);
	
	buffer = new uint8[bufferSize];
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
	uint64 headerSize = sizeof(bufferSize);
	uint64 archiveSize = GArchive::GetBufferBounds(headerSize + sizeof(uint8) * bufferSize);
	
	uint8* archiveBuffer = new uint8[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	archiveSize = GArchive::Compress(buffer, sizeof(uint8) * bufferSize, archiveBuffer + headerSize, archiveSize - headerSize);
	
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

#endif // PLATFORM_MACOSX || PLATFORM_IOS || PLATFORM_WEB
