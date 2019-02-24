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
		[_data->player play];
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
	return false;
}

void GSound::Resource::Delete () {
	bufferSize = 0;
	if(buffer) {
		delete [] buffer;
		buffer = NULL;
	}
}

bool GSound::Resource::WriteToPackage (GPackage& package, const GString& name) {
	return false;
}






#endif // PLATFORM_MACOSX
