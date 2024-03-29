#include "GImage.h"
#ifdef __APPLE__
#include <cmath>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>



// These are defined in GSystem_Apple.mm
extern id<MTLDevice> DEVICE;
extern id<MTLRenderCommandEncoder> RENDER;



struct GImage::Private {
	int width, height;
	GRect src, dst; // Temp data used by the last draw call
	GColor color; // Temp data used by the last draw call
	id<MTLTexture> texture;
	std::vector<Vertex> vertices;
	id<MTLBuffer> vertexObject;
	std::vector<uint16_t> indices;
	id<MTLBuffer> indexObject;
	inline Private (): width(0), height(0) {}
};



// These are required to be here to satisfy the hidden struct that is pointed to by the unique_ptr
GImage::GImage (): _data(new Private) {}
GImage::GImage (const Resource& resource): _data(new Private) { New(resource); }
GImage::GImage (const GString& resource): _data(new Private) { New(resource); }
GImage::GImage (const GColor& color): _data(new Private) { New(color); }
GImage::~GImage () {}



bool GImage::New (const Resource& resource) {
	_data->width = 0;
	_data->height = 0;
	_data->texture = nil;
	_data->vertices.clear();
	_data->vertexObject = nil;
	_data->indices.clear();
	_data->indexObject = nil;
	
	if(resource.width == 0 || resource.height == 0 || resource.bufferSize <= 0 || resource.buffer == nullptr)
		return false;
	
	_data->width = resource.width;
	_data->height = resource.height;
	
	MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
	textureDescriptor.textureType = MTLTextureType2D;
	textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
	textureDescriptor.width = resource.width;
	textureDescriptor.height = resource.height;
	_data->texture = [DEVICE newTextureWithDescriptor:textureDescriptor];
	if(_data->texture == nil)
		return false;
	
	[_data->texture replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)resource.width, (NSUInteger)resource.height) mipmapLevel:0 withBytes:resource.buffer bytesPerRow:(NSUInteger)(resource.width * 4)];
	
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
	if(RENDER == nil || _data->texture == nil)
		return;
	
	if(_data->indices.size() != 6) {
		_data->indices = {0, 1, 2, 1, 2, 3};
		_data->indexObject = [DEVICE newBufferWithBytes:_data->indices.data() length:(sizeof(uint16_t) * _data->indices.size()) options:MTLResourceStorageModeShared];
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
		_data->vertexObject = [DEVICE newBufferWithBytes:_data->vertices.data() length:(sizeof(Vertex) * _data->vertices.size()) options:MTLResourceStorageModeShared];
	}
	
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER setVertexBuffer:_data->vertexObject offset:0 atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indices.size() indexType:MTLIndexTypeUInt16 indexBuffer:_data->indexObject indexBufferOffset:0];
}



void GImage::DrawLine (const GPoint& a, const GPoint& b, int width, const GColor& color) {
	if(RENDER == nil || _data->texture == nil)
		return;
	
	if(_data->indices.size() != 6) {
		_data->indices = {0, 1, 2, 1, 2, 3};
		_data->indexObject = [DEVICE newBufferWithBytes:_data->indices.data() length:(sizeof(uint16_t) * _data->indices.size()) options:MTLResourceStorageModeShared];
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
		_data->vertexObject = [DEVICE newBufferWithBytes:_data->vertices.data() length:(sizeof(Vertex) * _data->vertices.size()) options:MTLResourceStorageModeShared];
	}
	
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER setVertexBuffer:_data->vertexObject offset:0 atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indices.size() indexType:MTLIndexTypeUInt16 indexBuffer:_data->indexObject indexBufferOffset:0];
}



void GImage::DrawEllipse (const GRect& dst, const GColor& color, const int sides) {
	if(RENDER == nil || _data->texture == nil)
		return;
	
	if(_data->indices.size() != sides * 3 - 2) {
		_data->indices.resize(sides * 3 - 2);
		for(int i = 2; i < sides; i++) {
			_data->indices[(i - 2) * 3 + 0] = 0;
			_data->indices[(i - 2) * 3 + 1] = i - 1;
			_data->indices[(i - 2) * 3 + 2] = i;
		}
		_data->indexObject = [DEVICE newBufferWithBytes:_data->indices.data() length:(sizeof(uint16_t) * _data->indices.size()) options:MTLResourceStorageModeShared];
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
		_data->vertexObject = [DEVICE newBufferWithBytes:_data->vertices.data() length:(sizeof(Vertex) * _data->vertices.size()) options:MTLResourceStorageModeShared];
	}
	
	[RENDER setFragmentTexture:_data->texture atIndex:0];
	[RENDER setVertexBuffer:_data->vertexObject offset:0 atIndex:0];
	[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indices.size() indexType:MTLIndexTypeUInt16 indexBuffer:_data->indexObject indexBufferOffset:0];
}



void GImage::DrawVertices (const std::vector<Vertex>& vertices_, const std::vector<uint16_t>& indices_) {
	if(RENDER != nil && _data->texture != nil) {
		_data->indexObject = [DEVICE newBufferWithBytes:indices_.data() length:(sizeof(uint16_t) * indices_.size()) options:MTLResourceStorageModeShared];
		_data->vertexObject = [DEVICE newBufferWithBytes:vertices_.data() length:(sizeof(Vertex) * vertices_.size()) options:MTLResourceStorageModeShared];
		[RENDER setFragmentTexture:_data->texture atIndex:0];
		[RENDER setVertexBuffer:_data->vertexObject offset:0 atIndex:0];
		[RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:indices_.size() indexType:MTLIndexTypeUInt16 indexBuffer:_data->indexObject indexBufferOffset:0];
	}
}



bool GImage::Resource::NewFromFile (const GString& path) {
	int64_t fileSize = GSystem::ResourceSizeFromFile(path);
	if(fileSize <= 0)
		return false;
	
	std::unique_ptr<uint8_t[]> fileBuffer(new uint8_t[fileSize]);
	if(!GSystem::ResourceReadFromFile(path, fileBuffer.get(), fileSize))
		return false;
	
	CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)fileBuffer.get(), (CFIndex)fileSize, kCFAllocatorNull);
	CGImageSourceRef imageSource = CGImageSourceCreateWithData(data, nullptr);
	CFRelease(data);
	if(imageSource == nullptr)
		return false;
	
	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
	CFRelease(imageSource);
	if(image == nullptr)
		return false;
	
	width = (int32_t)CGImageGetWidth(image);
	height = (int32_t)CGImageGetHeight(image);
	bufferSize = width * height * 4;
	buffer = new uint8_t[bufferSize];
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(buffer, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	CGContextSetBlendMode(context, kCGBlendModeCopy);
	CGContextDrawImage(context, CGRectMake((CGFloat)0, (CGFloat)0, (CGFloat)width, (CGFloat)height), image);
	CGContextRelease(context);
	CGColorSpaceRelease(colorSpace);
	CGImageRelease(image);
	return true;
}



#endif // __APPLE__
