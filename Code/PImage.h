#ifndef _P_IMAGE_H_
#define _P_IMAGE_H_

#include "GTypes.h"
#include "PSystem.h"
#include "PPackage.h"

class PImage {
public:
	class Resource;
	class Vertex;
	
	PImage ();
	PImage (const Resource& resource);
	PImage (const GString& resource);
	PImage (const GColor& color);
	~PImage ();
	
	bool New (const Resource& resource);
	bool New (const GString& resource);
	bool New (const GColor& color = GColor::WHITE);
	void Delete ();
	
	int_t GetWidth () const;
	int_t GetHeight () const;
	bool IsEmpty () const;
	
	void Draw ();
	void Draw (const GRect& src, const GRect& dst, const GColor& color = GColor::WHITE);
	void Draw (int_t x, int_t y, float alpha = 1.0f);
	void Draw (const GRect& dst, float alpha = 1.0f);
	void Draw (const GRect& src, int_t x, int_t y, float alpha = 1.0f);
	void DrawRect (const GRect& rect, const GColor& color = GColor::WHITE);
	void DrawLine (const GPoint& a, const GPoint& b, int_t width, const GColor& color = GColor::WHITE);
	void DrawEllipse (const GRect& rect, const GColor& color = GColor::WHITE, const int_t sides = 45);
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
		bool NewFromPackage (const GString& resource);
		void Delete ();
		bool WriteToPackage (PPackage& package, const GString& name);
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

#endif // _P_IMAGE_H_
