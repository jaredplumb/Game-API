#include "GImage.h"
#if PLATFORM_WEB

// These shaders are defined in GSystem_Web.cpp
extern GLint					_SHADER_XY;
extern GLint					_SHADER_RGBA;
extern GLint					_SHADER_UV;


struct GImage::_PrivateData {
	
	int_t width, height;
	
	// This is temp data set by the last draw call, use width and height instead of src and dst
	// when getting information about the texture
	// if src, dst, color, verticiesCount, or indiciesCount change, then data may be reset
	GRect src, dst;
	GColor color;
	
	GLuint texture;
	Vertex* vertices;
	uint_t verticesCount;
	GLushort* indicies;
	uint_t indiciesCount;
	
	GLuint vertexObject;
	GLuint indexObject;
};





GImage::GImage ()
:	_data(NULL)
{
}

GImage::GImage (const Resource& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GConsole::Debug("ERROR: Could not load image from resource!\n");
}

GImage::GImage (const GString& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GConsole::Debug("ERROR: Could not load image \"%s\"!\n", (const char*)resource);
}

GImage::GImage (const GColor& color)
:	_data(NULL)
{
	if(New(color) == false)
		GConsole::Debug("ERROR: Could not create image with color 0x%x!\n", color.color);
}

GImage::~GImage () {
	Delete();
}


bool GImage::New (const Resource& resource) {
	Delete();
	
	if(resource.width == 0 || resource.height == 0)
		return false;
	
	_data = new _PrivateData;
	_data->width = resource.width;
	_data->height = resource.height;
	_data->texture = 0;
	_data->vertices = NULL;
	_data->verticesCount = 0;
	_data->indicies = NULL;
	_data->indiciesCount = 0;
	_data->vertexObject = 0;
	_data->indexObject = 0;
	
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
	glEnableVertexAttribArray(_SHADER_XY);
	glEnableVertexAttribArray(_SHADER_RGBA);
	glEnableVertexAttribArray(_SHADER_UV);
	
	return true;
}

bool GImage::New (const GString& resource) {
	return New(Resource(resource));
}

bool GImage::New (const GColor& color) {
	Resource resource;
	resource.width = 4;
	resource.height = 4;
	resource.bufferSize = resource.width * resource.height * 4;
	resource.buffer = new uint8[resource.bufferSize];
	for(int_t i = 0; i < resource.bufferSize; i += 4) {
		resource.buffer[i + 0] = color.GetRed();
		resource.buffer[i + 1] = color.GetGreen();
		resource.buffer[i + 2] = color.GetBlue();
		resource.buffer[i + 3] = color.GetAlpha();
	}
	return New(resource);
}

void GImage::Delete () {
	if(_data) {
		glDeleteBuffers(1, &_data->vertexObject);
		glDeleteBuffers(1, &_data->indexObject);
		glDeleteTextures(1, &_data->texture);
		if(_data->vertices)
			delete [] _data->vertices;
		if(_data->indicies)
			delete [] _data->indicies;
		delete _data;
		_data = NULL;
	}
}

int_t GImage::GetWidth () const {
	return _data ? _data->width : 0;
}

int_t GImage::GetHeight () const {
	return _data ? _data->height : 0;
}

GRect GImage::GetRect () const {
	return _data ? GRect(0, 0, _data->width, _data->height) : GRect();
}

bool GImage::IsEmpty () const {
	return _data == NULL;
}

void GImage::Draw () {
	if(_data != NULL)
		return Draw(GRect(0, 0, _data->width, _data->height), GRect(0, 0, _data->width, _data->height), GColor::WHITE);
}

void GImage::Draw (const GRect& src, const GRect& dst, const GColor& color) {
	if(_data == NULL || _data->texture == 0)
		return;
	
	if(_data->verticesCount != 4) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = 4;
		_data->vertices = new Vertex[4];
		_data->src = GRect();
		_data->dst = GRect();
		_data->color = 0;
	}
	
	if(_data->indiciesCount != 6) {
		if(_data->indicies)
			delete [] _data->indicies;
		_data->indiciesCount = 6;
		_data->indicies = new GLushort[6] {0, 1, 2, 1, 2, 3};
	}
	
	if(_data->src != src || _data->dst != dst) {
		_data->src = src;
		_data->dst = dst;
		_data->vertices[0].xy[0] = (float)_data->dst.x;
		_data->vertices[0].xy[1] = (float)_data->dst.y;
		_data->vertices[0].uv[0] = (float)_data->src.x / (float)_data->width;
		_data->vertices[0].uv[1] = (float)_data->src.y / (float)_data->height;
		_data->vertices[1].xy[0] = (float)(_data->dst.x + _data->dst.width);
		_data->vertices[1].xy[1] = (float)_data->dst.y;
		_data->vertices[1].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
		_data->vertices[1].uv[1] = (float)_data->src.y / (float)_data->height;
		_data->vertices[2].xy[0] = (float)_data->dst.x;
		_data->vertices[2].xy[1] = (float)(_data->dst.y + _data->dst.height);
		_data->vertices[2].uv[0] = (float)_data->src.x / (float)_data->width;
		_data->vertices[2].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
		_data->vertices[3].xy[0] = (float)(_data->dst.x + _data->dst.width);
		_data->vertices[3].xy[1] = (float)(_data->dst.y + _data->dst.height);
		_data->vertices[3].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
		_data->vertices[3].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
	}
	
	if(_data->color != color) {
		_data->color = color;
		for(int_t i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->verticesCount, _data->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(_SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(_SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(_SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8) * 4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * _data->indiciesCount, _data->indicies, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glDrawElements(GL_TRIANGLES, _data->indiciesCount, GL_UNSIGNED_SHORT, NULL);
}

void GImage::DrawLine (const GPoint& a, const GPoint& b, int_t width, const GColor& color) {
	if(_data == NULL || _data->texture == 0)
		return;
	
	if(_data->verticesCount != 4) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = 4;
		_data->vertices = new Vertex[4];
		_data->src = GRect();
		_data->dst = GRect();
		_data->color = 0;
	}
	
	if(_data->indiciesCount != 6) {
		if(_data->indicies)
			delete [] _data->indicies;
		_data->indiciesCount = 6;
		_data->indicies = new GLushort[6] {0, 1, 2, 1, 2, 3};
	}
	
	GRect src(a.x, a.y, width, 0);
	GRect dst(b.x, b.y, width, 0);
	if(_data->src != src || _data->dst != dst) {
		_data->src = src;
		_data->dst = dst;
		float theta = (float)atan2f((float)(b.y - a.y), (float)(b.x - a.x));
		float tsin = (float)width * 0.5f * (float)sinf(theta);
		float tcos = (float)width * 0.5f * (float)cosf(theta);
		_data->vertices[0].xy[0] = (float)a.x + tsin;
		_data->vertices[0].xy[1] = (float)a.y - tcos;
		_data->vertices[0].uv[0] = (float)0;
		_data->vertices[0].uv[1] = (float)0;
		_data->vertices[1].xy[0] = (float)a.x - tsin;
		_data->vertices[1].xy[1] = (float)a.y + tcos;
		_data->vertices[1].uv[0] = (float)1;
		_data->vertices[1].uv[1] = (float)0;
		_data->vertices[2].xy[0] = (float)b.x + tsin;
		_data->vertices[2].xy[1] = (float)b.y - tcos;
		_data->vertices[2].uv[0] = (float)0;
		_data->vertices[2].uv[1] = (float)1;
		_data->vertices[3].xy[0] = (float)b.x - tsin;
		_data->vertices[3].xy[1] = (float)b.y + tcos;
		_data->vertices[3].uv[0] = (float)1;
		_data->vertices[3].uv[1] = (float)1;
	}
	
	if(_data->color != color) {
		_data->color = color;
		for(int_t i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->verticesCount, _data->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(_SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(_SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(_SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8) * 4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * _data->indiciesCount, _data->indicies, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glDrawElements(GL_TRIANGLES, _data->indiciesCount, GL_UNSIGNED_SHORT, NULL);
}

void GImage::DrawEllipse (const GRect& dst, const GColor& color, const int_t sides) {
	if(_data == NULL || _data->texture == 0)
		return;
	
	if(_data->verticesCount != sides) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = sides;
		_data->vertices = new Vertex[_data->verticesCount];
		_data->src = GRect();
		_data->dst = GRect();
		_data->color = 0;
		_data->indiciesCount = 0;
	}
	
	if(_data->indiciesCount != sides * 3 - 2) {
		if(_data->indicies)
			delete [] _data->indicies;
		_data->indiciesCount = sides * 3 - 2;
		_data->indicies = new GLushort[_data->indiciesCount];
		for(int_t i = 2; i < sides; i++) {
			_data->indicies[(i - 2) * 3 + 0] = 0;
			_data->indicies[(i - 2) * 3 + 1] = i - 1;
			_data->indicies[(i - 2) * 3 + 2] = i;
		}
	}
	
	GRect src(0, 0, _data->width, _data->height);
	if(_data->src != src || _data->dst != dst) {
		_data->src = src;
		_data->dst = dst;
		for(int_t i = 0; i < sides; i++) {
			float theta = 2.0f * (float)M_PI * (float)i / (float)sides;
			float x = cosf(theta) * (float)dst.width * 0.5f;
			float y = sinf(theta) * (float)dst.height * 0.5f;
			_data->vertices[i].xy[0] = (float)dst.x + (float)dst.width * 0.5f + x;
			_data->vertices[i].xy[1] = (float)dst.y + (float)dst.height * 0.5f + y;
			_data->vertices[i].uv[0] = ((float)dst.width * 0.5f + x) / (float)dst.width;
			_data->vertices[i].uv[1] = ((float)dst.height * 0.5f + y) / (float)dst.height;
		}
	}
	
	if(_data->color != color) {
		_data->color = color;
		for(int_t i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->verticesCount, _data->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(_SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(_SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(_SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8) * 4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * _data->indiciesCount, _data->indicies, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glDrawElements(GL_TRIANGLES, _data->indiciesCount, GL_UNSIGNED_SHORT, NULL);
}

void GImage::DrawQuad (const float vertices[8], const float coords[8], const GColor& color) {
	if(_data == NULL || _data->texture == 0)
		return;
	
	if(_data->verticesCount != 4) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = 4;
		_data->vertices = new Vertex[4];
		_data->src = GRect();
		_data->dst = GRect();
		_data->color = 0;
	}
	
	if(_data->indiciesCount != 6) {
		if(_data->indicies)
			delete [] _data->indicies;
		_data->indiciesCount = 6;
		_data->indicies = new GLushort[6] {0, 1, 2, 1, 2, 3};
	}
	
	for(int_t i = 0; i < 4; i++) {
		_data->vertices[i].xy[0] = vertices[i * 2 + 0];
		_data->vertices[i].xy[1] = vertices[i * 2 + 1];
		_data->vertices[i].uv[0] = coords[i * 2 + 0];
		_data->vertices[i].uv[1] = coords[i * 2 + 1];
	}
	
	if(_data->color != color) {
		_data->color = color;
		for(int_t i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _data->verticesCount, _data->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(_SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(_SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(_SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8) * 4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * _data->indiciesCount, _data->indicies, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glDrawElements(GL_TRIANGLES, _data->indiciesCount, GL_UNSIGNED_SHORT, NULL);
}

void GImage::DrawVertices (const Vertex verticies[], int_t verticesCount, const uint16 indicies[], int_t indiciesCount) {
	if(_data == NULL || _data->texture == 0)
		return;
	_data->verticesCount = 0;
	_data->indiciesCount = 0;
	glBindBuffer(GL_ARRAY_BUFFER, _data->vertexObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, verticies, GL_STATIC_DRAW);
	glVertexAttribPointer(_SHADER_XY, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	glVertexAttribPointer(_SHADER_RGBA, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const void*)(sizeof(float) * 2));
	glVertexAttribPointer(_SHADER_UV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(sizeof(float) * 2 + sizeof(uint8) * 4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _data->indexObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indiciesCount, indicies, GL_STATIC_DRAW);
	glBindTexture(GL_TEXTURE_2D, _data->texture);
	glDrawElements(GL_TRIANGLES, _data->indiciesCount, GL_UNSIGNED_SHORT, NULL);
}








GImage::Resource::Resource ()
:	width(0)
,	height(0)
,	bufferSize(0)
,	buffer(NULL)
{
}

GImage::Resource::Resource (const GString& resource)
:	width(0)
,	height(0)
,	bufferSize(0)
,	buffer(NULL)
{
	if(New(resource) == false)
		GConsole::Debug("ERROR: Could not load image resource \"%s\"!\n", (const char*)resource);
}

GImage::Resource::~Resource () {
	Delete();
}

bool GImage::Resource::New (const GString& resource) {
	if(NewFromPackage(resource))
		return true;
	if(NewFromFile(resource))
		return true;
	return false;
}

bool GImage::Resource::NewFromFile (const GString& resource) {
	Delete();
	
	GConsole::Print("Reading images from file is not yet implemented for Game API with Web!\n");
	
	//CFStringRef string = CFStringCreateWithCString(NULL, resource, kCFStringEncodingUTF8);
	//CFURLRef url = CFURLCreateWithFileSystemPath(NULL, string, kCFURLPOSIXPathStyle, false);
	//CGImageSourceRef imageSource = CGImageSourceCreateWithURL(url, NULL);
	//CFRelease(string);
	//CFRelease(url);
	//if(imageSource == NULL)
	//	return false;
	
	//CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	//CFRelease(imageSource);
	//if(image == NULL)
	//	return false;
	
	//width = (uint32)CGImageGetWidth(image);
	//height = (uint32)CGImageGetHeight(image);
	//bufferSize = width * height * 4;
	//buffer = new uint8[bufferSize];
	//memset(buffer, 0, sizeof(uint8) * bufferSize);
	
	//CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	//CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	//CGContextSetBlendMode(context, kCGBlendModeCopy);
	
	////CGContextTranslateCTM(context, (CGFloat)0, (CGFloat)height);
	////CGContextScaleCTM(context, (CGFloat)1, (CGFloat)-1);
	//CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
	
	//CGContextRelease(context);
	//CGColorSpaceRelease(colorSpace);
	//CGImageRelease(image);
	
	return false;
}

bool GImage::Resource::NewFromFileInMemory (void* resource, int_t size) {
	Delete();
	
	GConsole::Print("Reading images from memory is not yet implemented for Game API with Web!\n");
	
	//CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)resource, (CFIndex)size, kCFAllocatorNull);
	//CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, NULL);
	//CFRelease(data);
	//if(imageSource == NULL)
	//	return false;
	
	//CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	//CFRelease(imageSource);
	//if(image == NULL)
	//	return false;
	
	//width = (uint32)CGImageGetWidth(image);
	//height = (uint32)CGImageGetHeight(image);
	//bufferSize = width * height * 4;
	//buffer = new uint8[bufferSize];
	//memset(buffer, 0, sizeof(uint8) * bufferSize);
	
	//CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	//CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	//CGContextSetBlendMode(context, kCGBlendModeCopy);
	
	////CGContextTranslateCTM(context, (CGFloat)0, (CGFloat)height);
	////CGContextScaleCTM(context, (CGFloat)1, (CGFloat)-1);
	//CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
	
	//CGContextRelease(context);
	//CGColorSpaceRelease(colorSpace);
	//CGImageRelease(image);
	
	return false;
}

bool GImage::Resource::NewFromPackage (const GString& resource) {
	Delete();
	
	uint64 archiveSize = GPackage::GetSize(resource + ".image");
	if(archiveSize == 0)
		return false;
	
	uint8* archiveBuffer = new uint8[archiveSize];
	
	if(GPackage::Read(resource + ".image", archiveBuffer, archiveSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	uint64 headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	
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

void GImage::Resource::Delete () {
	width = 0;
	height = 0;
	bufferSize = 0;
	if(buffer) {
		delete [] buffer;
		buffer = NULL;
	}
}

bool GImage::Resource::WriteToPackage (GPackage& package, const GString& name) {
	
	uint64 headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	uint64 archiveSize = GArchive::GetBufferBounds(headerSize + sizeof(uint8) * bufferSize);
	
	uint8* archiveBuffer = new uint8[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	archiveSize = GArchive::Compress(buffer, sizeof(uint8) * bufferSize, archiveBuffer + headerSize, archiveSize - headerSize);
	
	if(archiveSize == 0) {
		delete [] archiveBuffer;
		return false;
	}
	
	if(package.Write(name + ".image", archiveBuffer, archiveSize + headerSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	delete [] archiveBuffer;
	return true;
}

#endif // PLATFORM_WEB
