#ifndef _GIMAGE_H_
#define _GIMAGE_H_

#include "GTypes.h"
#include "GSystem.h"
#include "GPackage.h"

class GImage {
public:
	class Resource;
	class Vertex;
	
	GImage ();
	GImage (const Resource& resource);
	GImage (const GString& resource);
	GImage (const GColor& color);
	~GImage ();
	
	bool New (const Resource& resource);
	bool New (const GString& resource);
	bool New (const GColor& color = GColor::WHITE);
	void Delete ();
	
	int_t GetWidth () const;
	int_t GetHeight () const;
	GRect GetRect () const; // Returns a rect using a 0,0 location and GetWdith and GetHeight (utility)
	bool IsEmpty () const;
	
	void Draw ();
	void Draw (const GRect& src, const GRect& dst, const GColor& color = GColor::WHITE);
	void DrawLine (const GPoint& a, const GPoint& b, int_t width, const GColor& color = GColor::WHITE);
	void DrawEllipse (const GRect& dst, const GColor& color = GColor::WHITE, const int_t sides = 45);
	
	// These are inline overload functions to allow for more drawing options
	inline void Draw (int_t x, int_t y, float alpha = 1.0f)							{ Draw(GetRect(), GetRect().Offset(x, y), GColor(0xff, 0xff, 0xff, (uint8)(alpha * 255.0f))); }
	inline void Draw (const GPoint& loc, float alpha = 1.0f)						{ Draw(loc.x, loc.y, alpha); }
	inline void Draw (const GRect& src, int_t x, int_t y, float alpha = 1.0f)		{ Draw(src, GRect(x, y, src.width, src.height), GColor(0xff, 0xff, 0xff, (uint8)(alpha * 255.0f))); }
	inline void Draw (const GRect& dst, float alpha = 1.0f)							{ Draw(GetRect(), dst, GColor(0xff, 0xff, 0xff, (uint8)(alpha * 255.0f))); }
	inline void DrawRect (const GRect& dst, const GColor& color = GColor::WHITE)	{ Draw(GetRect(), dst, color); }
	
	// DrawQuad will always reset the vertex data, and DrawVertices will always reset the vertex and index data
	void DrawQuad (const float vertices[8], const float coords[8], const GColor& color = GColor::WHITE);
	void DrawVertices (const Vertex verticies[], int_t verticesCount, const uint16 indicies[], int_t indiciesCount);
	
	class Resource {
	public:
		uint32 width;
		uint32 height;
		uint64 bufferSize;
		uint8* buffer;
		
		Resource ();
		Resource (const GString& resource);
		~Resource ();
		bool New (const GString& resource);
		bool NewFromFile (const GString& resource);
		bool NewFromFileInMemory (void* resource, int_t size);
		bool NewFromPackage (const GString& resource);
		void Delete ();
		bool WriteToPackage (GPackage& package, const GString& name);
	};
	
	class Vertex {
	public:
		float xy[2];
		uint8 rgba[4];
		float uv[2];
	};
	
private:
	struct _PrivateData;
	_PrivateData* _data;
};

#endif // _GIMAGE_H_
