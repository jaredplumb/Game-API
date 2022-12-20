#import "GSystem.h"
#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <dirent.h>



static int						SCREEN_NATIVE_WIDTH = 0;
static int						SCREEN_NATIVE_HEIGHT = 0;
static GRect					SCREEN_RECT;
static GRect					SAFE_RECT;
static GRect					PREFERRED_RECT;
static int						FPS = 60;
static int						ARG_C = 0;
static char**					ARG_V = nullptr;
id<MTLDevice>					DEVICE = nil; // Non-static to allow for extern access from GImage
id<MTLRenderCommandEncoder>		RENDER = nil; // Non-static to allow for extern access from GImage
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
	
	// If the preferred rect is not set, use a 1080p window (note that the system window is resizable and will fit correctly in the window if this size is too big for the screen)
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
}

- (id) initWithFrame: (CGRect)frameRect {
	
#if TARGET_OS_IPHONE
	frameRect = [[UIScreen mainScreen] nativeBounds]; // Convert the view to use the full pixel coordinates of the screen (retna)
#else // TARGET_OS_MAC
	frameRect = [[NSScreen mainScreen] convertRectToBacking:frameRect]; // Convert the view to use the full pixel coordinates of the screen (retna)
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
		[RENDER setViewport:(MTLViewport){(double)0, (double)0, (double)SCREEN_NATIVE_WIDTH, (double)SCREEN_NATIVE_HEIGHT, (double)-1, (double)1}];
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
	// The screen uses native pixels, but the window uses points, which makes a pixel to point someting line 3 to 1 depending on hardware
	SCREEN_NATIVE_WIDTH = (int)size.width;
	SCREEN_NATIVE_HEIGHT = (int)size.height;
	
#if TARGET_OS_IPHONE
	// Set the screen rect to the window so that the screen rect and safe rect have the same pixel correlation
	SCREEN_RECT.x = 0;
	SCREEN_RECT.y = 0;
	SCREEN_RECT.width = (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.width;
	SCREEN_RECT.height = (int)UIApplication.sharedApplication.windows.firstObject.bounds.size.height;
	
	// The safe area is a subsection of the screen that is fully visible and does not interfere with device interactions such as swiping areas, the safe rect is in window points (not pixels)
	const UIEdgeInsets windowInsets = UIApplication.sharedApplication.windows.firstObject.safeAreaInsets;
	SAFE_RECT.x = (int)(UIApplication.sharedApplication.windows.firstObject.bounds.origin.x + windowInsets.left);
	SAFE_RECT.y = (int)(UIApplication.sharedApplication.windows.firstObject.bounds.origin.y + windowInsets.top);
	SAFE_RECT.width = (int)(UIApplication.sharedApplication.windows.firstObject.bounds.size.width - windowInsets.left - windowInsets.right);
	SAFE_RECT.height = (int)(UIApplication.sharedApplication.windows.firstObject.bounds.size.height - windowInsets.top - windowInsets.bottom);
	
#else
	// On Mac the screen rect and safe rect is the size of the native window
	SCREEN_RECT.x = 0;
	SCREEN_RECT.y = 0;
	SCREEN_RECT.width = SCREEN_NATIVE_WIDTH;
	SCREEN_RECT.height = SCREEN_NATIVE_HEIGHT;
	SAFE_RECT = SCREEN_RECT;
#endif
	
	// If no preferred rect is set, use the safe rect's width and height
	if(PREFERRED_RECT.width == 0 || PREFERRED_RECT.height == 0) {
		PREFERRED_RECT.x = 0;
		PREFERRED_RECT.y = 0;
		PREFERRED_RECT.width = SAFE_RECT.width;
		PREFERRED_RECT.height = SAFE_RECT.height;
	}
	
	// Adjust the safe rect to the relative preferred rect size using the pixel size of the preferred rect and adjust the screen rect and safe rect accordingly
	if(PREFERRED_RECT.width * SAFE_RECT.height <= SAFE_RECT.width * PREFERRED_RECT.height) {
		SCREEN_RECT.width = SCREEN_RECT.width * PREFERRED_RECT.height / SAFE_RECT.height;
		SCREEN_RECT.height = SCREEN_RECT.height * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.x = SAFE_RECT.x * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.y = SAFE_RECT.y * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.width = SAFE_RECT.width * PREFERRED_RECT.height / SAFE_RECT.height;
		SAFE_RECT.height = PREFERRED_RECT.height;
	} else {
		SCREEN_RECT.width = SCREEN_RECT.width * PREFERRED_RECT.width / SAFE_RECT.width;
		SCREEN_RECT.height = SCREEN_RECT.height * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.x = SAFE_RECT.x * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.y = SAFE_RECT.y * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.height = SAFE_RECT.height * PREFERRED_RECT.width / SAFE_RECT.width;
		SAFE_RECT.width = PREFERRED_RECT.width;
	}
	
	// Center the preferred rect inside of the safe rect
	PREFERRED_RECT.x = SAFE_RECT.x + (SAFE_RECT.width - PREFERRED_RECT.width) / 2;
	PREFERRED_RECT.y = SAFE_RECT.y + (SAFE_RECT.height - PREFERRED_RECT.height) / 2;
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
		GSystem::RunTouchCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
	}
}

- (void) touchesMoved: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchMoveCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
	}
}

- (void) touchesEnded: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
	}
}

- (void) touchesCancelled: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
	}
}

#else // TARGET_OS_MAC
- (void) mouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y); // [theEvent buttonNumber]
}

- (void) rightMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) otherMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) mouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) rightMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) otherMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchUpCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) mouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) rightMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
}

- (void) otherMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)SCREEN_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)SCREEN_RECT.height / self.frame.size.height;
	GSystem::RunTouchMoveCallbacks((int)location.x - PREFERRED_RECT.x, (int)location.y - PREFERRED_RECT.y);
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



static FILE* PACKAGE_FILE = nullptr;
static std::unordered_map<std::string, std::pair<int64_t, int64_t>> PACKAGE_RESOURCES;



static const char* GetResourceDirectory () {
	static std::string RESOURCE_DIRECTORY;
	if(RESOURCE_DIRECTORY.empty()) {
		if([NSBundle mainBundle] != nil && [[NSBundle mainBundle] resourceURL] != nil && [[NSBundle mainBundle] bundleIdentifier] != nil)
			RESOURCE_DIRECTORY = std::string([[[NSBundle mainBundle] resourceURL] fileSystemRepresentation]);
		else
			RESOURCE_DIRECTORY = "./";
		GSystem::Debug("Resources: %s ... ", RESOURCE_DIRECTORY.c_str());
	}
	return RESOURCE_DIRECTORY.c_str();
}



bool GSystem::PackageOpen (const GString& resource) {
	if(PACKAGE_FILE != nullptr)
		PackageClose();
	
	GSystem::Debug("Opening package \"%s\"... ", (const char*)resource);
	
	PACKAGE_FILE = fopen(GString().Format("%s/%s", GetResourceDirectory(), (const char*)resource), "rb");
	if(PACKAGE_FILE == nullptr) {
		GSystem::Debug("Failed to open package for reading!\n");
		return false;
	}
	
	int64_t headerOffset;
	if(fread(&headerOffset, sizeof(headerOffset), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read package header offset!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	if(fseeko(PACKAGE_FILE, (off_t)headerOffset, SEEK_SET) != 0) { // The header is actually at the end of the file
		GSystem::Debug("Failed to set package position to header!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	int64_t bufferSize;
	if(fread(&bufferSize, sizeof(bufferSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read package header buffer size!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	int64_t archiveSize;
	if(fread(&archiveSize, sizeof(archiveSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read package header archive size!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	off_t current = ftello(PACKAGE_FILE);
	if(fseeko(PACKAGE_FILE, 0, SEEK_END) != 0 || ftello(PACKAGE_FILE) - current < archiveSize) {
		GSystem::Debug("Package file is corrupt!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	fseeko(PACKAGE_FILE, current, SEEK_SET);
	
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	if(fread(archive.get(), archiveSize * sizeof(uint8_t), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read package header archive!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[bufferSize]);
	if(GArchive::Decompress(archive.get(), archiveSize * sizeof(uint8_t), buffer.get(), bufferSize * sizeof(uint8_t)) != bufferSize) {
		GSystem::Debug("Failed to decompress package header data!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	int64_t count = *((int64_t*)(buffer.get()));
	if(count <= 0) {
		GSystem::Debug("Package contains no resources!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return true;
	}
	
	int64_t bufferOffset = sizeof(count);
	for(int64_t i = 0; i < count; i++) {
		char* resourceName = (char*)(buffer.get() + bufferOffset);
		bufferOffset += GString::strlen(resourceName) + 1;
		int64_t resourceSize = *((int64_t*)(buffer.get() + bufferOffset));
		bufferOffset += sizeof(resourceSize);
		int64_t resourceOffset = *((int64_t*)(buffer.get() + bufferOffset));
		bufferOffset += sizeof(resourceOffset);
		PACKAGE_RESOURCES[resourceName] = std::make_pair(resourceSize, resourceOffset);
	}
	
	GSystem::Debug("Done\n");
	return true;
}



bool GSystem::PackageOpenForWrite (const GString& resource) {
	if(PACKAGE_FILE != nullptr)
		PackageCloseForWrite();
	
	GSystem::Debug("Opening package \"%s\" for writing... ", (const char*)resource);
	
	PACKAGE_FILE = fopen(GString().Format("%s/%s", GetResourceDirectory(), (const char*)resource), "wb+");
	if(PACKAGE_FILE == nullptr) {
		GSystem::Debug("Failed to open package for writing!\n");
		return false;
	}
	
	int64_t headerOffset;
	if(fwrite(&headerOffset, sizeof(headerOffset), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write placeholder package header offset!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	GSystem::Debug("Done\n");
	return true;
}



bool GSystem::PackageClose () {
	if(PACKAGE_FILE) {
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
	}
	if(!PACKAGE_RESOURCES.empty())
		PACKAGE_RESOURCES.clear();
	return true;
}



bool GSystem::PackageCloseForWrite () {
	if(PACKAGE_FILE == nullptr) {
		GSystem::Debug("Package is not open for writing!\n");
		return false;
	}
	
	int64_t headerOffset = ftello(PACKAGE_FILE);
	if(headerOffset <= sizeof(headerOffset) || PACKAGE_RESOURCES.empty()) {
		GSystem::Debug("No resources were written to the package!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	int64_t bufferSize = sizeof(int64_t); // Save room for the number of resources
	for(auto i = PACKAGE_RESOURCES.begin(); i != PACKAGE_RESOURCES.end(); i++)
		bufferSize += i->first.length() + 1 + sizeof(int64_t) + sizeof(int64_t);
	
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[bufferSize]);
	*((int64_t*)buffer.get()) = PACKAGE_RESOURCES.size();
	int64_t bufferOffset = sizeof(int64_t);
	for(auto i = PACKAGE_RESOURCES.begin(); i != PACKAGE_RESOURCES.end(); i++) {
		GString::strcpy((char*)(buffer.get() + bufferOffset), i->first.c_str());
		bufferOffset += i->first.length() + 1;
		*((int64_t*)(buffer.get() + bufferOffset)) = i->second.first;
		bufferOffset += sizeof(int64_t);
		*((int64_t*)(buffer.get() + bufferOffset)) = i->second.second;
		bufferOffset += sizeof(int64_t);
	}
	
	int64_t archiveSize = GArchive::GetBufferBounds(bufferSize);
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	archiveSize = GArchive::Compress(buffer.get(), bufferSize * sizeof(uint8_t), archive.get(), archiveSize * sizeof(uint8_t));
	if(archiveSize == 0) {
		GSystem::Debug("Failed to compress package header data!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	if(fwrite(&bufferSize, sizeof(bufferSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write package header buffer size!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	if(fwrite(&archiveSize, sizeof(archiveSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write package header archive size!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	if(fwrite(archive.get(), archiveSize * sizeof(uint8_t), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write package header archive!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	if(fseeko(PACKAGE_FILE, 0, SEEK_SET) != 0 || fwrite(&headerOffset, sizeof(headerOffset), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write package header offset!\n");
		fclose(PACKAGE_FILE);
		PACKAGE_FILE = nullptr;
		return false;
	}
	
	fclose(PACKAGE_FILE);
	PACKAGE_FILE = nullptr;
	PACKAGE_RESOURCES.clear();
	return true;
}



int64_t GSystem::ResourceSize (const GString& name) {
	if(PACKAGE_FILE == nullptr) // Open default package if none exists
		PackageOpen();
	auto i = PACKAGE_RESOURCES.find((const char*)name);
	return i != PACKAGE_RESOURCES.end() ? i->second.first : ResourceSizeFromFile(name);
}



int64_t GSystem::ResourceSizeFromFile (const GString& path) {
	FILE* file = fopen(GString().Format("%s/%s", GetResourceDirectory(), (const char*)path), "rb");
	if(file == nullptr) {
		GSystem::Debug("Failed to find resource \"%s\"!\n", (const char*)path);
		return 0;
	}
	fseeko(file, 0, SEEK_END);
	int64_t size = ftello(file);
	fclose(file);
	return size;
}



bool GSystem::ResourceRead (const GString& name, void* data, int64_t size) {
	if(PACKAGE_FILE == nullptr) // Open default package if none exists
		PackageOpen();
	
	if(PACKAGE_FILE == nullptr) // No package file, attempting to find the raw resource in the resource location
		return ResourceReadFromFile(name, data, size);
	
	auto resource = PACKAGE_RESOURCES.find((const char*)name);
	if(resource == PACKAGE_RESOURCES.end()) // Failed to find the resource in the package, attempting to find the raw resource in the resource location
		return ResourceReadFromFile(name, data, size);
	
	int64_t archiveSize;
	if(fseeko(PACKAGE_FILE, (off_t)resource->second.second, SEEK_SET) != 0 || fread(&archiveSize, sizeof(archiveSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read archive size for resource \"%s\"!\n", (const char*)name);
		return false;
	}
	
	off_t current = ftello(PACKAGE_FILE);
	if(fseeko(PACKAGE_FILE, 0, SEEK_END) != 0 || ftello(PACKAGE_FILE) - current < archiveSize) {
		GSystem::Debug("Package is corrupt for resource \"%s\"!\n", (const char*)name);
		return false;
	}
	fseeko(PACKAGE_FILE, current, SEEK_SET);
	
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	if(fread(archive.get(), archiveSize * sizeof(uint8_t), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to read archive for resource \"%s\"!\n", (const char*)name);
		return false;
	}
	
	if(GArchive::Decompress(archive.get(), archiveSize, data, size) != size) {
		GSystem::Debug("Failed to decompress resource \"%s\"!\n", (const char*)name);
		return false;
	}
	
	return true;
}



bool GSystem::ResourceReadFromFile (const GString& path, void* data, int64_t size) {
	FILE* file = fopen(GString().Format("%s/%s", GetResourceDirectory(), (const char*)path), "rb");
	if(file == nullptr) {
		GSystem::Debug("Failed to open resource \"%s\"!\n", (const char*)path);
		return false;
	}
	
	if(fseeko(file, 0, SEEK_END) != 0 || ftello(file) < size) {
		GSystem::Debug("Resource \"%s\" is larger than data buffer!\n", (const char*)path);
		fclose(file);
		return false;
	}
	fseeko(file, 0, SEEK_SET);
	
	if(fread(data, size, 1, file) != 1) {
		GSystem::Debug("Failed to read resource \"%s\"!\n", (const char*)path);
		fclose(file);
		return false;
	}
	
	fclose(file);
	return true;
}



bool GSystem::ResourceWrite (const GString& name, void* data, int64_t size) {
	if(PACKAGE_FILE == nullptr) // Open default package if none exists
		PackageOpenForWrite();
	
	if(PACKAGE_FILE == nullptr) {
		GSystem::Debug("Package file is not open for writing!\n");
		return false;
	}
	
	int64_t archiveSize = GArchive::GetBufferBounds(size);
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	archiveSize = GArchive::Compress(data, size, archive.get(), archiveSize);
	if(archiveSize == 0) {
		GSystem::Debug("Failed to compress resource data!\n");
		return false;
	}
	
	int64_t offset = ftello(PACKAGE_FILE);
	
	if(fwrite(&archiveSize, sizeof(archiveSize), 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write resource data archive size!\n");
		return false;
	}
	
	if(fwrite(archive.get(), archiveSize, 1, PACKAGE_FILE) != 1) {
		GSystem::Debug("Failed to write resource data archive!\n");
		return false;
	}
	
	auto i = PACKAGE_RESOURCES.find((const char*)name);
	if(i != PACKAGE_RESOURCES.end()) {
		GSystem::Debug("Resource \"%s\" already exists!\n", (const char*) name);
		return false;
	}
	
	PACKAGE_RESOURCES[(const char*)name] = std::make_pair(size, offset);
	return true;
}



static const char* GetSaveDirectory () {
	static std::string SAVE_DIRECTORY;
	if(SAVE_DIRECTORY.empty()) {
		NSURL* support = [[[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:[[NSBundle mainBundle] bundleIdentifier]];
		if([[NSFileManager defaultManager] fileExistsAtPath:[support path]] == NO)
			[[NSFileManager defaultManager] createDirectoryAtPath:[support path] withIntermediateDirectories:YES attributes:nil error:nil];
		SAVE_DIRECTORY = std::string([support fileSystemRepresentation]);
		GSystem::Debug("Saves: \"%s\"\n", SAVE_DIRECTORY.c_str());
	}
	return SAVE_DIRECTORY.c_str();
}



bool GSystem::SaveRead (const GString& name, void* data, int64_t size) {
	
#if DEBUG
	GetSaveDirectory(); // This is to make the debug output in GetSaveDirectory happen before the debug output in this function
	GSystem::Debug("Reading save \"%s\" ... ", (const char*)name);
	int64_t elapse = GSystem::GetMilliseconds();
#endif
	
	FILE* file = fopen(GString().Format("%s/%s.sav", GetSaveDirectory(), (const char*)name), "rb");
	if(file == nullptr) {
		GSystem::Debug("Failed to open save for reading!\n");
		return false;
	}
	
	int64_t archiveSize;
	if(fread(&archiveSize, sizeof(archiveSize), 1, file) != 1 || archiveSize <= 0) {
		GSystem::Debug("Failed to read save data archive size!\n");
		fclose(file);
		return false;
	}
	
	off_t current = ftello(file);
	if(fseeko(file, 0, SEEK_END) != 0 || ftello(file) - current < archiveSize) {
		GSystem::Debug("Save file is corrupt!\n");
		fclose(file);
		return false;
	}
	fseeko(file, current, SEEK_SET);
	
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	if(fread(archive.get(), archiveSize * sizeof(uint8_t), 1, file) != 1) {
		GSystem::Debug("Failed to read save data archive!\n");
		fclose(file);
		return false;
	}
	
	if(GArchive::Decompress(archive.get(), archiveSize, data, size) != size) {
		GSystem::Debug("Failed to decompress save data!\n");
		fclose(file);
		return false;
	}
	
	fclose(file);
	
#if DEBUG
	elapse = GSystem::GetMilliseconds() - elapse;
	GSystem::Debug("Done (%d ms)\n", (int)elapse);
#endif
	
	return true;
}



bool GSystem::SaveWrite (const GString& name, const void* data, int64_t size) {
	
#if DEBUG
	GSystem::Debug("Writing save \"%s\" ... ", (const char*)name);
	int64_t elapse = GSystem::GetMilliseconds();
#endif
	
	int64_t archiveSize = GArchive::GetBufferBounds(size);
	std::unique_ptr<uint8_t[]> archive(new uint8_t[archiveSize]);
	archiveSize = GArchive::Compress(data, size, archive.get(), archiveSize);
	if(archiveSize == 0) {
		GSystem::Debug("Failed to compress save data!\n");
		return false;
	}
	
	FILE* file = fopen(GString().Format("%s/%s.sav", (const char*)GetSaveDirectory(), (const char*)name), "wb+");
	if(file == nullptr) {
		GSystem::Debug("Failed to open save for writing!\n");
		return false;
	}
	
	if(fwrite(&archiveSize, sizeof(archiveSize), 1, file) != 1) {
		GSystem::Debug("Failed to write save data archive size!\n");
		fclose(file);
		return false;
	}
	
	if(fwrite(archive.get(), archiveSize, 1, file) != 1) {
		GSystem::Debug("Failed to write save data archive!\n");
		fclose(file);
		return false;
	}
	
	fclose(file);
	
#if DEBUG
	elapse = GSystem::GetMilliseconds() - elapse;
	GSystem::Debug("Done (%d bytes compressed to %d bytes in %d ms)\n", (int)size, (int)archiveSize, (int)elapse);
#endif
	
	return true;
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
		return UIApplicationMain(ARG_C, ARG_V, nil, NSStringFromClass([_MyAppDelegate class]));
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



// This is s good reference if matrix math if needed in the future
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
