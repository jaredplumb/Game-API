#include "PImage.h"
#if PLATFORM_WINDOWS

extern LPDIRECT3DDEVICE9 _DEVICE;

struct _CustomVertex {
	FLOAT x, y, z, w;
	D3DCOLOR color;
	FLOAT u, v;
};

static const DWORD _FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

struct PImage::_PrivateData {
	int_t refCount; // Used for the smart pointer code
	PString refName; // Used for the smart pointer code
	
	LPDIRECT3DVERTEXBUFFER9 vb;
	LPDIRECT3DTEXTURE9 tx;
	
	int_t imageWidth, imageHeight;
	int_t bufferWidth, bufferHeight;
	PRect src;
	PRect dst;
	PColor color;
};

static uint32 _MAX_TEXTURE_SIZE = 0;

bool PImage::New(const PImageResource& resource, const PString& name) {
	
	_data = _FindData(name);
	if (_data != NULL) {
		_data->refCount++;
		return true;
	}

	if (_MAX_TEXTURE_SIZE == 0) {
		if (_DEVICE != NULL) {
			D3DCAPS9 caps;
			_DEVICE->GetDeviceCaps(&caps);
			_MAX_TEXTURE_SIZE = caps.MaxTextureWidth < caps.MaxTextureHeight ? caps.MaxTextureWidth : caps.MaxTextureHeight;
		}
	}

	if (resource.bufferWidth > _MAX_TEXTURE_SIZE || resource.bufferHeight > _MAX_TEXTURE_SIZE)
		return New(PImageResource(resource, _MAX_TEXTURE_SIZE, _MAX_TEXTURE_SIZE));

	Delete();

	if (_DEVICE == NULL)
		return false;

	if (resource.bufferWidth == 0 || resource.bufferHeight == 0 || resource.bufferSize == 0 || resource.buffer == NULL)
		return false;

	if (_data == NULL)
		_data = new _PrivateData;

	_data->vb = NULL;
	_data->tx = NULL;
	_data->imageWidth = resource.width;
	_data->imageHeight = resource.height;
	_data->bufferWidth = resource.bufferWidth;
	_data->bufferHeight = resource.bufferHeight;
	_data->src.x = 0;
	_data->src.y = 0;
	_data->src.width = resource.width;
	_data->src.height = resource.height;
	_data->dst.x = 0;
	_data->dst.y = 0;
	_data->dst.width = resource.width;
	_data->dst.height = resource.height;
	_data->color = 0xffffffff;

	// Only use bufferWidth and bufferHeight when calculating uv values
	if (_data->bufferWidth < _data->imageWidth)
		_data->bufferWidth = _data->imageWidth;
	if (_data->bufferHeight < _data->imageHeight)
		_data->bufferHeight = _data->imageHeight;

	if (FAILED(_DEVICE->CreateVertexBuffer(sizeof(_CustomVertex)* 4, D3DUSAGE_WRITEONLY, _FVF, D3DPOOL_DEFAULT, &_data->vb, NULL))) {
		Delete();
		return false;
	}

	_CustomVertex* vertices;
	_data->vb->Lock(0, 0, (void**)&vertices, 0);
	vertices[0].x = (FLOAT)_data->dst.x;
	vertices[0].y = (FLOAT)_data->dst.y;
	vertices[0].z = (FLOAT)0;
	vertices[0].w = (FLOAT)1;
	vertices[0].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[0].u = (FLOAT)_data->src.x / (FLOAT)_data->bufferWidth;
	vertices[0].v = (FLOAT)_data->src.y / (FLOAT)_data->bufferHeight;
	vertices[1].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[1].y = (FLOAT)_data->dst.y;
	vertices[1].z = (FLOAT)0;
	vertices[1].w = (FLOAT)1;
	vertices[1].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[1].u = (FLOAT)(_data->src.x + _data->src.width) / (FLOAT)_data->bufferWidth;
	vertices[1].v = (FLOAT)_data->src.y / (FLOAT)_data->bufferHeight;
	vertices[2].x = (FLOAT)_data->dst.x;
	vertices[2].y = (FLOAT)(_data->dst.y + _data->dst.height);
	vertices[2].z = (FLOAT)0;
	vertices[2].w = (FLOAT)1;
	vertices[2].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[2].u = (FLOAT)_data->src.x / (FLOAT)_data->bufferWidth;
	vertices[2].v = (FLOAT)(_data->src.y + _data->src.height) / (FLOAT)_data->bufferHeight;
	vertices[3].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[3].y = (FLOAT)(_data->dst.y + _data->dst.height);
	vertices[3].z = (FLOAT)0;
	vertices[3].w = (FLOAT)1;
	vertices[3].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[3].u = (FLOAT)(_data->src.x + _data->src.width) / (FLOAT)_data->bufferWidth;
	vertices[3].v = (FLOAT)(_data->src.y + _data->src.height) / (FLOAT)_data->bufferHeight;
	_data->vb->Unlock();

	_DEVICE->CreateTexture(resource.bufferWidth, resource.bufferHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &_data->tx, NULL);
	if (_data->tx == NULL)
		return false;

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(_data->tx->LockRect(0, &lockedRect, NULL, 0))) {
		memcpy(lockedRect.pBits, resource.buffer, (uint_t)resource.bufferSize);
		_data->tx->UnlockRect(0);
	}

	_AddData(name, _data);

	return true;
}

void PImage::Delete() {
	if (_data) {
		
		_data->refCount--;
		if (_data->refCount > 0) {
			_data = NULL;
			return;
		}

		_RemoveData(_data->refName);
		if (_data->vb)
			_data->vb->Release();
		if (_data->tx)
			_data->tx->Release();
		delete _data;
		_data = NULL;
	}
}

int_t PImage::GetWidth() const {
	return _data ? _data->imageWidth : 0;
}

int_t PImage::GetHeight() const {
	return _data ? _data->imageHeight : 0;
}

int_t PImage::GetTextureWidth() const {
	return _data ? _data->bufferWidth : 0;
}

int_t PImage::GetTextureHeight() const {
	return _data ? _data->bufferHeight : 0;
}

bool PImage::IsEmpty() const {
	return _data == NULL || _data->vb == 0 || _data->tx == 0;
}

void PImage::Draw() {
	if (_DEVICE == NULL || _data == NULL || _data->vb == NULL)
		return;

	_DEVICE->SetTexture(0, _data->tx);
	_DEVICE->SetFVF(_FVF);
	_DEVICE->SetStreamSource(0, _data->vb, 0, sizeof(_CustomVertex));
	_DEVICE->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	_DEVICE->SetTexture(0, NULL);
}

void PImage::Draw(int_t x, int_t y) {
	if (_data == NULL || _data->vb == 0)
		return;

	if (_data->dst.x == x && _data->dst.y == y)
		return Draw();

	_data->dst.x = x;
	_data->dst.y = y;

	_CustomVertex* vertices;
	_data->vb->Lock(0, 0, (void**)&vertices, 0);
	vertices[0].x = (FLOAT)_data->dst.x;
	vertices[0].y = (FLOAT)_data->dst.y;
	vertices[1].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[1].y = (FLOAT)_data->dst.y;
	vertices[2].x = (FLOAT)_data->dst.x;
	vertices[2].y = (FLOAT)(_data->dst.y + _data->dst.height);
	vertices[3].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[3].y = (FLOAT)(_data->dst.y + _data->dst.height);
	_data->vb->Unlock();

	Draw();
}

void PImage::Draw(const PRect& src, int_t x, int_t y, float alpha) {
	if (_data == NULL || _data->vb == 0 || _data->tx == 0)
		return;

	if (_data->src == src && _data->dst.x == x && _data->dst.y == y && _data->color.GetAlpha() == (uint8)(alpha * 255.0f))
		return Draw();

	_data->src = src;
	_data->dst.x = x;
	_data->dst.y = y;
	_data->dst.width = src.width;
	_data->dst.height = src.height;
	_data->color.SetAlpha((uint8)(alpha * 255.0f));

	_CustomVertex* vertices;
	_data->vb->Lock(0, 0, (void**)&vertices, 0);
	vertices[0].x = (FLOAT)_data->dst.x;
	vertices[0].y = (FLOAT)_data->dst.y;
	vertices[0].z = (FLOAT)0;
	vertices[0].w = (FLOAT)1;
	vertices[0].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[0].u = (FLOAT)_data->src.x / (FLOAT)_data->bufferWidth;
	vertices[0].v = (FLOAT)_data->src.y / (FLOAT)_data->bufferHeight;
	vertices[1].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[1].y = (FLOAT)_data->dst.y;
	vertices[1].z = (FLOAT)0;
	vertices[1].w = (FLOAT)1;
	vertices[1].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[1].u = (FLOAT)(_data->src.x + _data->src.width) / (FLOAT)_data->bufferWidth;
	vertices[1].v = (FLOAT)_data->src.y / (FLOAT)_data->bufferHeight;
	vertices[2].x = (FLOAT)_data->dst.x;
	vertices[2].y = (FLOAT)(_data->dst.y + _data->dst.height);
	vertices[2].z = (FLOAT)0;
	vertices[2].w = (FLOAT)1;
	vertices[2].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[2].u = (FLOAT)_data->src.x / (FLOAT)_data->bufferWidth;
	vertices[2].v = (FLOAT)(_data->src.y + _data->src.height) / (FLOAT)_data->bufferHeight;
	vertices[3].x = (FLOAT)(_data->dst.x + _data->dst.width);
	vertices[3].y = (FLOAT)(_data->dst.y + _data->dst.height);
	vertices[3].z = (FLOAT)0;
	vertices[3].w = (FLOAT)1;
	vertices[3].color = D3DCOLOR_ARGB(_data->color.GetAlpha(), _data->color.GetRed(), _data->color.GetGreen(), _data->color.GetBlue());
	vertices[3].u = (FLOAT)(_data->src.x + _data->src.width) / (FLOAT)_data->bufferWidth;
	vertices[3].v = (FLOAT)(_data->src.y + _data->src.height) / (FLOAT)_data->bufferHeight;
	_data->vb->Unlock();

	Draw();
}

#endif // PLATFORM_WINDOWS
