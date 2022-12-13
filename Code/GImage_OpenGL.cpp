#include "GImage.h"
#ifdef __BIONIC__
#include <GLES2/gl2.h>
#include <cmath>


// These shaders are defined in GSystem_Android.cpp
extern GLint SHADER_XY;
extern GLint SHADER_RGBA;
extern GLint SHADER_UV;



struct GImage::Private {
	int width, height;
	GRect src, dst; // Temp data used by the last draw call
	GColor color; // Temp data used by the last draw call
	GLuint texture;
    std::vector<Vertex> vertices;
	GLuint vertexObject;
    std::vector<uint16_t> indices;
	GLuint indexObject;
	inline Private (): width(0), height(0), texture(0), vertexObject(0), indexObject(0) {}
	inline ~Private () { if(indexObject) glDeleteBuffers(1, &indexObject); if(vertexObject) glDeleteBuffers(1, &vertexObject); if(texture) glDeleteTextures(1, &texture); indexObject = 0; vertexObject = 0; texture = 0; }
};



// These are required to be here to satisfy the hidden struct that is pointed to by the unique_ptr
GImage::GImage (): _data(new Private) {}
GImage::GImage (const Resource& resource): _data(new Private) { New(resource); }
GImage::GImage (const GString& resource): _data(new Private) { New(resource); }
GImage::GImage (const GColor& color): _data(new Private) { New(color); }
GImage::~GImage () {}



bool GImage::New (const Resource& resource) {
	if(_data->indexObject)
		glDeleteBuffers(1, &_data->indexObject);
	if(_data->vertexObject)
		glDeleteBuffers(1, &_data->vertexObject);
	if(_data->texture)
		glDeleteTextures(1, &_data->texture);
	_data->width = 0;
	_data->height = 0;
	_data->texture = 0;
	_data->vertices.clear();
	_data->vertexObject = 0;
	_data->indices.clear();
	_data->indexObject = 0;

	if(resource.width == 0 || resource.height == 0 || resource.bufferSize <= 0 || resource.buffer == nullptr)
		return false;
	
	_data->width = resource.width;
	_data->height = resource.height;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glGenTextures(1, &_data->texture);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Required for non-power of 2 textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Required for non-power of 2 textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _data->width, _data->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, resource.buffer);

	glGenBuffers(1, &_data->vertexObject);
	glGenBuffers(1, &_data->indexObject);
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glEnableVertexAttribArray(SHADER_XY);
	glEnableVertexAttribArray(SHADER_RGBA);
	glEnableVertexAttribArray(SHADER_UV);

	return true;
}



int GImage::GetWidth () const {
	return _data->width;
}



int GImage::GetHeight () const {
	return _data->height;
}



GRect GImage::GetRect () const {
	return (GRect){0, 0, _data->width, _data->height};
}



bool GImage::IsEmpty () const {
	return _data->width == 0 || _data->height == 0;
}



void GImage::Draw () {
	Draw(GRect(0, 0, _data->width, _data->height), GRect(0, 0, _data->width, _data->height), GColor::WHITE);
}



void GImage::Draw (const GRect& src, const GRect& dst, const GColor& color) {
	if(!_data->texture)
		return;

	if(_data->indices.size() != 6) {
		_data->indices = {0, 1, 2, 1, 2, 3};
	}

	if(_data->vertices.size() != 4 || _data->src != src || _data->dst != dst || _data->color != color) {
		_data->src = src;
		_data->dst = dst;
		_data->color = color;
		_data->vertices.resize(4);
		_data->vertices[0].xy[0] = (float)_data->dst.x;
		_data->vertices[0].xy[1] = (float)_data->dst.y;
		_data->vertices[0].uv[0] = (float)_data->src.x / (float)_data->width;
		_data->vertices[0].uv[1] = (float)_data->src.y / (float)_data->height;
		_data->vertices[0].rgba[0] = color.GetRed();
		_data->vertices[0].rgba[1] = color.GetGreen();
		_data->vertices[0].rgba[2] = color.GetBlue();
		_data->vertices[0].rgba[3] = color.GetAlpha();
		_data->vertices[1].xy[0] = (float)(_data->dst.x + _data->dst.width);
		_data->vertices[1].xy[1] = (float)_data->dst.y;
		_data->vertices[1].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
		_data->vertices[1].uv[1] = (float)_data->src.y / (float)_data->height;
		_data->vertices[1].rgba[0] = color.GetRed();
		_data->vertices[1].rgba[1] = color.GetGreen();
		_data->vertices[1].rgba[2] = color.GetBlue();
		_data->vertices[1].rgba[3] = color.GetAlpha();
		_data->vertices[2].xy[0] = (float)_data->dst.x;
		_data->vertices[2].xy[1] = (float)(_data->dst.y + _data->dst.height);
		_data->vertices[2].uv[0] = (float)_data->src.x / (float)_data->width;
		_data->vertices[2].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
		_data->vertices[2].rgba[0] = color.GetRed();
		_data->vertices[2].rgba[1] = color.GetGreen();
		_data->vertices[2].rgba[2] = color.GetBlue();
		_data->vertices[2].rgba[3] = color.GetAlpha();
		_data->vertices[3].xy[0] = (float)(_data->dst.x + _data->dst.width);
		_data->vertices[3].xy[1] = (float)(_data->dst.y + _data->dst.height);
		_data->vertices[3].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
		_data->vertices[3].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
		_data->vertices[3].rgba[0] = color.GetRed();
		_data->vertices[3].rgba[1] = color.GetGreen();
		_data->vertices[3].rgba[2] = color.GetBlue();
		_data->vertices[3].rgba[3] = color.GetAlpha();
	}

	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->vertices.size(), _data->vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * _data->indices.size(), _data->indices.data(), GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glVertexAttribPointer(SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8_t) * 4));
	glDrawElements(GL_TRIANGLES, (GLsizei)_data->indices.size(), GL_UNSIGNED_SHORT, nullptr);
}



void GImage::DrawLine (const GPoint& a, const GPoint& b, int width, const GColor& color) {
	if(!_data->texture)
		return;

	if(_data->indices.size() != 6) {
		_data->indices = {0, 1, 2, 1, 2, 3};
	}

	GRect src(a.x, a.y, width, 0);
	GRect dst(b.x, b.y, width, 0);
	if(_data->vertices.size() != 4 || _data->src != src || _data->dst != dst || _data->color != color) {
		_data->src = src;
		_data->dst = dst;
		_data->color = color;
		_data->vertices.resize(4);
		float theta = std::atan2f((float)(b.y - a.y), (float)(b.x - a.x));
		float tsin = (float)width * 0.5f * std::sinf(theta);
		float tcos = (float)width * 0.5f * std::cosf(theta);
		_data->vertices[0].xy[0] = (float)a.x + tsin;
		_data->vertices[0].xy[1] = (float)a.y - tcos;
		_data->vertices[0].uv[0] = 0.0f;
		_data->vertices[0].uv[1] = 0.0f;
		_data->vertices[0].rgba[0] = color.GetRed();
		_data->vertices[0].rgba[1] = color.GetGreen();
		_data->vertices[0].rgba[2] = color.GetBlue();
		_data->vertices[0].rgba[3] = color.GetAlpha();
		_data->vertices[1].xy[0] = (float)a.x - tsin;
		_data->vertices[1].xy[1] = (float)a.y + tcos;
		_data->vertices[1].uv[0] = 1.0f;
		_data->vertices[1].uv[1] = 0.0f;
		_data->vertices[1].rgba[0] = color.GetRed();
		_data->vertices[1].rgba[1] = color.GetGreen();
		_data->vertices[1].rgba[2] = color.GetBlue();
		_data->vertices[1].rgba[3] = color.GetAlpha();
		_data->vertices[2].xy[0] = (float)b.x + tsin;
		_data->vertices[2].xy[1] = (float)b.y - tcos;
		_data->vertices[2].uv[0] = 0.0f;
		_data->vertices[2].uv[1] = 1.0f;
		_data->vertices[2].rgba[0] = color.GetRed();
		_data->vertices[2].rgba[1] = color.GetGreen();
		_data->vertices[2].rgba[2] = color.GetBlue();
		_data->vertices[2].rgba[3] = color.GetAlpha();
		_data->vertices[3].xy[0] = (float)b.x - tsin;
		_data->vertices[3].xy[1] = (float)b.y + tcos;
		_data->vertices[3].uv[0] = 1.0f;
		_data->vertices[3].uv[1] = 1.0f;
		_data->vertices[3].rgba[0] = color.GetRed();
		_data->vertices[3].rgba[1] = color.GetGreen();
		_data->vertices[3].rgba[2] = color.GetBlue();
		_data->vertices[3].rgba[3] = color.GetAlpha();
	}

	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->vertices.size(), _data->vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * _data->indices.size(), _data->indices.data(), GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glVertexAttribPointer(SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8_t) * 4));
	glDrawElements(GL_TRIANGLES, (GLsizei)_data->indices.size(), GL_UNSIGNED_SHORT, nullptr);
}



void GImage::DrawEllipse (const GRect& dst, const GColor& color, const int sides) {
	if(!_data->texture)
		return;

	if(_data->indices.size() != sides * 3 - 2) {
		_data->indices.resize(sides * 3 - 2);
		for(int i = 2; i < sides; i++) {
			_data->indices[(i - 2) * 3 + 0] = 0;
			_data->indices[(i - 2) * 3 + 1] = i - 1;
			_data->indices[(i - 2) * 3 + 2] = i;
		}
	}

	GRect src(0, 0, _data->width, _data->height);
	if(_data->vertices.size() != sides || _data->src != src || _data->dst != dst || _data->color != color) {
		_data->src = src;
		_data->dst = dst;
		_data->color = color;
		_data->vertices.resize(sides);
		for(int i = 0; i < sides; i++) {
			float theta = 2.0f * (float)M_PI * (float)i / (float)sides;
			float x = cosf(theta) * (float)dst.width * 0.5f;
			float y = sinf(theta) * (float)dst.height * 0.5f;
			_data->vertices[i].xy[0] = (float)dst.x + (float)dst.width * 0.5f + x;
			_data->vertices[i].xy[1] = (float)dst.y + (float)dst.height * 0.5f + y;
			_data->vertices[i].uv[0] = ((float)dst.width * 0.5f + x) / (float)dst.width;
			_data->vertices[i].uv[1] = ((float)dst.height * 0.5f + y) / (float)dst.height;
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->vertices.size(), _data->vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * _data->indices.size(), _data->indices.data(), GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glVertexAttribPointer(SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8_t) * 4));
	glDrawElements(GL_TRIANGLES, (GLsizei)_data->indices.size(), GL_UNSIGNED_SHORT, nullptr);
}



void GImage::DrawVertices (const std::vector<Vertex>& vertices_, const std::vector<uint16_t>& indices_) {
	if(!_data->texture)
		return;
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices_.size(), vertices_.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices_.size(), indices_.data(), GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glVertexAttribPointer(SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8_t) * 4));
	glDrawElements(GL_TRIANGLES, (GLsizei)indices_.size(), GL_UNSIGNED_SHORT, nullptr);
}



bool GImage::Resource::NewFromFile (const GString& path) {
	int64_t fileSize = GSystem::ResourceSizeFromFile(path);
	if(fileSize <= 0)
		return false;
	
	std::unique_ptr<uint8_t[]> fileBuffer(new uint8_t[fileSize]);
	if(!GSystem::ResourceReadFromFile(path, fileBuffer.get(), fileSize))
		return false;
	
	//CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)fileBuffer.get(), (CFIndex)fileSize, kCFAllocatorNull);
	//CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, nullptr);
	//CFRelease(data);
	//if(imageSource == nullptr)
	//	return false;
	
	//CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
	//CFRelease(imageSource);
	//if(image == nullptr)
	//	return false;
	
	//width = (int32_t)CGImageGetWidth(image);
	//height = (int32_t)CGImageGetHeight(image);
	//bufferSize = width * height * 4;
	//buffer = new uint8_t[bufferSize];
	//CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	//CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	//CGContextSetBlendMode(context, kCGBlendModeCopy);
	//CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
	//CGContextRelease(context);
	//CGColorSpaceRelease(colorSpace);
	//CGImageRelease(image);
	return true;
}



#endif // __BIONIC__
