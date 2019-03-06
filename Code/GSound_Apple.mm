#include "GSound.h"
#if PLATFORM_MACOSX

struct GSound::_PrivateData {
	AVAudioPlayer* player;
};

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
	Delete();
	
	_data = new _PrivateData;
	
	NSError* error;
	_data->player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:resource.buffer length:resource.bufferSize] error:&error];
	if(error != nil || _data->player == nil)
		return false;
	
	[_data->player prepareToPlay];
	
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
	if(_data != NULL && _data->player != nil)
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
			[_data->player play];
			[_data->player prepareToPlay];
		});
}

void GSound::Stop () {
	if(_data != NULL && _data->player != nil)
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
			[_data->player stop];
			[_data->player prepareToPlay];
		});
}

void GSound::Pause () {
	if(_data != NULL && _data->player != nil)
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
			[_data->player pause];
			[_data->player prepareToPlay];
		});
}

bool GSound::IsPlaying () {
	return _data != NULL ? [_data->player isPlaying] : false;
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
	NSData* data = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:resource]];
	if(data == nil)
		return false;
	bufferSize = [data length];
	buffer = new uint8[bufferSize];
	[data getBytes:buffer length:bufferSize];
	return true;
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

#endif // PLATFORM_MACOSX
