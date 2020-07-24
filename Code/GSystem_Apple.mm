#import "GSystem.h"
#if PLATFORM_MACOSX || PLATFORM_IOS

static GRect			_RECT			(0, 0, 1280, 720);
static GRect			_SAFE_RECT		(0, 0, 1280, 720);
static GRect			_PREFERRED_RECT	(0, 0, 1280, 720);
static int_t			_FPS		    = 60;
static int_t			_ARG_C			= 0;
static char**			_ARG_V			= NULL;

id<MTLDevice>				_DEVICE = nil;
id<MTLRenderCommandEncoder>	_RENDER = nil;
static matrix_float4x4		_MODEL_MATRIX;
static matrix_float4x4		_PROJECTION_MATRIX;








#if PLATFORM_MACOSX
@interface _MyViewController : NSViewController
@end

@interface _MyAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong, nonatomic) NSWindow* _window;
@end
#endif

#if PLATFORM_IOS
@interface _MyViewController : UIViewController
@end

@interface _MyAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* _window;
@property (strong, nonatomic) _MyViewController* _controller;
@end
#endif




@interface _MyMetalView : MTKView <MTKViewDelegate>
@end




#if PLATFORM_IOS
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
#endif




@implementation _MyAppDelegate

#if PLATFORM_MACOSX

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
	
	// Setup the window
	NSRect windowRect = NSMakeRect(0, 0, _PREFERRED_RECT.width, _PREFERRED_RECT.height);
	self._window = [[NSWindow alloc] initWithContentRect:windowRect styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable) backing:NSBackingStoreBuffered defer:YES];
	[self._window setContentAspectRatio:NSMakeSize((CGFloat)_PREFERRED_RECT.width / (CGFloat)_PREFERRED_RECT.height, ((CGFloat)1))];
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

#endif // PLATFORM_MACOSX

#if PLATFORM_IOS

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

#endif // PLATFORM_IOS

@end // _MyAppDelegate








// Added this as a string for simplicity for multiple platforms.  This can be added to a .metal file instead
// and the id<MTLLibrary> defaultLibrary code below can be changed to newDefaultLibrary.
static NSString* _SHADER = @""
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








@implementation _MyMetalView
{
	id<MTLCommandQueue> _commandQueue;
	id<MTLRenderPipelineState> _pipelineState;
	vector_uint2 _viewport;
}

- (id) initWithFrame: (CGRect)frameRect {
	
#if PLATFORM_MACOSX
	// Convert the view to use the full pixel coordinates of the screen (retina)
	frameRect = [[NSScreen mainScreen] convertRectToBacking:frameRect];
#endif
	
#if PLATFORM_IOS
	// Convert the view to use the full pixel coordinates of the screen (retina)
	frameRect = [[UIScreen mainScreen] nativeBounds];
#endif
	
	_DEVICE = MTLCreateSystemDefaultDevice();
	self = [super initWithFrame:frameRect device:_DEVICE];
	[self setDelegate:self];
	[self setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
	[self setClearColor:MTLClearColorMake(0.0, 0.0, 0.0, 0.0)];
	[self setPreferredFramesPerSecond:(NSInteger)_FPS];
	//[self mtkView:self drawableSizeWillChange:frameRect.size];
	
	NSError* error = NULL;
	id<MTLLibrary> defaultLibrary = [self.device newLibraryWithSource:_SHADER options:nil error:&error];
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
		
		_RENDER = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		[_RENDER setViewport:(MTLViewport){(double)0, (double)0, (double)_viewport.x, (double)_viewport.y, (double)-1, (double)1}];
		[_RENDER setRenderPipelineState:_pipelineState];
		
		GSystem::MatrixSetModelDefault();
		GSystem::MatrixSetProjectionDefault();
		GSystem::MatrixUpdate();
		GSystem::RunDrawCallbacks();
		
		[_RENDER endEncoding];
		
		[commandBuffer presentDrawable:view.currentDrawable];
	}
	[commandBuffer commit];
}

- (void) mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
	_viewport.x = size.width;
	_viewport.y = size.height;
	
	// Set the rect used for drawing, this includes the entire screen including hidden areas because of curved edges
	_RECT.x = 0;
	_RECT.y = 0;
	_RECT.width = _viewport.x;
	_RECT.height = _viewport.y;
	
	// Set the safe are for for interactions.  This is used to make sure UI elements and mouse/touch interaction is in the correct area
	_SAFE_RECT = _RECT;
	
#if PLATFORM_IOS
	// Get the safe area already part of iOS.  The UIWindow may not be retina, so adjust offsets accordingly
	int_t left = _RECT.width * (int_t)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.left / (int_t)UIApplication.sharedApplication.windows.firstObject.bounds.size.width;
	int_t top = _RECT.height * (int_t)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.top / (int_t)UIApplication.sharedApplication.windows.firstObject.bounds.size.height;
	int_t right = _RECT.width * (int_t)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.right / (int_t)UIApplication.sharedApplication.windows.firstObject.bounds.size.width;
	int_t bottom = _RECT.height * (int_t)UIApplication.sharedApplication.windows.firstObject.safeAreaInsets.bottom / (int_t)UIApplication.sharedApplication.windows.firstObject.bounds.size.height;
	_SAFE_RECT.x += left;
	_SAFE_RECT.y += top;
	_SAFE_RECT.width -= (left + right);
	_SAFE_RECT.height -= (top + bottom);
#endif
	
	// Adjust the rect and safe rect if they are not sized correctly for the preferred rect
	// The preferred rect should entirely fit within the safe rect, and maximize the safe rect to that size
	if(_SAFE_RECT.width != _PREFERRED_RECT.width) {
		_RECT.x = _RECT.x * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_RECT.y = _RECT.y * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_RECT.width = _RECT.width * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_RECT.height = _RECT.height * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_SAFE_RECT.x = _SAFE_RECT.x * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_SAFE_RECT.y = _SAFE_RECT.y * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_SAFE_RECT.height = _SAFE_RECT.height * _PREFERRED_RECT.width / _SAFE_RECT.width;
		_SAFE_RECT.width = _PREFERRED_RECT.width; // This must be last because it is used for the aspect ratio
	}
	if(_SAFE_RECT.height < _PREFERRED_RECT.height) {
		_RECT.x = _RECT.x * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_RECT.y = _RECT.y * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_RECT.width = _RECT.width * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_RECT.height = _RECT.height * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_SAFE_RECT.x = _SAFE_RECT.x * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_SAFE_RECT.y = _SAFE_RECT.y * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_SAFE_RECT.width = _SAFE_RECT.width * _PREFERRED_RECT.height / _SAFE_RECT.height;
		_SAFE_RECT.height = _PREFERRED_RECT.height; // This must be last because it is used for the aspect ratio
	}
	
	// Center the preferred rect within the screen rect then adjust to fit within the safe rect
	_PREFERRED_RECT.x = _RECT.width / 2 - _PREFERRED_RECT.width / 2;
	_PREFERRED_RECT.y = _RECT.height / 2 - _PREFERRED_RECT.height / 2;
	if(_PREFERRED_RECT.x < _SAFE_RECT.x)
		_PREFERRED_RECT.x = _SAFE_RECT.x;
	if(_PREFERRED_RECT.y < _SAFE_RECT.y)
		_PREFERRED_RECT.y = _SAFE_RECT.y;
	if(_PREFERRED_RECT.x + _PREFERRED_RECT.width > _SAFE_RECT.x + _SAFE_RECT.width)
		_PREFERRED_RECT.x = _SAFE_RECT.x + _SAFE_RECT.width - _PREFERRED_RECT.width;
	if(_PREFERRED_RECT.y + _PREFERRED_RECT.height > _SAFE_RECT.y + _SAFE_RECT.height)
		_PREFERRED_RECT.y = _SAFE_RECT.y + _SAFE_RECT.height - _PREFERRED_RECT.height;
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

#if PLATFORM_MACOSX

- (void) mouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) rightMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) otherMouseDown: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) mouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseUpCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchUpCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) rightMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseUpCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchUpCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) otherMouseUp: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseUpCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchUpCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) mouseMoved: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseMoveCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) mouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseDragCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchMoveCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) rightMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseDragCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchMoveCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) otherMouseDragged: (NSEvent*)theEvent {
	NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	location.y = self.frame.size.height - location.y;
	location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
	location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
	GSystem::RunMouseDragCallbacks((int_t)location.x, (int_t)location.y, (int_t)[theEvent buttonNumber]);
	GSystem::RunTouchMoveCallbacks((int_t)location.x, (int_t)location.y);
}

- (void) scrollWheel: (NSEvent*)theEvent {
	GSystem::RunMouseWheelCallbacks((float_t)[theEvent deltaX], (float_t)[theEvent deltaY]);
}

- (void) keyDown: (NSEvent*)theEvent {
	if([theEvent isARepeat] == NO)
		GSystem::RunKeyCallbacks((vkey_t)[theEvent keyCode]);
	if([[theEvent characters] length] > 0 && isprint((int)[[theEvent characters] UTF8String][0]))
		GSystem::RunASCIICallbacks((char)[[theEvent characters] UTF8String][0]);
	//wchar_t key = [[theEvent characters] characterAtIndex:0];
}

- (void) keyUp: (NSEvent*)theEvent {
	if([theEvent isARepeat] == NO)
		GSystem::RunKeyUpCallbacks((vkey_t)[theEvent keyCode]);
}

- (void) flagsChanged: (NSEvent*)theEvent {
	
	//unsigned long cmods = [theEvent modifierFlags];
	//if ((cmods & 0xffff0000) != _Modifiers)
	//{
	//   uint32_t mods = MapModifiers(cmods);
	//  for (int i = 1; i <= 4; i++)
	// {
	//    unsigned long m = (1 << (16+i));
	//     if ((cmods & m) != (_Modifiers & m))
	//     {
	//         if (cmods & m)
	//             _App->OnKey(ModifierKeys[i], 0, true, mods); // Key Down
	//         else
	//             _App->OnKey(ModifierKeys[i], 0, false, mods); // Key Up
	//     }
	// }
	//    _Modifiers = cmods & 0xffff0000;
	//}
	
	//GetCurrentKeyModifiers
	//GetCurrentEventKeyModifiers
	//kVK_Shift
	//kVK_RightShift
	//printf("0x%x %d\n", [theEvent keyCode], (bool)([theEvent modifierFlags] & NSShiftKeyMask));
}

#endif // PLATFORM_MACOSX

#if PLATFORM_IOS

- (void) touchesBegan: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
		GSystem::RunTouchCallbacks((int_t)location.x, (int_t)location.y);
	}
}

- (void) touchesMoved: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
		GSystem::RunTouchMoveCallbacks((int_t)location.x, (int_t)location.y);
	}
}

- (void) touchesEnded: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int_t)location.x, (int_t)location.y);
	}
}

- (void) touchesCancelled: (NSSet*)touches withEvent:(UIEvent*)event {
	for(UITouch* touch in touches) {
		CGPoint location = [touch locationInView:self];
		location.x = location.x * (CGFloat)_RECT.width / self.frame.size.width;
		location.y = location.y * (CGFloat)_RECT.height / self.frame.size.height;
		GSystem::RunTouchUpCallbacks((int_t)location.x, (int_t)location.y);
	}
}

#endif // PLATFORM_IOS


@end // _MyMetalView
























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
	GConsole::Debug("WD: %s\n", getwd(NULL));
	
#if PLATFORM_MACOSX
	@autoreleasepool {
		[NSApplication sharedApplication];
		_MyAppDelegate* delegate = [[_MyAppDelegate alloc] init];
		[[NSApplication sharedApplication] setDelegate:delegate];
		[[NSApplication sharedApplication] run];
	}
#endif
	
#if PLATFORM_IOS
	@autoreleasepool {
		return UIApplicationMain((int)_ARG_C, _ARG_V, nil, NSStringFromClass([_MyAppDelegate class]));
	}
#endif
	
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
	if(_RENDER != nil) {
		static matrix_float4x4 MATRIX;
		MATRIX = matrix_multiply(_PROJECTION_MATRIX, _MODEL_MATRIX);
		[_RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	}
}

#endif // PLATFORM_MACOSX // PLATFORM_IOS
