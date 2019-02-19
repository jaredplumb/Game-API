#ifndef _P_IMAGE_H_
#define _P_IMAGE_H_

#include "PPlatform.h"
#include "PString.h"
#include "PSystem.h"
#include "PColor.h"
#include "PRect.h"
#include "PPackage.h"

class PImage {
public:
	class Resource;
	class Vertex;
	
	PImage ();
	PImage (const Resource& resource);
	PImage (const PString& resource);
	PImage (const PColor& color);
	~PImage ();
	
	bool New (const Resource& resource);
	bool New (const PString& resource);
	bool New (const PColor& color = PColor::WHITE);
	void Delete ();
	
	int_t GetWidth () const;
	int_t GetHeight () const;
	bool IsEmpty () const;
	
	void Draw ();
	void Draw (const PRect& src, const PRect& dst, const PColor& color = PColor::WHITE);
	void Draw (int_t x, int_t y, float alpha = 1.0f);
	void Draw (const PRect& dst, float alpha = 1.0f);
	void Draw (const PRect& src, int_t x, int_t y, float alpha = 1.0f);
	void DrawRect (const PRect& rect, const PColor& color = PColor::WHITE);
	void DrawLine (const PPoint& a, const PPoint& b, int_t width, const PColor& color = PColor::WHITE);
	void DrawEllipse (const PRect& rect, const PColor& color = PColor::WHITE, const int_t sides = 45);
	void DrawQuad (const float vertices[8], const float coords[8], const PColor& color = PColor::WHITE);
	void DrawVertices (const Vertex verticies[], int_t verticesCount, const uint16 indicies[], int_t indiciesCount);
	
	class Resource {
	public:
		uint32 width;
		uint32 height;
		uint64 bufferSize;
		uint8* buffer;
		
		Resource ();
		Resource (const PString& resource);
		~Resource ();
		bool New (const PString& resource);
		bool NewFromFile (const PString& resource);
		bool NewFromPackage (const PString& resource);
		void Delete ();
		bool WriteToPackage (PPackage& package, const PString& name);
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
