#ifndef G_IMAGE_H_
#define G_IMAGE_H_

#include "GTypes.h"
#include "GSystem.h"
#include <cstdint>
#include <memory>

class GImage {
public:
	struct Vertex;
	struct Resource;
	
	GImage ();
	GImage (const Resource& resource);
	GImage (const GString& resource);
	GImage (const GColor& color);
	~GImage ();
	
	bool New (const Resource& resource);
	inline bool New (const GString& resource) { return New(Resource(resource)); }
	bool New (const GColor& color = GColor::WHITE);
	
	int GetWidth () const;
	int GetHeight () const;
	GRect GetRect () const; // Returns a rect using a 0,0 location and GetWdith and GetHeight (utility)
	bool IsEmpty () const;
	
	void Draw ();
	void Draw (const GRect& src, const GRect& dst, const GColor& color = GColor::WHITE);
	void DrawLine (const GPoint& a, const GPoint& b, int width, const GColor& color = GColor::WHITE);
	void DrawEllipse (const GRect& dst, const GColor& color = GColor::WHITE, const int sides = 45);
	void DrawVertices (const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
	
	// These are inline overload functions to allow for more drawing options
	inline void Draw (int x, int y, float alpha = 1.0f)								{ Draw(GetRect(), GetRect().Offset(x, y), GColor(0xff, 0xff, 0xff, (uint8_t)(alpha * 255.0f))); }
	inline void Draw (const GPoint& loc, float alpha = 1.0f)						{ Draw(loc.x, loc.y, alpha); }
	inline void Draw (const GRect& src, int x, int y, float alpha = 1.0f)			{ Draw(src, GRect(x, y, src.width, src.height), GColor(0xff, 0xff, 0xff, (uint8_t)(alpha * 255.0f))); }
	inline void Draw (const GRect& dst, float alpha = 1.0f)							{ Draw(GetRect(), dst, GColor(0xff, 0xff, 0xff, (uint8_t)(alpha * 255.0f))); }
	inline void DrawRect (const GRect& dst, const GColor& color = GColor::WHITE)	{ Draw(GetRect(), dst, color); }
	
	struct Vertex {
		float xy[2];
		uint8_t rgba[4];
		float uv[2];
	};
	
	struct Resource {
		int32_t width;
		int32_t height;
		int64_t bufferSize;
		uint8_t* buffer;
		inline Resource (): width(0), height(0), bufferSize(0), buffer(nullptr) {}
		inline Resource (const GString& name): width(0), height(0), bufferSize(0), buffer(nullptr) { New(name); }
		inline ~Resource () { width = 0; height = 0; bufferSize = 0; if(buffer) delete [] buffer; buffer = nullptr; }
		bool New (const GString& name);
		bool NewFromFile (const GString& path);
		bool Write (const GString& name);
	};
	
private:
	struct Private;
	std::unique_ptr<Private> _data;
};

#endif // G_IMAGE_H_
