#import "GSystem.h"
#if PLATFORM_WEB

static GRect			_RECT			(0, 0, 1280, 720);
static GRect			_SAFE_RECT		(0, 0, 1280, 720);
static GRect			_PREFERRED_RECT	(0, 0, 1280, 720);
static int_t			_FPS		    = 60;
static int_t			_ARG_C			= 0;
static char**			_ARG_V			= NULL;


struct matrix_float4x4 {
	float columns[4][4];
};

static matrix_float4x4		_MODEL_MATRIX;
static matrix_float4x4		_PROJECTION_MATRIX;

















GRect GSystem::GetRect () {
	return _RECT;
}

GRect GSystem::GetSafeRect () {
	return _SAFE_RECT;
}

GRect GSystem::GetPreferredRect () {
	return _PREFERRED_RECT;
}

int_t GSystem::GetFPS () {
	return _FPS;
}

int_t GSystem::GetUniqueRef () {
	static int_t REF = 1;
	return REF++;
}

uint64 GSystem::GetMilliseconds () {
	return 0;
	//return (uint64)(CFAbsoluteTimeGetCurrent() * 1000.0);
}

uint64 GSystem::GetMicroseconds () {
	return 0;
	//return (uint64)(CFAbsoluteTimeGetCurrent() * 1000000.0);
}

uint64 GSystem::GetNanoseconds () {
	return 0;
	//return (uint64)(CFAbsoluteTimeGetCurrent() * 1000000000.0);
}

void GSystem::SetDefaultWD () {
	//CFURLRef directory = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	char resources[PATH_MAX] = "Resources/";
	//CFURLGetFileSystemRepresentation(directory, true, (UInt8*)resources, PATH_MAX);
	//CFRelease(directory);
	chdir(resources);
}





void GSystem::RunPreferredSize (int_t width, int_t height) {
	_PREFERRED_RECT = GRect(0, 0, width, height);
}

void GSystem::RunPreferredFPS (int_t fps) {
	_FPS = fps;
}

void GSystem::RunPreferredArgs (int_t argc, char* argv[]) {
	_ARG_C = argc;
	_ARG_V = argv;
}

int_t GSystem::Run () {
	GSystem::SetDefaultWD();
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	GConsole::Debug("WD: %s\n", cwd);
	
	
	
	// TODO Run stuff here
	
	
	
	return EXIT_SUCCESS;
}







// https://developer.apple.com/library/content/samplecode/AdoptingMetalI/Listings/MetalTexturedMesh_AAPLMathUtilities_m.html#//apple_ref/doc/uid/TP40017287-MetalTexturedMesh_AAPLMathUtilities_m-DontLinkElementID_5

static inline matrix_float4x4 matrix_make (float m00, float m10, float m20, float m30,
										   float m01, float m11, float m21, float m31,
										   float m02, float m12, float m22, float m32,
										   float m03, float m13, float m23, float m33) {
	return (matrix_float4x4){ {
		{ m00, m10, m20, m30 },
		{ m01, m11, m21, m31 },
		{ m02, m12, m22, m32 },
		{ m03, m13, m23, m33 } } };
}

static inline matrix_float4x4 matrix_ortho (float left, float right, float bottom, float top, float nearZ, float farZ) {
	return matrix_make(2 / (right - left), 0, 0, 0,
					   0, 2 / (top - bottom), 0, 0,
					   0, 0, 1 / (farZ - nearZ), 0,
					   (left + right) / (left - right), (top + bottom) / (bottom - top), nearZ / (nearZ - farZ), 1);
}

static inline matrix_float4x4 matrix4x4_translate (matrix_float4x4 matrix, float tx, float ty, float tz) {
	return (matrix_float4x4){ {
		{ matrix.columns[0][0], matrix.columns[0][1], matrix.columns[0][2], matrix.columns[0][3] },
		{ matrix.columns[1][0], matrix.columns[1][1], matrix.columns[1][2], matrix.columns[1][3] },
		{ matrix.columns[2][0], matrix.columns[2][1], matrix.columns[2][2], matrix.columns[2][3] },
		{ matrix.columns[0][0] * tx + matrix.columns[1][0] * ty + matrix.columns[2][0] * tz + matrix.columns[3][0], 
		  matrix.columns[0][1] * tx + matrix.columns[1][1] * ty + matrix.columns[2][1] * tz + matrix.columns[3][1], 
		  matrix.columns[0][2] * tx + matrix.columns[1][2] * ty + matrix.columns[2][2] * tz + matrix.columns[3][2], 
		  matrix.columns[0][3] * tx + matrix.columns[1][3] * ty + matrix.columns[2][3] * tz + matrix.columns[3][3] } } };
}



void GSystem::MatrixSetModelDefault () {
	//_EFFECT.transform.modelviewMatrix = GLKMatrix4Identity;
	//_MODEL_MATRIX = matrix_identity_float4x4;
}

void GSystem::MatrixSetProjectionDefault () {
	//_EFFECT.transform.projectionMatrix = GLKMatrix4MakeOrtho((float)0, (float)_WIDTH, (float)_HEIGHT, (float)0, (float)-1, (float)1);
	_PROJECTION_MATRIX = matrix_ortho((float)_RECT.x, (float)_RECT.width, (float)_RECT.height, (float)_RECT.y, (float)-1, (float)1);
}

void GSystem::MatrixTranslateModel (float x, float y) {
	//_EFFECT.transform.modelviewMatrix = GLKMatrix4Translate(_EFFECT.transform.modelviewMatrix, x, y, (float)0);
	_MODEL_MATRIX = matrix4x4_translate(_MODEL_MATRIX, x, y, (float)0);
}

void GSystem::MatrixTranslateProjection (float x, float y) {
	//_EFFECT.transform.projectionMatrix = GLKMatrix4Translate(_EFFECT.transform.projectionMatrix, x, y, (float)0);
	_PROJECTION_MATRIX = matrix4x4_translate(_PROJECTION_MATRIX, x, y, (float)0);
}

void GSystem::MatrixScaleModel (float x, float y) {
	//_EFFECT.transform.modelviewMatrix = GLKMatrix4Scale(_EFFECT.transform.modelviewMatrix, x, y, (float)1);
}

void GSystem::MatrixScaleProjection (float x, float y) {
	//_EFFECT.transform.projectionMatrix = GLKMatrix4Scale(_EFFECT.transform.projectionMatrix, x, y, (float)1);
}

void GSystem::MatrixRotateModel (float degrees) {
	//_EFFECT.transform.modelviewMatrix = GLKMatrix4Rotate(_EFFECT.transform.modelviewMatrix, GLKMathDegreesToRadians(degrees), (float)0, (float)0, (float)1);
}

void GSystem::MatrixRotateProjection (float degrees) {
	//_EFFECT.transform.projectionMatrix = GLKMatrix4Rotate(_EFFECT.transform.projectionMatrix, GLKMathDegreesToRadians(degrees), (float)0, (float)0, (float)1);
}

void GSystem::MatrixUpdate () {
	//if(_RENDER != nil) {
		//static matrix_float4x4 MATRIX;
		//MATRIX = matrix_multiply(_PROJECTION_MATRIX, _MODEL_MATRIX);
		//[_RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	//}
}

#endif // PLATFORM_WEB
