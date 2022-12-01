#import "GSystem.h"
#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <dirent.h>

static GRect					SCREEN_RECT;
static GRect					SAFE_RECT;
static GRect					PREFERRED_RECT;
static int						FPS		    = 60;
static int						ARG_C			= 0;
static char**					ARG_V			= nullptr;
id<MTLDevice>					DEVICE			= nil; // Non-static to allow for extern access from GImage
id<MTLRenderCommandEncoder>		RENDER			= nil; // Non-static to allow for extern access from GImage
static GMatrix32_4x4			MODEL_MATRIX;
static GMatrix32_4x4			PROJECTION_MATRIX;





#if TARGET_OS_IPHONE

@interface _MyViewController : UIViewController
@end

@interface _MyAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* _window;
@property (strong, nonatomic) _MyViewController* _controller;
@end

#else // TARGET_OS_MAC

@interface _MyViewController : NSViewController
@end

@interface _MyAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong, nonatomic) NSWindow* _window;
@end

#endif // TARGET_OS_IPHONE // TARGET_OS_MAC






@interface _MyMetalView : MTKView <MTKViewDelegate>
@end

#if TARGET_OS_IPHONE

@implementation _MyViewController

- (void) viewDidLoad {
	[super viewDidLoad];
	self.view = [[_MyMetalView alloc] initWithFrame:self.view.frame];
}

- (BOOL) shouldAutorotate {
	return YES;
}

- (UIInterfaceOrientationMask) supportedInterfaceOrientations {
	return UIInterfaceOrientationMaskLandscape;
}

- (BOOL) prefersStatusBarHidden {
	return YES;
}

- (BOOL) prefersHomeIndicatorAutoHidden {
	return YES;
}

@end // _MyViewController

#endif // TARGET_OS_IPHONE







@implementation _MyAppDelegate

#if TARGET_OS_IPHONE

- (BOOL) application: (UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
	
	// Setup the window
	self._window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[self._window setBackgroundColor:[UIColor blackColor]];
	[self._window setRootViewController:[_MyViewController new]];
	[self._window makeKeyAndVisible];
	
	// Run the startup callbacks after everything is turned on
	GSystem::RunStartupCallbacks();
	
	return YES;
}

- (void) applicationWillResignActive: (UIApplication*)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void) applicationDidEnterBackground: (UIApplication*)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void) applicationWillEnterForeground: (UIApplication*)application {
	// Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}

- (void) applicationDidBecomeActive: (UIApplication*)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void) applicationWillTerminate: (UIApplication*)application {
	// Run the shutdown callbacks before everything is turned off
	GSystem::RunShutdownCallbacks();
}

#else // TARGET_OS_MAC

- (void) applicationDidFinishLaunching: (NSNotification*)aNotification {
	
	// Setup the menus
	NSString* appName = [[NSProcessInfo processInfo] processName];
	NSMenu* mainMenu = [NSMenu new];
	NSMenuItem* appMenuItem = [NSMenuItem new];
	[mainMenu addItem:appMenuItem];
	NSMenu* appMenu = [NSMenu new];
	[appMenuItem setSubmenu:appMenu];
	NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:appName] action:@selector(terminate:) keyEquivalent:@"q"];
	[appMenu addItem:quitMenuItem];
	[NSApp setMainMenu:mainMenu];
	
	// If the preferred rect is not set, use a 1080p window (note that the window is resizable and will fit correctly in the window if this is too big)
	if(PREFERRED_RECT.width == 0 || PREFERRED_RECT.height == 0) {
		PREFERRED_RECT.x = 0;
		PREFERRED_RECT.y = 0;
		PREFERRED_RECT.width = 1920;
		PREFERRED_RECT.height = 1080;
	}
	
	// Setup the window
	NSRect windowRect = NSMakeRect(0, 0, PREFERRED_RECT.width, PREFERRED_RECT.height);
	self._window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable) backing:NSBackingStoreBuffered defer:YES];
	[self._window setContentAspectRatio:NSMakeSize((CGFloat)PREFERRED_RECT.width / (CGFloat)PREFERRED_RECT.height, ((CGFloat)1))];
	[self._window setTitle:[[NSProcessInfo processInfo] processName]];
	[self._window setContentView:[[_MyMetalView alloc] initWithFrame:windowRect]];
	[self._window makeKeyAndOrderFront:self];
	[self._window center];
	
	// Run the startup callbacks after everything is turned on
	GSystem::RunStartupCallbacks();
}

- (void) applicationWillTerminate: (NSNotification*)notification {
	// Run the shutdown callbacks before everything is turned off
	GSystem::RunShutdownCallbacks();
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)sender {
	return YES;
}

#endif // TARGET_OS_IPHONE // TARGET_OS_MAC

@end // _MyAppDelegate

















@implementation _MyMetalView {
	id<MTLCommandQueue> _commandQueue;
	id<MTLRenderPipelineState> _pipelineState;
	vector_uint2 _viewport;
}

- (id) initWithFrame: (CGRect)frameRect {
	
#if TARGET_OS_IPHONE
	frameRect = [[UIScreen mainScreen] nativeBounds]; // Convert the view to use the full pixel coordinates of the screen (retina)
#else // TARGET_OS_MAC
	frameRect = [[NSScreen mainScreen] convertRectToBacking:frameRect]; // Convert the view to use the full pixel coordinates of the screen (retina)
#endif // TARGET_OS_IPHONE //TARGET_OS_MAC
	
	DEVICE = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame:frameRect device:DEVICE];
	[self setDelegate:self];
	[self setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
	[self setClearColor:MTLClearColorMake(0.0, 0.0, 0.0, 0.0)];
	[self setPreferredFramesPerSecond:(NSInteger)FPS];
	//[self mtkView:self drawableSizeWillChange:frameRect.size];
	
	// Added this as a string for simplicity for multiple platforms.  This can be added to a .metal file instead
	// and the id<MTLLibrary> defaultLibrary code below can be changed to newDefaultLibrary.
	constexpr NSString* SHADER = @""
	"#include <metal_stdlib>\n"
	"#include <simd/simd.h>\n"
	"using namespace metal;\n"
	"\n"
	"struct _CustomVertex {\n"
	"	packed_float2 xy;\n"
	"	packed_uchar4 rgba;\n"
	"	packed_float2 uv;\n"
	"};\n"
	"\n"
	"struct _VertexOutput {\n"
	"	float4 xyzw [[position]];\n"
	"	float4 rgba;\n"
	"	float2 uv;\n"
	"};\n"
	"\n"
	"vertex _VertexOutput _VertexShader (constant _CustomVertex* vertexArray [[buffer(0)]],\n"
	"									constant float4x4* matrix  [[buffer(1)]],\n"
	"									ushort vertexID [[vertex_id]]) {\n"
	"	_VertexOutput out;\n"
	"	out.xyzw = *matrix * float4(vertexArray[vertexID].xy, 0.0, 1.0);\n"
	"	out.rgba = float4(vertexArray[vertexID].rgba) / 255.0;\n"
	"	out.uv = float2(vertexArray[vertexID].uv);\n"
	"	return out;\n"
	"}\n"
	"\n"
	"fragment float4 _FragmentShader (_VertexOutput in [[stage_in]],\n"
	"								 texture2d<float> texture [[texture(0)]]) {\n"
	"	return in.rgba * float4(texture.sample(sampler(mag_filter::linear, min_filter::linear), in.uv));\n"
	"}\n"
	"";
	
	NSError* error = NULL;
	id<MTLLibrary> defaultLibrary = [self.device newLibraryWithSource:SHADER options:nil error:&error];
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
	
	_pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
	_commandQueue = [self.device newCommandQueue];
	
	return self;
}

- (void) drawInMTKView:(nonnull MTKView*)view {
	id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
	MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
	if(renderPassDescriptor != nil) {
		RENDER = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[RENDER setViewport:(MTLViewport){(double)0, (double)0, (double)_viewport.x, (double)_viewport.y, (double)-1, (double)1}];
		[RENDER setRenderPipelineState:_pipelineState];
		GSystem::MatrixSetModelDefault();
		GSystem::MatrixSetProjectionDefault();
		GSystem::MatrixUpdate();
		GSystem::RunDrawCallbacks();
		[RENDER endEncoding];
		[commandBuffer presentDrawable:view.currentDrawable];
	}
	[commandBuffer commit];
}

- (void) mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
	_viewport.x = size.width;
	_viewport.y = size.height;
	
	// Set the rect used for drawing, this includes the entire viewport including hidden areas because of curved edges or notches
	SCREEN_RECT.x = 0;
	SCREEN_RECT.y = 0;
	SCREEN_RECT.width = _viewport.x;
	SCREEN_RECT.height = _viewport.y;
	
	// The safe area is a subsection of the screen that is fully visible and does not interfere with device interactions such as swiping areas (initially set to the entire window/screen)
#if TARGET_OS_IPHONE
	// Get the safe area insets from the edge of the screen already part of iOS and set the safe rect using them
	int left = SCREEN_RECT.width * (int)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.left / (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.width;
	int top = SCREEN_RECT.height * (int)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.top / (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.height;
	int right = SCREEN_RECT.width * (int)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.right / (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.width;
	int bottom = SCREEN_RECT.height * (int)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.bottom / (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.height;
	SAFE_RECT.x = left;
	SAFE_RECT.y = top;
	SAFE_RECT.width = SCREEN_RECT.width - left - right;
	SAFE_RECT.height = SCREEN_RECT.height - top - bottom;
#else
	// On Mac the safe rect is the screen rect
	SAFE_RECT = SCREEN_RECT;
#endif
	
	// If no preferred rect is set, use the safe rect's width and height
	if(PREFERRED_RECT.width == 0 || PREFERRED_RECT.height == 0) {
		PREFERRED_RECT.x = 0;
		PREFERRED_RECT.y = 0;
		PREFERRED_RECT.width = SAFE_RECT.width;
		PREFERRED_RECT.height = SAFE_RECT.height;
	}
	
	// The screen rect and safe rect are adjusted to fit the preferred rect as best as possible
	if(SAFE_RECT.width != PREFERRED_RECT.width) {
		SCREEN_RECT.x = SCREEN_RECT.x * PREFERRED_RECT.width / SAFE_RECT.width;
		SCREEN_RECT.y = SCREEN_RECT.y * PREFERRED_RECT.width / SAFE_RECT.width;
		SCREEN_RECT.width = SCREEN_RECT.width * PREFERRED_RECT.width / SAFE_RECT.width;
		SCREEN_RECT.height = SCREEN_RECT.height * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.x = SAFE_RECT.x * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.y = SAFE_RECT.y * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.height = SAFE_RECT.height * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.width = PREFERRED_RECT.width; // This must be last because it is used for the aspect ratio
	}
	if(SAFE_RECT.height < PREFERRED_RECT.height) {
		SCREEN_RECT.x = SCREEN_RECT.x * PREFERRED_RECT.height / SAFE_RECT.height;
		SCREEN_RECT.y = SCREEN_RECT.y * PREFERRED_RECT.height / SAFE_RECT.height;
		SCREEN_RECT.width = SCREEN_RECT.width * PREFERRED_RECT.height / SAFE_RECT.height;
		SCREEN_RECT.height = SCREEN_RECT.height * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.x = SAFE_RECT.x * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.y = SAFE_RECT.y * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.width = SAFE_RECT.width * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.height = PREFERRED_RECT.height; // This must be last because it is used for the aspect ratio
	}
	
	// Center the preferred rect within the screen rect then adjust to fit within the safe rect
	PREFERRED_RECT.x = SCREEN_RECT.width / 2 - PREFERRED_RECT.width / 2;
	PREFERRED_RECT.y = SCREEN_RECT.height / 2 - PREFERRED_RECT.height / 2;
	if(PREFERRED_RECT.x < SAFE_RECT.x)
		PREFERRED_RECT.x = SAFE_RECT.x;
	if(PREFERRED_RECT.y < SAFE_RECT.y)
		PREFERRED_RECT.y = SAFE_RECT.y;
	if(PREFERRED_RECT.x + PREFERRED_RECT.width > SAFE_RECT.x + SAFE_RECT.width)
		PREFERRED_RECT.x = SAFE_RECT.x + SAFE_RECT.width - PREFERRED_RECT.width;
	if(PREFERRED_RECT.y + PREFERRED_RECT.height > SAFE_RECT.y + SAFE_RECT.height)
		PREFERRED_RECT.y = SAFE_RECT.y + SAFE_RECT.height - PREFERRED_RECT.height;
}

- (void) encodeWithCoder:(nonnull NSCoder*)aCoder { 
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (BOOL) becomeFirstResponder {
	[super becomeFirstResponder];
	return YES;
}

- (BOOL) resignFirstResponder {
	return [super resignFirstResponder];
}

#if TARGET_OS_IPHONE

- (void) touchesBegan: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchCallbacks((int)location.x, (int)location.y);
	}
}

- (void) touchesMoved: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchMoveCallbacks((int)location.x, (int)location.y);
	}
}

- (void) touchesEnded: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int)location.x, (int)location.y);
	}
}

- (void) touchesCancelled: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int)location.x, (int)location.y);
	}
}

#else // TARGET_OS_MAC

- (void) mouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x, (int)location.y); // [theEvent buttonNumber]
}

- (void) rightMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x, (int)location.y);
}

- (void) otherMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x, (int)location.y);
}

- (void) mouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x, (int)location.y);
}

- (void) rightMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x, (int)location.y);
}

- (void) otherMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x, (int)location.y);
}

- (void) mouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x, (int)location.y);
}

- (void) rightMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x, (int)location.y);
}

- (void) otherMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x, (int)location.y);
}

#endif // TARGET_OS_IPHONE // TARGET_OS_MAC

@end // _MyMetalView
























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




static const GString& _GetResourceDirectory () {
	static GString RESOURCE_DIRECTORY;
	if(RESOURCE_DIRECTORY.IsEmpty()) {
		CFURLRef directory = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
		char resources[PATH_MAX];
		CFURLGetFileSystemRepresentation(directory, true, (UInt8*)resources, PATH_MAX);
		CFRelease(directory);
		RESOURCE_DIRECTORY.New(resources);
	}
	return RESOURCE_DIRECTORY;
}

static const GString& _GetSaveDirectory () {
	static GString SAVE_DIRECTORY;
	if(SAVE_DIRECTORY.IsEmpty()) {
#if TARGET_OS_IPHONE
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
#else
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
#endif
		if([paths count] > 0)
			SAVE_DIRECTORY.New([[paths objectAtIndex:0] fileSystemRepresentation]);
	}
	return SAVE_DIRECTORY;
}

FILE* GSystem::OpenResourceFileForRead (const GString& resource) {
	return fopen(GString().Format("%s/%s", (const char*)_GetResourceDirectory(), (const char*)resource), "rb");
}

FILE* GSystem::OpenSaveFileForRead (const GString& name) {
	return fopen(GString().Format("%s/%s.sav", (const char*)_GetSaveDirectory(), (const char*)name), "rb");
}

FILE* GSystem::OpenSaveFileForWrite (const GString& name) {
	return fopen(GString().Format("%s/%s.sav", (const char*)_GetSaveDirectory(), (const char*)name), "wb+");
}

std::vector<GString> GSystem::GetFileNamesInDirectory (const GString& path) {
	std::vector<GString> files;
	
	GString directory = path;
	if(directory.IsEmpty())
		directory = "./";
	
	if(directory[directory.GetLength() - 1] != '/')
		directory += "/";
	
	DIR* dir = opendir(directory);
	if(dir == nullptr) {
		FILE* file = fopen(path, "rb");
		if(file != nullptr) {
			files.push_back(path);
			fclose(file);
			return files;
		}
		return files;
	}
	
	for(dirent* info = readdir(dir); info != nullptr; info = readdir(dir)) {
		if(info->d_type == DT_DIR) {
			if(GString::strcmp(info->d_name, ".") != 0 && GString::strcmp(info->d_name, "..") != 0) {
				std::vector<GString> sub = GetFileNamesInDirectory(directory + info->d_name);
				files.reserve(files.size() + sub.size());
				files.insert(files.end(), sub.begin(), sub.end());
			}
		} else {
			files.push_back(directory + info->d_name);
		}
	}
	
	closedir(dir);
	return files;
}

void GSystem::RunPreferredSize (int width, int height) {
	SCREEN_RECT = GRect(0, 0, width, height);
	SAFE_RECT = GRect(0, 0, width, height);
	PREFERRED_RECT = GRect(0, 0, width, height);
}

void GSystem::RunPreferredFPS (int fps) {
	FPS = fps;
}

void GSystem::RunPreferredArgs (int argc, char* argv[]) {
	ARG_C = argc;
	ARG_V = argv;
}

int GSystem::Run () {
#if TARGET_OS_IPHONE
	GSystem::Debug("Running iOS Application...\n");
	@autoreleasepool {
		return UIApplicationMain((int)ARG_C, ARG_V, nil, NSStringFromClass([_MyAppDelegate class]));
	}
#else // TARGET_OS_MAC
	GSystem::Debug("Running MacOS Application...\n");
	@autoreleasepool {
		[[NSApplication sharedApplication] setDelegate:[[_MyAppDelegate alloc] init]];
		[[NSApplication sharedApplication] run];
	}
#endif // TARGET_OS_IPHONE // TARGET_OS_MAC
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
	if(RENDER != nil) {
		static GMatrix32_4x4 MATRIX;
		MATRIX = PROJECTION_MATRIX * MODEL_MATRIX;
		[RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	}
}

void GSystem::Print (const char* message, ...) {
	if (message) {
		va_list args;
		va_start(args, message);
		vprintf(message, args);
		va_end(args);
	}
}

void GSystem::Debug (const char* message, ...) {
#if DEBUG
	if (message) {
		va_list args;
		va_start(args, message);
		vprintf(message, args);
		va_end(args);
	}
#endif
}

#endif // __APPLE__
