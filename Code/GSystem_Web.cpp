#import "GSystem.h"
#if PLATFORM_WEB

static GRect			_RECT			(0, 0, 1280, 720);
static GRect			_SAFE_RECT		(0, 0, 1280, 720);
static GRect			_PREFERRED_RECT	(0, 0, 1280, 720);
static int_t			_FPS		    = 60;
static int_t			_ARG_C			= 0;
static char**			_ARG_V			= NULL;
static EGLDisplay		_DISPLAY		= NULL;
static EGLSurface		_SURFACE		= NULL;
static EGLContext		_CONTEXT		= NULL;
static GLuint			_PROGRAM		= 0;
static GLint			_SHADER_MATRIX	= 0;
GLint					_SHADER_XY		= 0; // Non-static to allow for extern access from GImage
GLint					_SHADER_RGBA	= 0; // Non-static to allow for extern access from GImage
GLint					_SHADER_UV		= 0; // Non-static to allow for extern access from GImage
static GLint			_SHADER_TEXTURE	= 0;
static GMatrix32_4x4	_MODEL_MATRIX;
static GMatrix32_4x4	_PROJECTION_MATRIX;



static void _MAIN_LOOP (void* arg) {
	eglMakeCurrent(_DISPLAY, _SURFACE, _SURFACE, _CONTEXT);
	glViewport(0, 0, _RECT.width, _RECT.height);
#if DEBUG
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
#else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#endif
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(_PROGRAM);
	GSystem::MatrixSetModelDefault();
	GSystem::MatrixSetProjectionDefault();
	GSystem::MatrixUpdate();
	GSystem::RunDrawCallbacks();
	eglSwapBuffers(_DISPLAY, _SURFACE);
}

static EM_BOOL _MOUSE_DOWN_CALLBACK (int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) {
	GSystem::RunMouseCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY, (int_t)mouseEvent->buttons - 1);
	GSystem::RunTouchCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY);
	return true;
}

static EM_BOOL _MOUSE_UP_CALLBACK (int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) {
	GSystem::RunMouseUpCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY, (int_t)mouseEvent->buttons - 1);
	GSystem::RunTouchUpCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY);
	return true;
}

static EM_BOOL _MOUSE_MOVE_CALLBACK (int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData) {
	if(mouseEvent->buttons > 0) {
		GSystem::RunMouseDragCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY, (int_t)mouseEvent->buttons - 1);
		GSystem::RunTouchMoveCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY);
	} else {
		GSystem::RunMouseMoveCallbacks((int_t)mouseEvent->targetX, (int_t)mouseEvent->targetY);
	}
	return true;
}

static EM_BOOL _MOUSE_WHEEL_CALLBACK (int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData) {
	GSystem::RunMouseWheelCallbacks((float_t)wheelEvent->deltaX, (float_t)wheelEvent->deltaY);
	return true;
}

static EM_BOOL _KEY_DOWN_CALLBACK (int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
	// TODO: These do not work yet, but should probably be evaluated with the Mac and Windows versions
	if(keyEvent->repeat == false)
		GSystem::RunKeyCallbacks((vkey_t)keyEvent->keyCode);
	//if([[theEvent characters] length] > 0 && isprint((int)[[theEvent characters] UTF8String][0]))
	//	GSystem::RunASCIICallbacks((char)[[theEvent characters] UTF8String][0]);
	return false;
}

static EM_BOOL _KEY_UP_CALLBACK (int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
	// TODO: These do not work yet, but should probably be evaluated with the Mac and Windows versions
	if(keyEvent->repeat == false)
		GSystem::RunKeyUpCallbacks((vkey_t)keyEvent->keyCode);
	return false;
}

static EM_BOOL _TOUCH_START_CALLBACK (int eventType, const EmscriptenTouchEvent* touchEvent, void* userData) {
	for(int i = 0; i < touchEvent->numTouches; i++)
		GSystem::RunTouchCallbacks((int_t)touchEvent->touches[i].targetX, (int_t)touchEvent->touches[i].targetY);
	return true;
}

static EM_BOOL _TOUCH_MOVED_CALLBACK (int eventType, const EmscriptenTouchEvent* touchEvent, void* userData) {
	for(int i = 0; i < touchEvent->numTouches; i++)
		GSystem::RunTouchMoveCallbacks((int_t)touchEvent->touches[i].targetX, (int_t)touchEvent->touches[i].targetY);
	return true;
}

static EM_BOOL _TOUCH_END_CALLBACK (int eventType, const EmscriptenTouchEvent* touchEvent, void* userData) {
	for(int i = 0; i < touchEvent->numTouches; i++)
		GSystem::RunTouchUpCallbacks((int_t)touchEvent->touches[i].targetX, (int_t)touchEvent->touches[i].targetY);
	return true;
}

static EM_BOOL _TOUCH_CANCEL_CALLBACK (int eventType, const EmscriptenTouchEvent* touchEvent, void* userData) {
	for(int i = 0; i < touchEvent->numTouches; i++)
		GSystem::RunTouchUpCallbacks((int_t)touchEvent->touches[i].targetX, (int_t)touchEvent->touches[i].targetY);
	return true;
}






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
	struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (uint64)ts.tv_sec * 1000L + (uint64)ts.tv_nsec;
	//return (uint64)emscripten_get_now(); // Not sure if timespec should be replaced with emscripten_get_now
}

uint64 GSystem::GetMicroseconds () {
	struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (uint64)ts.tv_sec * 1000000L + (uint64)ts.tv_nsec;
}

uint64 GSystem::GetNanoseconds () {
	struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (uint64)ts.tv_sec * 1000000000L + (uint64)ts.tv_nsec;
}

void GSystem::SetDefaultWD () {
	// This does nothing for web because the files must be preloaded
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





static GLuint _LOAD_SHADER ( GLenum type, const char *shaderSrc ) {
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );
   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 ) {
         char* infoLog = (char*)malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         GConsole::Print ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}




int_t GSystem::Run () {
	
	_DISPLAY = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(_DISPLAY == EGL_NO_DISPLAY) {
		GConsole::Print("Could not find OpenGL display!\n");
		return EXIT_FAILURE;
	}
	
	EGLint majorVersion;
	EGLint minorVersion;
	EGLBoolean result = eglInitialize(_DISPLAY, &majorVersion, &minorVersion);
	if(result != EGL_TRUE) {
		GConsole::Print("Could not initialize OpenGL display!\n");
		return EXIT_FAILURE;
	}
	
	EGLint attribs[] = {
		EGL_RED_SIZE,       8,
		EGL_GREEN_SIZE,     8,
		EGL_BLUE_SIZE,      8,
		EGL_ALPHA_SIZE,     8,
		EGL_DEPTH_SIZE,     8,
		EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	
	result = eglChooseConfig(_DISPLAY, attribs, &config, 1, &numConfigs);
	if(result != EGL_TRUE) {
		GConsole::Print("Could not choose OpenGL configuration!\n");
		return EXIT_FAILURE;
	}
		
	_SURFACE = eglCreateWindowSurface(_DISPLAY, config, EGL_DEFAULT_DISPLAY, NULL);
	if(_SURFACE == EGL_NO_SURFACE) {
		GConsole::Print("Could not create OpenGL window surface!\n");
		return EXIT_FAILURE;
	}
	
	EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, 
		EGL_NONE, EGL_NONE
	};
	_CONTEXT = eglCreateContext(_DISPLAY, config, EGL_NO_CONTEXT, contextAttribs);
	if(_CONTEXT == EGL_NO_CONTEXT) {
		GConsole::Print("Could not create OpenGL context!\n");
		return EXIT_FAILURE;
	}
	
	result = eglMakeCurrent(_DISPLAY, _SURFACE, _SURFACE, _CONTEXT);
	if(result != EGL_TRUE) {
		GConsole::Print("Could not make OpenGL display, surface, and context current!\n");
		return EXIT_FAILURE;
	}
	
	_RECT = _PREFERRED_RECT;
	_SAFE_RECT = _PREFERRED_RECT;
	EMSCRIPTEN_RESULT emsResult = emscripten_set_canvas_element_size("#canvas", _RECT.width, _RECT.height);
	if(emsResult != EMSCRIPTEN_RESULT_SUCCESS) {
		GConsole::Print("Could not set canvas element size!\n");
		return EXIT_FAILURE;
	}
	
#if DEBUG
	EM_ASM(Module['canvas'].style.backgroundColor = 'green';);
#endif
	
	// Add callbacks for mouse, keyboard, and touch events
	emsResult = emscripten_set_mousedown_callback("#canvas", NULL, true, _MOUSE_DOWN_CALLBACK);
	emsResult = emscripten_set_mouseup_callback("#canvas", NULL, true, _MOUSE_UP_CALLBACK);
	emsResult = emscripten_set_mousemove_callback("#canvas", NULL, true, _MOUSE_MOVE_CALLBACK);
	emsResult = emscripten_set_wheel_callback("#canvas", NULL, true, _MOUSE_WHEEL_CALLBACK);
	emsResult = emscripten_set_keydown_callback("#canvas", NULL, true, _KEY_DOWN_CALLBACK);
	emsResult = emscripten_set_keyup_callback("#canvas", NULL, true, _KEY_UP_CALLBACK);
	emsResult = emscripten_set_touchstart_callback("#canvas", NULL, true, _TOUCH_START_CALLBACK);
	emsResult = emscripten_set_touchmove_callback("#canvas", NULL, true, _TOUCH_MOVED_CALLBACK);
	emsResult = emscripten_set_touchend_callback("#canvas", NULL, true, _TOUCH_END_CALLBACK);
	emsResult = emscripten_set_touchcancel_callback("#canvas", NULL, true, _TOUCH_CANCEL_CALLBACK);
	
	// These might be needed for the key events, but for now I am moving to other events
	//EM_ASM(Module['canvas'].tabindex = '1';);
	//EM_ASM(Module['canvas'].focus(););
	
	
	const char* vertexString = 
	"uniform mat4 u_matrix;\n"
	"attribute vec2 in_xy;\n"
	"attribute vec4 in_rgba;\n"
	"attribute vec2 in_uv;\n"
	"varying vec4 out_rgba;\n"
	"varying vec2 out_uv;\n"
	"void main() {\n"
	"	gl_Position = u_matrix * vec4(in_xy.xy, 0, 1);\n"
	"	out_rgba = in_rgba;\n"
	"	out_uv = in_uv;\n"
	"}\n";
	
	const char* fragmentString = 
	"precision mediump float;\n"
	"varying vec4 out_rgba;\n"
	"varying vec2 out_uv;\n"
	"uniform sampler2D u_texture;\n"
	"void main() {\n"
	"	gl_FragColor = out_rgba * texture2D(u_texture, out_uv);\n"
	"}\n";
	
	_PROGRAM = glCreateProgram();
	if(_PROGRAM == 0) {
		GConsole::Print("Could not create OpenGL program object!\n");
		return EXIT_FAILURE;
	}
	
	GLuint vertexShader = _LOAD_SHADER(GL_VERTEX_SHADER, vertexString);
	glAttachShader(_PROGRAM, vertexShader);
	
	GLuint fragmentShader = _LOAD_SHADER(GL_FRAGMENT_SHADER, fragmentString);
	glAttachShader(_PROGRAM, fragmentShader);
	
	//glBindAttribLocation(_PROGRAM, 0, "vPosition");
	glLinkProgram(_PROGRAM);
	
	// Setup remaining OpenGL functions
	glViewport(0, 0, _RECT.width, _RECT.height);
	glActiveTexture(GL_TEXTURE0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	// Setup OpenGL Shaders
	glUseProgram(_PROGRAM);
	_SHADER_MATRIX = glGetUniformLocation(_PROGRAM, "u_matrix");
	_SHADER_XY = glGetAttribLocation(_PROGRAM, "in_xy");
	_SHADER_RGBA = glGetAttribLocation(_PROGRAM, "in_rgba");
	_SHADER_UV = glGetAttribLocation(_PROGRAM, "in_uv");
	_SHADER_TEXTURE = glGetUniformLocation(_PROGRAM, "u_texture");
	glUniform1i(_SHADER_TEXTURE, 0);
	
	// Run the main event loop
	emscripten_set_main_loop_arg(_MAIN_LOOP, NULL, _FPS, 1);
	
	// These are actually never reached, but this is how cleanup is handled
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(_PROGRAM);
	eglMakeCurrent(_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(_DISPLAY, _CONTEXT);
	eglDestroySurface(_DISPLAY, _SURFACE);
	eglTerminate(_DISPLAY);
	
	return EXIT_SUCCESS;
}














void GSystem::MatrixSetModelDefault () {
	_MODEL_MATRIX.SetIdentity();
}

void GSystem::MatrixSetProjectionDefault () {
	_PROJECTION_MATRIX.SetOrtho2D((float32)_RECT.x, (float32)_RECT.width, (float32)_RECT.height, (float32)_RECT.y, (float32)-1, (float32)1);
}

void GSystem::MatrixTranslateModel (float_t x, float_t y) {
	_MODEL_MATRIX.SetTranslation((float32)x, (float32)y, (float32)0);
}

void GSystem::MatrixTranslateProjection (float_t x, float_t y) {
	_PROJECTION_MATRIX.SetTranslation((float32)x, (float32)y, (float32)0);
}

void GSystem::MatrixScaleModel (float_t x, float_t y) {
	_MODEL_MATRIX.SetScale((float32)x, (float32)y, (float32)1);
}

void GSystem::MatrixScaleProjection (float_t x, float_t y) {
	_PROJECTION_MATRIX.SetScale((float32)x, (float32)y, (float32)1);
}

void GSystem::MatrixRotateModel (float_t degrees) {
	_MODEL_MATRIX.SetRotation((float32)degrees * ((float32)M_PI / (float32)180));
}

void GSystem::MatrixRotateProjection (float_t degrees) {
	_PROJECTION_MATRIX.SetRotation((float32)degrees * ((float32)M_PI / (float32)180));
}

void GSystem::MatrixUpdate () {
	if(_PROGRAM != 0) {
		static GMatrix32_4x4 MATRIX;
		MATRIX = _PROJECTION_MATRIX * _MODEL_MATRIX;
		glUniformMatrix4fv(_SHADER_MATRIX, 1, GL_FALSE, &MATRIX.numbers[0][0]);
	}
}

#endif // PLATFORM_WEB
