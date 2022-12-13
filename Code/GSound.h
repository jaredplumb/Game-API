#ifndef G_SOUND_H_
#define G_SOUND_H_

#include "GTypes.h"
#include <memory>

// NOTE: To self contain GSound, the audio engine is turned on when accessed and shuts down 
// automatically upon exit of the application.  If there is a long pause accessing the first 
// audio, just create a dummy audio to turn on the engine.

// GSound only supports wav files.  Use the GMusic class for other file formats and streaming buffers

class GSound {
public:
	static void Startup ();
	static void Shutdown ();
	
	class Resource;
	
	GSound ();
	GSound (const Resource& resource);
	GSound (const GString& resource);
	~GSound ();
	
	bool New (const Resource& resource);
	inline bool New (const GString& resource) { return New(Resource(resource)); }
	
	void Play ();
	void Stop ();
	void Pause ();
	bool IsPlaying ();
	
	class Resource {
	public:
		uint64_t bufferSize;
		uint8_t* buffer;
		inline Resource (): bufferSize(0), buffer(nullptr) {}
		inline Resource (const GString& name): bufferSize(0), buffer(nullptr) { New(name); }
		inline ~Resource () { if(buffer) delete [] buffer; buffer = nullptr; }
		bool New (const GString& name);
		bool NewFromFile (const GString& path);
		bool Write (const GString& name);
	};
	
private:
	struct Private;
	std::unique_ptr<Private> _data;
};

#endif // G_SOUND_H_
