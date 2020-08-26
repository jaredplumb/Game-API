#ifndef _GSOUND_H_
#define _GSOUND_H_

#include "GTypes.h"
#include "GSystem.h"
#include "GPackage.h"

// NOTE: To self contain GSound, the audio engine is turned on when accessed and shuts down 
// automatically upon exit of the application.  If there is a long pause accessing the first 
// audio, just create a dummy audio to turn on the engine.

class GSound {
public:
	class Resource;
	
	GSound ();
	GSound (const Resource& resource);
	GSound (const GString& resource);
	~GSound ();
	
	bool New (const Resource& resource);
	bool New (const GString& resource);
	void Delete ();
	
	void Play ();
	void Stop ();
	void Pause ();
	bool IsPlaying (); // Returns if the sound is playing, but may return false if called immediately after calling play due to the background thread
	
	static void Startup ();
	
	class Resource {
	public:
		uint64 bufferSize;
		uint8* buffer;
		Resource ();
		Resource (const GString& resource);
		~Resource ();
		bool New (const GString& resource);
		bool NewFromFile (const GString& resource);
		bool NewFromPackage (const GString& resource);
		void Delete ();
		bool WriteToPackage (GPackage& package, const GString& name);
	};
	
private:
	struct _PrivateData;
	_PrivateData* _data;
};

#endif // _GSOUND_H_
