#import "GSystem.h"
#if PLATFORM_WEB

static GRect			_RECT			(0, 0, 1280, 720);
static GRect			_SAFE_RECT		(0, 0, 1280, 720);
static GRect			_PREFERRED_RECT	(0, 0, 1280, 720);
static int_t			_FPS		    = -1; // Call the draw function as fast as the browser prefers
static int_t			_ARG_C			= 0;
static char**			_ARG_V			= NULL;
static EGLDisplay		_DISPLAY		= NULL;
static EGLSurface		_SURFACE		= NULL;
static EGLContext		_CONTEXT		= NULL;
static GLuint			_PROGRAM		= 0;
static GMatrix32_4x4	_MODEL_MATRIX;
static GMatrix32_4x4	_PROJECTION_MATRIX;






static void _MAIN_LOOP (void* arg) {
	eglMakeCurrent(_DISPLAY, _SURFACE, _SURFACE, _CONTEXT);
	GSystem::MatrixSetModelDefault();
	GSystem::MatrixSetProjectionDefault();
	GSystem::MatrixUpdate();
	GSystem::RunDrawCallbacks();
	eglSwapBuffers(_DISPLAY, _SURFACE);
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
	chdir("Resources/");
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





static GLuint _LOAD_SHADER ( GLenum type, const char *shaderSrc )
{
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

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
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
	
	GSystem::SetDefaultWD();
	char cwd[PATH_MAX] = "";
	getcwd(cwd, PATH_MAX);
	GConsole::Debug("WD: %s\n", cwd);
	
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
	
	
	//EMSCRIPTEN_RESULT ret = emscripten_set_click_callback("#canvas", 0, 1, mouse_callback);
	//ret = emscripten_set_wheel_callback("#canvas", 0, 1, wheel_callback);
	
	
	
	// https://learnopengl.com/Getting-started/Textures
	
	const char* vertexString = 
	"attribute vec4 vPosition;    \n"
	"void main()                  \n"
	"{                            \n"
	"	gl_Position = vPosition;  \n"
	"}                            \n";
	
	const char* fragmentString = 
	"precision mediump float;\n"\
	"void main()                                  \n"
	"{                                            \n"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);  \n"
	"}                                            \n";
	
	_PROGRAM = glCreateProgram();
	if(_PROGRAM == 0) {
		GConsole::Print("Could not create OpenGL program object!\n");
		return EXIT_FAILURE;
	}
	
	GLuint vertexShader = _LOAD_SHADER(GL_VERTEX_SHADER, vertexString);
	glAttachShader(_PROGRAM, vertexShader);
	
	GLuint fragmentShader = _LOAD_SHADER(GL_FRAGMENT_SHADER, fragmentString);
	glAttachShader(_PROGRAM, fragmentShader);
	
	glBindAttribLocation(_PROGRAM, 0, "vPosition");
	glLinkProgram(_PROGRAM);
	
	
	// Setup remaining OpenGL functions
	glViewport(0, 0, _RECT.width, _RECT.height);
	//glEnableClientState(GL_VERTEX_ARRAY); // -s LEGACY_GL_EMULATION=1
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrthof(0.0f, (float)_RECT.width, (float)_RECT.height, 0.0f, -32768.0f, 32768.0f);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	glUseProgram(_PROGRAM);
	
	// Run the main event loop
	emscripten_set_main_loop_arg(_MAIN_LOOP, NULL, _FPS, 1);
	
	// These are actually never reached, but this is how cleanup is handled
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
	//if(_RENDER != nil) {
	//	static GMatrix32_4x4 MATRIX;
	//	MATRIX = _PROJECTION_MATRIX * _MODEL_MATRIX;
	//	[_RENDER setVertexBytes:&MATRIX length:sizeof(MATRIX) atIndex:1];
	//}
}

#endif // PLATFORM_WEB
