#import "GSystem.h"
#ifdef __BIONIC__
#include <chrono>

static GRect			SCREEN_RECT(0, 0, 1280, 720);
static GRect			SAFE_RECT(0, 0, 1280, 720);
static GRect			PREFERRED_RECT(0, 0, 1280, 720);
static int				FPS = 60;
static int				ARG_C = 0;
static char**			ARG_V = nullptr;
static GMatrix32_4x4	MODEL_MATRIX;
static GMatrix32_4x4	PROJECTION_MATRIX;











GRect GSystem::GetScreenRect () {
	return SCREEN_RECT;
}

GRect GSystem::GetSafeRect () {
	return SAFE_RECT;
}

GRect GSystem::GetPreferredRect () {
	return PREFERRED_RECT;
}

int GSystem::GetFPS () {
	return FPS;
}

void GSystem::SetDefaultWD () {
	//CFURLRef directory = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	//char resources[PATH_MAX];
	//CFURLGetFileSystemRepresentation(directory, true, (UInt8*)resources, PATH_MAX);
	//CFRelease(directory);
	//chdir(resources);
}

const GString& GSystem::GetSaveDirectory () {
	static GString DIRECTORY;
	//if(DIRECTORY.IsEmpty()) {
	//	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	//	if([paths count] > 0)
	//		_DIRECTORY.New([[paths objectAtIndex:0] fileSystemRepresentation]);
	//}
	return DIRECTORY;
}




void GSystem::RunPreferredSize (int width, int height) {
	SCREEN_RECT.Set(0, 0, width, height);
	SAFE_RECT.Set(0, 0, width, height);
	PREFERRED_RECT.Set(0, 0, width, height);
}

void GSystem::RunPreferredFPS (int fps) {
	FPS = fps;
}

void GSystem::RunPreferredArgs (int argc, char* argv[]) {
	ARG_C = argc;
	ARG_V = argv;
}

int GSystem::Run () {
	GSystem::SetDefaultWD();
	GSystem::Debug("WD: %s\n", getwd(nullptr));

    // TODO: Run loop here
	
	return EXIT_SUCCESS;
}













// This is s good reference if matrix math is needed in the future
// https://developer.apple.com/library/content/samplecode/AdoptingMetalI/Listings/MetalTexturedMesh_AAPLMathUtilities_m.html#//apple_ref/doc/uid/TP40017287-MetalTexturedMesh_AAPLMathUtilities_m-DontLinkElementID_5

void GSystem::MatrixSetModelDefault () {
	MODEL_MATRIX.SetIdentity();
}

void GSystem::MatrixSetProjectionDefault () {
	PROJECTION_MATRIX.SetOrtho2D((float)SCREEN_RECT.x, (float)SCREEN_RECT.width, (float)SCREEN_RECT.height, (float)SCREEN_RECT.y, (float)-1, (float)1);
}

void GSystem::MatrixTranslateModel (float x, float y) {
	MODEL_MATRIX.SetTranslation((float)x, (float)y, (float)0);
}

void GSystem::MatrixTranslateProjection (float x, float y) {
	PROJECTION_MATRIX.SetTranslation((float)x, (float)y, (float)0);
}

void GSystem::MatrixScaleModel (float x, float y) {
	MODEL_MATRIX.SetScale((float)x, (float)y, (float)1);
}

void GSystem::MatrixScaleProjection (float x, float y) {
	PROJECTION_MATRIX.SetScale((float)x, (float)y, (float)1);
}

void GSystem::MatrixRotateModel (float degrees) {
	MODEL_MATRIX.SetRotation((float)degrees * ((float)M_PI / (float)180));
}

void GSystem::MatrixRotateProjection (float degrees) {
	PROJECTION_MATRIX.SetRotation((float)degrees * ((float)M_PI / (float)180));
}

void GSystem::MatrixUpdate () {
	//if(_RENDER != nil) {
	//	static GMatrix32_4x4 MATRIX;
	//	MATRIX = PROJECTION_MATRIX * MODEL_MATRIX;
	//	[_RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	//}
}

#endif // __BIONIC__
