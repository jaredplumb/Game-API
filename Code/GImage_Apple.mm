#include "GImage.h"
#if PLATFORM_MACOSX











extern id<MTLDevice>				_P_DEVICE;
extern id<MTLRenderCommandEncoder>	_P_RENDER;



struct GImage::_PrivateData {
	
	int_t width, height;
	
	GRect src, dst;
	
	
	id<MTLTexture> texture;
	Vertex* vertices;
	NSUInteger verticesCount;
	id<MTLBuffer> indicies;
	NSUInteger indiciesCount;
	
};





GImage::GImage ()
:	_data(NULL)
{
	if(New(GColor::WHITE) == false)
		GSystem::Debug("ERROR: Could not create image with color 0xffffffff!\n");
}

GImage::GImage (const Resource& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load image from resource!\n");
}

GImage::GImage (const GString& resource)
:	_data(NULL)
{
	if(New(resource) == false)
		GSystem::Debug("ERROR: Could not load image \"%s\"!\n", (const char*)resource);
}

GImage::GImage (const GColor& color)
:	_data(NULL)
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
	_data->src.x = 0;
	_data->src.y = 0;
	_data->src.width = resource.width;
	_data->src.height = resource.height;
	_data->dst.x = 0;
	_data->dst.y = 0;
	_data->dst.width = resource.width;
	_data->dst.height = resource.height;
	_data->texture = nil;
	_data->vertices = NULL;
	_data->verticesCount = 0;
	_data->indicies = nil;
	_data->indiciesCount = 0;
	
	MTLTextureDescriptor* textureDescriptor = [[MTLTextureDescriptor alloc] init];
	textureDescriptor.textureType = MTLTextureType2D;
	textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
	textureDescriptor.width = resource.width;
	textureDescriptor.height = resource.height;
	_data->texture = [_P_DEVICE newTextureWithDescriptor:textureDescriptor];
	if(_data->texture == nil) {
		Delete();
		return false;
	}
	
	[_data->texture replaceRegion:MTLRegionMake2D(0, 0, (NSUInteger)resource.width, (NSUInteger)resource.height) mipmapLevel:0 withBytes:resource.buffer bytesPerRow:(NSUInteger)(resource.width * 4)];
	
	_data->verticesCount = 4;
	_data->vertices = new Vertex[4];
	
	_data->vertices[0].xy[0] = (float)_data->dst.x;
	_data->vertices[0].xy[1] = (float)_data->dst.y;
	_data->vertices[0].rgba[0] = (uint8)0xff;
	_data->vertices[0].rgba[1] = (uint8)0xff;
	_data->vertices[0].rgba[2] = (uint8)0xff;
	_data->vertices[0].rgba[3] = (uint8)0xff;
	_data->vertices[0].uv[0] = (float)_data->src.x / (float)_data->width;
	_data->vertices[0].uv[1] = (float)_data->src.y / (float)_data->height;
	_data->vertices[1].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[1].xy[1] = (float)_data->dst.y;
	_data->vertices[1].rgba[0] = (uint8)0xff;
	_data->vertices[1].rgba[1] = (uint8)0xff;
	_data->vertices[1].rgba[2] = (uint8)0xff;
	_data->vertices[1].rgba[3] = (uint8)0xff;
	_data->vertices[1].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
	_data->vertices[1].uv[1] = (float)_data->src.y / (float)_data->height;
	_data->vertices[2].xy[0] = (float)_data->dst.x;
	_data->vertices[2].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[2].rgba[0] = (uint8)0xff;
	_data->vertices[2].rgba[1] = (uint8)0xff;
	_data->vertices[2].rgba[2] = (uint8)0xff;
	_data->vertices[2].rgba[3] = (uint8)0xff;
	_data->vertices[2].uv[0] = (float)_data->src.x / (float)_data->width;
	_data->vertices[2].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
	_data->vertices[3].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[3].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[3].rgba[0] = (uint8)0xff;
	_data->vertices[3].rgba[1] = (uint8)0xff;
	_data->vertices[3].rgba[2] = (uint8)0xff;
	_data->vertices[3].rgba[3] = (uint8)0xff;
	_data->vertices[3].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
	_data->vertices[3].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
	
	uint16 indicies[6] = {
		0, 1, 2, 1, 2, 3
	};
	_data->indiciesCount = 6;
	_data->indicies = [_P_DEVICE newBufferWithBytes:indicies length:sizeof(indicies) options:MTLResourceStorageModeShared];
	if(_data->indicies == nil) {
		Delete();
		return false;
	}
	
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
		_data->texture = nil; // Autorelease ??? [_data->texture release]
		if(_data->vertices)
			delete [] _data->vertices;
		_data->indicies = nil; // Autorelease ???
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
	if(_P_RENDER == nil || _data == NULL || _data->vertices == NULL || _data->texture == nil || _data->indicies == nil)
		return;
	
	[_P_RENDER setVertexBytes:_data->vertices length:sizeof(Vertex) * _data->verticesCount atIndex:0];
	[_P_RENDER setFragmentTexture:_data->texture atIndex:0];
	[_P_RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
	
}

void GImage::Draw (const GRect& src, const GRect& dst, const GColor& color) {
	if(_data == NULL)
		return;
	
	//if(src == _data->src && dst == _data->dst && color.GetRed() == _data->vertices[0].rgba[0] && color.GetGreen() == _data->vertices[0].rgba[1] && color.GetBlue() == _data->vertices[0].rgba[2] && color.GetAlpha() == _data->vertices[0].rgba[3])
	//	return Draw();
	
	//if(_data->verticesCount != 4) {
	//	if(_data->vertices)
	//		delete [] _data->vertices;
	//	_data->verticesCount = 4;
	//	_data->vertices = new Vertex[4];
	//}
	_data->src = src;
	_data->dst = dst;
	
	
	_data->vertices[0].xy[0] = (float)_data->dst.x;
	_data->vertices[0].xy[1] = (float)_data->dst.y;
	_data->vertices[0].rgba[0] = color.GetRed();
	_data->vertices[0].rgba[1] = color.GetGreen();
	_data->vertices[0].rgba[2] = color.GetBlue();
	_data->vertices[0].rgba[3] = color.GetAlpha();
	_data->vertices[0].uv[0] = (float)_data->src.x / (float)_data->width;
	_data->vertices[0].uv[1] = (float)_data->src.y / (float)_data->height;
	_data->vertices[1].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[1].xy[1] = (float)_data->dst.y;
	_data->vertices[1].rgba[0] = color.GetRed();
	_data->vertices[1].rgba[1] = color.GetGreen();
	_data->vertices[1].rgba[2] = color.GetBlue();
	_data->vertices[1].rgba[3] = color.GetAlpha();
	_data->vertices[1].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
	_data->vertices[1].uv[1] = (float)_data->src.y / (float)_data->height;
	_data->vertices[2].xy[0] = (float)_data->dst.x;
	_data->vertices[2].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[2].rgba[0] = color.GetRed();
	_data->vertices[2].rgba[1] = color.GetGreen();
	_data->vertices[2].rgba[2] = color.GetBlue();
	_data->vertices[2].rgba[3] = color.GetAlpha();
	_data->vertices[2].uv[0] = (float)_data->src.x / (float)_data->width;
	_data->vertices[2].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
	_data->vertices[3].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[3].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[3].rgba[0] = color.GetRed();
	_data->vertices[3].rgba[1] = color.GetGreen();
	_data->vertices[3].rgba[2] = color.GetBlue();
	_data->vertices[3].rgba[3] = color.GetAlpha();
	_data->vertices[3].uv[0] = (float)(_data->src.x + _data->src.width) / (float)_data->width;
	_data->vertices[3].uv[1] = (float)(_data->src.y + _data->src.height) / (float)_data->height;
	
	Draw();
}

void GImage::Draw (int_t x, int_t y, float alpha) {
	if(_data == NULL)
		return;
	
	if(x == _data->dst.x && y == _data->dst.y && (uint8)(alpha * 255.0f) == _data->vertices[0].rgba[3])
		return Draw();
	
	_data->dst.x = x;
	_data->dst.y = y;
	
	_data->vertices[0].xy[0] = (float)_data->dst.x;
	_data->vertices[0].xy[1] = (float)_data->dst.y;
	_data->vertices[0].rgba[3] = (uint8)(alpha * 255.0f);
	_data->vertices[1].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[1].xy[1] = (float)_data->dst.y;
	_data->vertices[1].rgba[3] = (uint8)(alpha * 255.0f);
	_data->vertices[2].xy[0] = (float)_data->dst.x;
	_data->vertices[2].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[2].rgba[3] = (uint8)(alpha * 255.0f);
	_data->vertices[3].xy[0] = (float)(_data->dst.x + _data->dst.width);
	_data->vertices[3].xy[1] = (float)(_data->dst.y + _data->dst.height);
	_data->vertices[3].rgba[3] = (uint8)(alpha * 255.0f);
	Draw();
}

void GImage::Draw (const GRect& dst, float alpha) {
	if(_data)
		Draw(_data->src, dst, GColor(0xff, 0xff, 0xff, (uint8)(alpha * 255.0f)));
}

void GImage::Draw (const GRect& src, int_t x, int_t y, float alpha) {
	Draw(src, GRect(x, y, src.width, src.height), GColor(0xff, 0xff, 0xff, (uint8)(alpha * 255.0f)));
}

void GImage::DrawRect (const GRect& rect, const GColor& color) {
	if(_data)
		Draw(GRect(0, 0, _data->width, _data->height), rect, color);
}

void GImage::DrawLine (const GPoint& a, const GPoint& b, int_t width, const GColor& color) {
	if(_data == NULL || _data->vertices == NULL || _data->verticesCount == 0)
		return;
	
	if(a.x == _data->src.x && a.y == _data->src.y && b.x == _data->dst.x && b.y == _data->dst.y && width == _data->dst.width && 
	   color.GetRed() == _data->vertices[0].rgba[0] && color.GetGreen() == _data->vertices[0].rgba[1] && color.GetBlue() == _data->vertices[0].rgba[2] && color.GetAlpha() == _data->vertices[0].rgba[3])
		Draw();
	
	_data->src.x = a.x;
	_data->src.y = a.y;
	_data->dst.x = b.x;
	_data->dst.y = b.y;
	_data->dst.width = width;
	
	float theta = (float)atan2f((float)(b.y - a.y), (float)(b.x - a.x));
	float tsin = (float)width * 0.5f * (float)sinf(theta);
	float tcos = (float)width * 0.5f * (float)cosf(theta);
	
	_data->vertices[0].xy[0] = (float)a.x + tsin;
	_data->vertices[0].xy[1] = (float)a.y - tcos;
	_data->vertices[0].rgba[0] = color.GetRed();
	_data->vertices[0].rgba[1] = color.GetGreen();
	_data->vertices[0].rgba[2] = color.GetBlue();
	_data->vertices[0].rgba[3] = color.GetAlpha();
	_data->vertices[1].xy[0] = (float)a.x - tsin;
	_data->vertices[1].xy[1] = (float)a.y + tcos;
	_data->vertices[1].rgba[0] = color.GetRed();
	_data->vertices[1].rgba[1] = color.GetGreen();
	_data->vertices[1].rgba[2] = color.GetBlue();
	_data->vertices[1].rgba[3] = color.GetAlpha();
	_data->vertices[2].xy[0] = (float)b.x + tsin;
	_data->vertices[2].xy[1] = (float)b.y - tcos;
	_data->vertices[2].rgba[0] = color.GetRed();
	_data->vertices[2].rgba[1] = color.GetGreen();
	_data->vertices[2].rgba[2] = color.GetBlue();
	_data->vertices[2].rgba[3] = color.GetAlpha();
	_data->vertices[3].xy[0] = (float)b.x - tsin;
	_data->vertices[3].xy[1] = (float)b.y + tcos;
	_data->vertices[3].rgba[0] = color.GetRed();
	_data->vertices[3].rgba[1] = color.GetGreen();
	_data->vertices[3].rgba[2] = color.GetBlue();
	_data->vertices[3].rgba[3] = color.GetAlpha();
	
	Draw();
}

void GImage::DrawEllipse (const GRect& rect, const GColor& color, const int_t sides) {
	if(_data == NULL)
		return;
	
	if(_data->verticesCount != sides) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = sides;
		_data->vertices = new Vertex[_data->verticesCount];
	}
	
	
	if(_data->indiciesCount != sides * 3 - 2) {
		_data->indiciesCount = sides * 3 - 2;
		_data->indicies = [_P_DEVICE newBufferWithLength:(sizeof(uint16) * _data->indiciesCount) options:MTLResourceStorageModeShared];
		uint16* indicies = (uint16*)_data->indicies.contents;
		for(int_t i = 2; i < sides; i++) {
			indicies[(i - 2) * 3 + 0] = 0;
			indicies[(i - 2) * 3 + 1] = i - 1;
			indicies[(i - 2) * 3 + 2] = i;
		}
	}
	
	for(int_t i = 0; i < sides; i++) {
		float theta = 2.0f * (float)M_PI * (float)i / (float)sides;
		float x = cosf(theta) * (float)rect.width * 0.5f;
		float y = sinf(theta) * (float)rect.height * 0.5f;
		_data->vertices[i].xy[0] = (float)rect.x + (float)rect.width * 0.5f + x;
		_data->vertices[i].xy[1] = (float)rect.y + (float)rect.height * 0.5f + y;
		_data->vertices[i].rgba[0] = color.GetRed();
		_data->vertices[i].rgba[1] = color.GetGreen();
		_data->vertices[i].rgba[2] = color.GetBlue();
		_data->vertices[i].rgba[3] = color.GetAlpha();
		_data->vertices[i].uv[0] = ((float)rect.width * 0.5f + x) / (float)rect.width;
		_data->vertices[i].uv[1] = ((float)rect.height * 0.5f + y) / (float)rect.height;
	}
	
	Draw();
	
}

void GImage::DrawQuad (const float vertices[8], const float coords[8], const GColor& color) {
	if(_data == NULL)
		return;
	
	if(_data->verticesCount != 4) {
		if(_data->vertices)
			delete [] _data->vertices;
		_data->verticesCount = 4;
		_data->vertices = new Vertex[_data->verticesCount];
	}
	
	if(_data->indiciesCount != 6) {
		_data->indiciesCount = 6;
		_data->indicies = [_P_DEVICE newBufferWithLength:(sizeof(uint16) * _data->indiciesCount) options:MTLResourceStorageModeShared];
		uint16* indicies = (uint16*)_data->indicies.contents;
		indicies[0] = 0;
		indicies[1] = 1;
		indicies[2] = 2;
		indicies[3] = 1;
		indicies[4] = 2;
		indicies[5] = 3;
	}
	
	for(int_t i = 0; i < 4; i++) {
		_data->vertices[i].xy[0] = vertices[i * 2 + 0];
		_data->vertices[i].xy[1] = vertices[i * 2 + 1];
		_data->vertices[i].rgba[0] = color.GetRed();
		_data->vertices[i].rgba[1] = color.GetGreen();
		_data->vertices[i].rgba[2] = color.GetBlue();
		_data->vertices[i].rgba[3] = color.GetAlpha();
		_data->vertices[i].uv[0] = coords[i * 2 + 0];
		_data->vertices[i].uv[1] = coords[i * 2 + 1];
	}
	
	Draw();
	
}

void GImage::DrawVertices (const Vertex verticies[], int_t verticesCount, const uint16 indicies[], int_t indiciesCount) {
	if(_P_RENDER == nil || _data == NULL || _data->texture == nil)
		return;
	_data->indicies = [_P_DEVICE newBufferWithBytes:indicies length:(sizeof(uint16) * indiciesCount) options:MTLResourceStorageModeShared];
	[_P_RENDER setVertexBytes:verticies length:(sizeof(Vertex) * verticesCount) atIndex:0];
	[_P_RENDER setFragmentTexture:_data->texture atIndex:0];
	[_P_RENDER drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:indiciesCount indexType:MTLIndexTypeUInt16 indexBuffer:_data->indicies indexBufferOffset:0];
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
	
	CFStringRef string = CFStringCreateWithCString(NULL, resource, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, string, kCFURLPOSIXPathStyle, false);
	CGImageSourceRef imageSource = CGImageSourceCreateWithURL(url, NULL);
	CFRelease(string);
	CFRelease(url);
	if(imageSource == NULL)
		return false;
	
	CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
	CFRelease(imageSource);
	if(image == NULL)
		return false;
	
	width = (uint32)CGImageGetWidth(image);
	height = (uint32)CGImageGetHeight(image);
	bufferSize = width * height * 4;
	buffer = new uint8[bufferSize];
	memset(buffer, 0, sizeof(uint8) * bufferSize);
	
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

bool GImage::Resource::NewFromPackage (const GString& resource) {
	Delete();
	
	uint64 archiveSize = PPackage::GetSize(resource + ".image");
	if(archiveSize == 0)
		return false;
	
	uint8* archiveBuffer = new uint8[archiveSize];
	
	
	if(PPackage::Read(resource + ".image", archiveBuffer, archiveSize) == false) {
		//ERROR("Failed to read from package resource \"%s\"!", resource.GetString());
		delete [] archiveBuffer;
		return false;
	}
	
	uint64 headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	
	memcpy(this, archiveBuffer, headerSize);
	
	buffer = new uint8[bufferSize];
	
	archiveSize = PArchive::Decompress(archiveBuffer + headerSize, archiveSize - headerSize, buffer, bufferSize);
	
	if(archiveSize != bufferSize) {
		//ERROR("Failed to decompress resource \"%s\"!", resource.GetString());
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

bool GImage::Resource::WriteToPackage (PPackage& package, const GString& name) {
	
	uint64 headerSize = sizeof(width) + sizeof(height) + sizeof(bufferSize);
	uint64 archiveSize = PArchive::GetBufferBounds(headerSize + sizeof(uint8) * bufferSize);
	
	uint8* archiveBuffer = new uint8[archiveSize];
	memcpy(archiveBuffer, this, headerSize);
	
	archiveSize = PArchive::Compress(buffer, sizeof(uint8) * bufferSize, archiveBuffer + headerSize, archiveSize - headerSize);
	
	if(archiveSize == 0) {
		//ERROR("Failed to compress image resource \"%s\"!", resource.GetString());
		delete [] archiveBuffer;
		return false;
	}
	
	if(package.Write(name + ".image", archiveBuffer, archiveSize + headerSize) == false) {
		//ERROR("Failed to write to package resource \"%s\"!", resource.GetString());
		delete [] archiveBuffer;
		return false;
	}
	
	
	delete [] archiveBuffer;
	
	return true;
}






#endif // PLATFORM_MACOSX
