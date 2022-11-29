#include "GImage.h"
#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

// These are defined in GSystem_Apple.mm
extern id<MTLDevice>				DEVICE;
extern id<MTLRenderCommandEncoder>	RENDER;



struct GImage::_PrivateData {
	int width, height;
	
	// This is temp data set by the last draw call, use width and height instead of src and dst
	// when getting information about the texture
	// if src, dst, color, verticiesCount, or indiciesCount change, then data may be reset
	GRect src, dst;
	GColor color;
	
	id<MTLTexture> texture;
	Vertex* vertices;
	NSUInteger verticesCount;
	id<MTLBuffer> indicies;
	NSUInteger indiciesCount;
};





GImage::GImage ()
:	_data(nullptr)
{
}

GImage::GImage (const Resource& resource)
:	_data(nullptr)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load image from resource!\n");
}

GImage::GImage (const GString& resource)
:	_data(nullptr)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load image \"%s\"!\n", (const char*)resource);
}

GImage::GImage (const GColor& color)
:	_data(nullptr)
{
	if(New(color) == false)
		GSystem::Debug("ERROR: Could not create image with color 0x%x!\n", color.color);
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
	_data->texture = nil;
	_data->vertices = nullptr;
	_data->verticesCount = 0;
	_data->indicies = nil;
	_data->indiciesCount = 0;
	
	MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
	textureDescriptor.textureType = MTLTextureType2D;
	textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
	textureDescriptor.width = resource.width;
	textureDescriptor.height = resource.height;
	_data->texture = [DEVICE newTextureWithDescriptor:textureDescriptor];
	if(_data->texture == nil) {
		Delete();
		return false;
	}
	
	[_data->texture replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)resource.width, (NSUInteger)resource.height) mipmapLevel:0 withBytes:resource.buffer bytesPerRow:(NSUInteger)(resource.width * 4)];
	
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
	resource.buffer = new uint8_t[resource.bufferSize];
	for(int i = 0; i < resource.bufferSize; i += 4) {
		resource.buffer[i + 0] = color.GetRed();
		resource.buffer[i + 1] = color.GetGreen();
		resource.buffer[i + 2] = color.GetBlue();
		resource.buffer[i + 3] = color.GetAlpha();
	}
	return New(resource);
}

void GImage::Delete () {
	if(_data) {
		_data->texture = nil;
		if(_data->vertices)
			delete [] _data->vertices;
		_data->indicies = nil;
		delete _data;
		_data = nullptr;
	}
}

int GImage::GetWidth () const {
	return _data ? _data->width : 0;
}

int GImage::GetHeight () const {
	return _data ? _data->height : 0;
}

GRect GImage::GetRect () const {
	return _data ? GRect(0, 0, _data->width, _data->height) : GRect();
}

bool GImage::IsEmpty () const {
	return _data == nullptr;
}

void GImage::Draw () {
	if(_data != nullptr)
		return Draw(GRect(0, 0, _data->width, _data->height), GRect(0, 0, _data->width, _data->height), GColor::WHITE);
}

void GImage::Draw (const GRect& src, const GRect& dst, const GColor& color) {
	if(RENDER == nil || _data == nullptr || _data->texture == nil)
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
		uint16_t indicies[6] = {0, 1, 2, 1, 2, 3};
		_data->indiciesCount = 6;
		_data->indicies = [DEVICE newBufferWithBytes:indicies length:sizeof(indicies) options:MTLResourceStorageModeShared];
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
		for(int i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	[RENDER setVertexBytes:_data->vertices length:(sizeof(Vertex) * _data->verticesCount) atIndex:0];
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
}

void GImage::DrawLine (const GPoint& a, const GPoint& b, int width, const GColor& color) {
	if(RENDER == nil || _data == nullptr || _data->texture == nil)
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
		uint16_t indicies[6] = {
			0, 1, 2, 1, 2, 3
		};
		_data->indiciesCount = 6;
		_data->indicies = [DEVICE newBufferWithBytes:indicies length:sizeof(indicies) options:MTLResourceStorageModeShared];
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
		for(int i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	[RENDER setVertexBytes:_data->vertices length:(sizeof(Vertex) * _data->verticesCount) atIndex:0];
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
}

void GImage::DrawEllipse (const GRect& dst, const GColor& color, const int sides) {
	if(RENDER == nil || _data == nullptr || _data->texture == nil)
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
		_data->indiciesCount = sides * 3 - 2;
		_data->indicies = [DEVICE newBufferWithLength:(sizeof(uint16_t) * _data->indiciesCount) options:MTLResourceStorageModeShared];
		uint16_t* indicies = (uint16_t*)_data->indicies.contents;
		for(int i = 2; i < sides; i++) {
			indicies[(i - 2) * 3 + 0] = 0;
			indicies[(i - 2) * 3 + 1] = i - 1;
			indicies[(i - 2) * 3 + 2] = i;
		}
	}
	
	GRect src(0, 0, _data->width, _data->height);
	if(_data->src != src || _data->dst != dst) {
		_data->src = src;
		_data->dst = dst;
		for(int i = 0; i < sides; i++) {
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
		for(int i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	[RENDER setVertexBytes:_data->vertices length:(sizeof(Vertex) * _data->verticesCount) atIndex:0];
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
}

void GImage::DrawQuad (const float vertices[8], const float coords[8], const GColor& color) {
	if(RENDER == nil || _data == nullptr || _data->texture == nil)
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
		uint16_t indicies[6] = {
			0, 1, 2, 1, 2, 3
		};
		_data->indiciesCount = 6;
		_data->indicies = [DEVICE newBufferWithBytes:indicies length:sizeof(indicies) options:MTLResourceStorageModeShared];
	}
	
	for(int i = 0; i < 4; i++) {
		_data->vertices[i].xy[0] = vertices[i * 2 + 0];
		_data->vertices[i].xy[1] = vertices[i * 2 + 1];
		_data->vertices[i].uv[0] = coords[i * 2 + 0];
		_data->vertices[i].uv[1] = coords[i * 2 + 1];
	}
	
	if(_data->color != color) {
		_data->color = color;
		for(int i = 0; i < _data->verticesCount; i++) {
			_data->vertices[i].rgba[0] = color.GetRed();
			_data->vertices[i].rgba[1] = color.GetGreen();
			_data->vertices[i].rgba[2] = color.GetBlue();
			_data->vertices[i].rgba[3] = color.GetAlpha();
		}
	}
	
	[RENDER setVertexBytes:_data->vertices length:(sizeof(Vertex) * _data->verticesCount) atIndex:0];
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
}

void GImage::DrawVertices (const Vertex verticies[], int verticesCount, const uint16_t indicies[], int indiciesCount) {
	if(RENDER == nil || _data == nullptr || _data->texture == nil)
		return;
	_data->indicies = [DEVICE newBufferWithBytes:indicies length:(sizeof(uint16_t) * indiciesCount) options:MTLResourceStorageModeShared];
	[RENDER setVertexBytes:verticies length:(sizeof(Vertex) * verticesCount) atIndex:0];
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
}








GImage::Resource::Resource ()
:	width(0)
,	height(0)
,	bufferSize(0)
,	buffer(nullptr)
{
}

GImage::Resource::Resource (const GString& resource)
:	width(0)
,	height(0)
,	bufferSize(0)
,	buffer(nullptr)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load image resource \"%s\"!\n", (const char*)resource);
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
	
	CFStringRef string = CFStringCreateWithCString(nullptr, resource, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, string, kCFURLPOSIXPathStyle, false);
	CGImageSourceRef imageSource = CGImageSourceCreateWithURL(url, nullptr);
	CFRelease(string);
	CFRelease(url);
	if(imageSource == nullptr)
		return false;
	
	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
	CFRelease(imageSource);
	if(image == nullptr)
		return false;
	
	width = (uint32_t)CGImageGetWidth(image);
	height = (uint32_t)CGImageGetHeight(image);
	bufferSize = width * height * 4;
	buffer = new uint8_t[bufferSize];
	memset(buffer, 0, sizeof(uint8_t) * bufferSize);
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	CGContextSetBlendMode(context, kCGBlendModeCopy);
	
	//CGContextTranslateCTM(context, (CGFloat)0, (CGFloat)height);
	//CGContextScaleCTM(context, (CGFloat)1, (CGFloat)-1);
	CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
	
	CGContextRelease(context);
	CGColorSpaceRelease(colorSpace);
	CGImageRelease(image);
	
	return true;
}

bool GImage::Resource::NewFromFileInMemory (void* resource, int size) {
	Delete();
	
	@autoreleasepool {
		CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)resource, (CFIndex)size, kCFAllocatorNull);
		CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, nullptr);
		CFRelease(data);
		if(imageSource == nullptr)
			return false;
		
		CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
		CFRelease(imageSource);
		if(image == nullptr)
			return false;
		
		width = (uint32_t)CGImageGetWidth(image);
		height = (uint32_t)CGImageGetHeight(image);
		bufferSize = width * height * 4;
		buffer = new uint8_t[bufferSize];
		memset(buffer, 0, sizeof(uint8_t) * bufferSize);
		
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
		CGContextSetBlendMode(context, kCGBlendModeCopy);
		
		//CGContextTranslateCTM(context, (CGFloat)0, (CGFloat)height);
		//CGContextScaleCTM(context, (CGFloat)1, (CGFloat)-1);
		CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
		
		CGContextRelease(context);
		CGColorSpaceRelease(colorSpace);
		CGImageRelease(image);
	} // @autoreleasepool
	
	return true;
}

bool GImage::Resource::NewFromPackage (const GString& resource) {
	Delete();
	
	uint64_t archiveSize = GPackage::GetSize(resource + ".image");
	if(archiveSize == 0)
		return false;
	
	uint8_t* archiveBuffer = new uint8_t[archiveSize];
	
	if(GPackage::Read(resource + ".image", archiveBuffer, archiveSize) == false) {
		delete [] archiveBuffer;
		return false;
	}
	
	uint64_t headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	
	memcpy(this, archiveBuffer, headerSize);
	
	buffer = new uint8_t[bufferSize];
	
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
		buffer = nullptr;
	}
}

bool GImage::Resource::WriteToPackage (GPackage& package, const GString& name) {
	
	uint64_t headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	uint64_t archiveSize = GArchive::GetBufferBounds(headerSize + sizeof(uint8_t) * bufferSize);
	
	uint8_t* archiveBuffer = new uint8_t[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	archiveSize = GArchive::Compress(buffer, sizeof(uint8_t) * bufferSize, archiveBuffer + headerSize, archiveSize - headerSize);
	
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

#endif // __APPLE__
