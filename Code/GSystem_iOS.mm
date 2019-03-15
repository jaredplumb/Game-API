#import "GSystem.h"
#if PLATFORM_IOS

//float left = self.view.safeAreaInsets.left;
//float top = self.view.safeAreaInsets.top;
//float right = self.view.safeAreaInsets.right;
//float bottom = self.view.safeAreaInsets.bottom;
//printf("%f,%f,%f,%f\n", left, top, right, bottom);



static GRect			_RECT			(0, 0, 1280, 720);
static GRect			_SAFE_RECT		(0, 0, 1280, 720);
static int_t			_FPS		    = 60;
static int_t			_ARG_C			= 0;
static char**			_ARG_V			= NULL;

id<MTLDevice>				_P_DEVICE = nil;
id<MTLRenderCommandEncoder>	_P_RENDER = nil;
static matrix_float4x4		_MODEL_MATRIX;
static matrix_float4x4		_PROJECTION_MATRIX;




@interface _MyAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@interface _MyMetalView : MTKView <MTKViewDelegate>
@end







@implementation _MyAppDelegate

- (BOOL) application: (UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = [UIColor blackColor];
    //self.viewController = [[_MyViewController alloc] initWithNibName:nil bundle:nil];
    //self.window.rootViewController = self.viewController;
    [self.window.rootViewController setView:[[_MyMetalView alloc] initWithFrame:self.window.bounds]];
    [self.window makeKeyAndVisible];
    
    // TODO: this and the shutdown version should be moved to be called after OpenGL is turned on and right before it is turned off
    GSystem::RunStartupCallbacks();
    
    return YES;
}

- (void) applicationWillResignActive: (UIApplication*)application {
    //GSystem::RunPauseCallbacks();
}

- (void) applicationDidEnterBackground: (UIApplication*)application {
    //GSystem::RunDeactivateCallbacks();
}

- (void) applicationWillEnterForeground: (UIApplication*)application {
    //GSystem::RunActivateCallbacks();
}

- (void) applicationDidBecomeActive: (UIApplication*)application {
    //GSystem::RunResumeCallbacks();
}

- (void) applicationWillTerminate: (UIApplication*)application {
    // Run the shutdown callbacks before everything is turned off
    GSystem::RunShutdownCallbacks();
}

@end // _MyAppDelegate















@implementation _MyMetalView
{
	id<MTLCommandQueue> _commandQueue;
	id<MTLRenderPipelineState> _pipelineState;
	vector_uint2 _viewport;
}

- (id) initWithFrame: (CGRect)frameRect {
	
	// Convert the view to use the full pixel coordinates of the screen (retina)
	//frameRect = [[UIScreen mainScreen] convertRectToBacking:frameRect];
	
	_P_DEVICE = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame:frameRect device:_P_DEVICE];
	self.delegate = self;
	self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
	self.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
	[self mtkView:self drawableSizeWillChange:frameRect.size];
	
	id<MTLLibrary> defaultLibrary = [self.device newDefaultLibrary];
	id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"_VertexShader"];
	id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"_FragmentShader"];
	
	MTLRenderPipelineDescriptor* pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pipelineStateDescriptor.vertexFunction = vertexFunction;
	pipelineStateDescriptor.fragmentFunction = fragmentFunction;
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
	pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
	pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
	pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
	pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
	pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
	
	NSError* error = NULL;
	_pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
	_commandQueue = [self.device newCommandQueue];
	
	return self;
}

- (void) drawInMTKView:(nonnull MTKView*)view { 
	
	id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
	
	MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
	
	if(renderPassDescriptor != nil) {
		
		_P_RENDER = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[_P_RENDER setViewport:(MTLViewport){(double)0, (double)0, (double)_viewport.x, (double)_viewport.y, (double)-1, (double)1}];
		[_P_RENDER setRenderPipelineState:_pipelineState];
		
		GSystem::MatrixSetModelDefault();
		GSystem::MatrixSetProjectionDefault();
		GSystem::MatrixUpdate();
		
		GSystem::RunDrawCallbacks();
		
		[_P_RENDER endEncoding];
		
		[commandBuffer presentDrawable:view.currentDrawable];
		
	}
	
	[commandBuffer commit];
	
}

- (void) mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
	_viewport.x = size.width;
	_viewport.y = size.height;
}

- (void) encodeWithCoder:(nonnull NSCoder*)aCoder { 
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (BOOL) becomeFirstResponder {
	return YES;
}

- (BOOL) resignFirstResponder {
	return YES;
}

- (void) touchesBegan: (NSSet*)touches withEvent:(UIEvent*)event {
    for(UITouch* touch in touches) {
        CGPoint touchLocation = [touch locationInView:self];
        GSystem::RunTouchCallbacks((int_t)touchLocation.x, (int_t)touchLocation.y);
    }
}

- (void) touchesMoved: (NSSet*)touches withEvent:(UIEvent*)event {
    for(UITouch* touch in touches) {
        CGPoint touchLocation = [touch locationInView:self];
        GSystem::RunTouchMoveCallbacks((int_t)touchLocation.x, (int_t)touchLocation.y);
    }
}

- (void) touchesEnded: (NSSet*)touches withEvent:(UIEvent*)event {
    for(UITouch* touch in touches) {
        CGPoint touchLocation = [touch locationInView:self];
        GSystem::RunTouchUpCallbacks((int_t)touchLocation.x, (int_t)touchLocation.y);
    }
}

- (void) touchesCancelled: (NSSet*)touches withEvent:(UIEvent*)event {
    for(UITouch* touch in touches) {
        CGPoint touchLocation = [touch locationInView:self];
        GSystem::RunTouchUpCallbacks((int_t)touchLocation.x, (int_t)touchLocation.y);
    }
}

@end // _MyMetalView






















GRect GSystem::GetRect () {
	return _RECT;
}

GRect GSystem::GetSafeRect () {
	return _SAFE_RECT;
}

int_t GSystem::GetFPS () {
	return _FPS;
}

int_t GSystem::GetUniqueRef () {
	static int_t REF = 1;
	return REF++;
}

uint64 GSystem::GetMilliseconds () {
	return (uint64)(CFAbsoluteTimeGetCurrent() * 1000.0);
}

uint64 GSystem::GetMicroseconds () {
	return (uint64)(CFAbsoluteTimeGetCurrent() * 1000000.0);
}

uint64 GSystem::GetNanoseconds () {
	return (uint64)(CFAbsoluteTimeGetCurrent() * 1000000000.0);
}

void GSystem::SetDefaultWD () {
	CFURLRef directory = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	char resources[PATH_MAX];
	CFURLGetFileSystemRepresentation(directory, true, (UInt8*)resources, PATH_MAX);
	CFRelease(directory);
	chdir(resources);
}





void GSystem::RunPreferredSize (int_t width, int_t height) {
	_RECT = GRect(0, 0, width, height);
	_SAFE_RECT = GRect(0, 0, width, height);
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
	GConsole::Debug("WD: %s\n", getwd(NULL));
	
	@autoreleasepool {
        return UIApplicationMain((int)_ARG_C, _ARG_V, nil, NSStringFromClass([_MyAppDelegate class]));
	}
	
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
	_MODEL_MATRIX = matrix_identity_float4x4;
}

void GSystem::MatrixSetProjectionDefault () {
	//_EFFECT.transform.projectionMatrix = GLKMatrix4MakeOrtho((float)0, (float)_WIDTH, (float)_HEIGHT, (float)0, (float)-1, (float)1);
	_PROJECTION_MATRIX = matrix_ortho((float)0, (float)_RECT.width, (float)_RECT.height, (float)0, (float)-1, (float)1);
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
	if(_P_RENDER != nil) {
		static matrix_float4x4 MATRIX;
		MATRIX = matrix_multiply(_PROJECTION_MATRIX, _MODEL_MATRIX);
		[_P_RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	}
}

#endif // PLATFORM_MACOSX
