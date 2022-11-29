#import "GSystem.h"
#ifdef __BIONIC__
#include <jni.h>
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <game-activity/native_app_glue/android_native_app_glue.c>
#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

static GRect			SCREEN_RECT;
static GRect			SAFE_RECT;
static GRect			PREFERRED_RECT;
static int				FPS = 60;
static android_app*		ANDROID_APP = nullptr;
static GMatrix32_4x4	MODEL_MATRIX;
static GMatrix32_4x4	PROJECTION_MATRIX;
static EGLDisplay       DISPLAY = EGL_NO_DISPLAY;
static EGLSurface		SURFACE = EGL_NO_SURFACE;
static EGLContext		CONTEXT = EGL_NO_CONTEXT;
static GLuint			PROGRAM = 0;
static GLuint           VERTEX_SHADER = 0;
static GLuint           FRAGMENT_SHADER = 0;
static GLint			SHADER_MATRIX = 0;
GLint					SHADER_XY = 0; // Non-static to allow for extern access from GImage
GLint					SHADER_RGBA = 0; // Non-static to allow for extern access from GImage
GLint					SHADER_UV = 0; // Non-static to allow for extern access from GImage
static GLint			SHADER_TEXTURE = 0;

extern int main (int argc, char* argv[]);

static GLuint AndroidLoadShader (GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if(shader) {
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if(!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen > 1) {
                auto* infoLog = new GLchar[infoLen + 1];
                glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                GSystem::Print("Failed to compile shader: %s\n", infoLog);
                delete [] infoLog;
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

static void AndroidStartupOpenGL () {
    DISPLAY = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(DISPLAY == EGL_NO_DISPLAY) {
        GSystem::Print("Could not find OpenGL display!\n");
        return;
    }

    EGLBoolean result = eglInitialize(DISPLAY, nullptr, nullptr);
    if(result != EGL_TRUE) {
        GSystem::Print("Could not initialize OpenGL display!\n");
        return;
    }

    constexpr EGLint attribs[] = {
            //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE,       8,
            EGL_GREEN_SIZE,     8,
            EGL_BLUE_SIZE,      8,
            EGL_ALPHA_SIZE,     8,
            EGL_DEPTH_SIZE,     8,
            EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    result = eglChooseConfig(DISPLAY, attribs, &config, 1, &numConfigs);
    if(result != EGL_TRUE || numConfigs != 1) {
        GSystem::Print("Could not choose OpenGL configuration!\n");
        return;
    }

    SURFACE = eglCreateWindowSurface(DISPLAY, config, ANDROID_APP->window, nullptr);
    if(SURFACE == EGL_NO_SURFACE) {
        GSystem::Print("Could not create OpenGL window surface!\n");
        return;
    }

    constexpr EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    CONTEXT = eglCreateContext(DISPLAY, config, EGL_NO_CONTEXT, contextAttribs);
    if(CONTEXT == EGL_NO_CONTEXT) {
        GSystem::Print("Could not create OpenGL context!\n");
        return;
    }

    result = eglMakeCurrent(DISPLAY, SURFACE, SURFACE, CONTEXT);
    if(result != EGL_TRUE) {
        GSystem::Print("Could not make OpenGL display, surface, and context current!\n");
        return;
    }

#if DEBUG
    GSystem::Debug("OpenGL Vendor=%s Renderer=%s Version=%s\n", (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_VERSION));
#endif

    // Find the screen size
    SCREEN_RECT.x = ANDROID_APP->contentRect.left;
    SCREEN_RECT.y = ANDROID_APP->contentRect.top;
    SCREEN_RECT.width = ANDROID_APP->contentRect.right - ANDROID_APP->contentRect.left;
    SCREEN_RECT.height = ANDROID_APP->contentRect.bottom - ANDROID_APP->contentRect.top;

    // Find the safe area within the screen
    SAFE_RECT = SCREEN_RECT; // TODO: This needs to be adjusted for the insets and navigation areas

    // If no preferred rect is set, use the safe rect as the preferred rect
    if(PREFERRED_RECT.width == 0 || PREFERRED_RECT.height == 0) {
        PREFERRED_RECT.x = 0;
        PREFERRED_RECT.y = 0;
        PREFERRED_RECT.width = SAFE_RECT.width;
        PREFERRED_RECT.height = SAFE_RECT.height;
    }

    // Adjust the rect and safe rect if they are not sized correctly for the preferred rect
    // The preferred rect should entirely fit within the safe rect, and maximize the safe rect to that size
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

    // Create the main shader object
    PROGRAM = glCreateProgram();
    if(PROGRAM == 0) {
        GSystem::Print("Could not create OpenGL program object!\n");
        return;
    }

    constexpr const char* VERTEX_STRING =
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
    VERTEX_SHADER = AndroidLoadShader(GL_VERTEX_SHADER, VERTEX_STRING);
    glAttachShader(PROGRAM, VERTEX_SHADER);

    constexpr const char* FRAGMENT_STRING =
            "precision mediump float;\n"
            "varying vec4 out_rgba;\n"
            "varying vec2 out_uv;\n"
            "uniform sampler2D u_texture;\n"
            "void main() {\n"
            "	gl_FragColor = out_rgba * texture2D(u_texture, out_uv);\n"
            "}\n";
    FRAGMENT_SHADER = AndroidLoadShader(GL_FRAGMENT_SHADER, FRAGMENT_STRING);
    glAttachShader(PROGRAM, FRAGMENT_SHADER);

    glLinkProgram(PROGRAM);

    // Setup remaining OpenGL functions
    glViewport(0, 0, SCREEN_RECT.width, SCREEN_RECT.height);
    glActiveTexture(GL_TEXTURE0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Setup OpenGL Shaders
    glUseProgram(PROGRAM);
    SHADER_MATRIX = glGetUniformLocation(PROGRAM, "u_matrix");
    SHADER_XY = glGetAttribLocation(PROGRAM, "in_xy");
    SHADER_RGBA = glGetAttribLocation(PROGRAM, "in_rgba");
    SHADER_UV = glGetAttribLocation(PROGRAM, "in_uv");
    SHADER_TEXTURE = glGetUniformLocation(PROGRAM, "u_texture");
    glUniform1i(SHADER_TEXTURE, 0);
}

static void AndroidShutdownOpenGL () {
    glDeleteShader(VERTEX_SHADER);
    glDeleteShader(FRAGMENT_SHADER);
    glDeleteProgram(PROGRAM);
    eglMakeCurrent(DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(DISPLAY, CONTEXT);
    eglDestroySurface(DISPLAY, SURFACE);
    eglTerminate(DISPLAY);
}

static void AndroidCommandHandler (android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            AndroidStartupOpenGL();
            break;
        case APP_CMD_TERM_WINDOW:
            AndroidShutdownOpenGL();
            break;
        default:
            break;
    }
}

static void AndroidInputHandler () {
    for(int i = 0; i < ANDROID_APP->motionEventsCount; i++) {

    }
}

static void AndroidGraphicsHandler () {
    eglMakeCurrent(DISPLAY, SURFACE, SURFACE, CONTEXT);
    glViewport(0, 0, SCREEN_RECT.width, SCREEN_RECT.height);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(PROGRAM);
    GSystem::MatrixSetModelDefault();
    GSystem::MatrixSetProjectionDefault();
    GSystem::MatrixUpdate();
    GSystem::RunDrawCallbacks();
    eglSwapBuffers(DISPLAY, SURFACE);
}




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
    // Android's working directory is root, so it should not be used
    // and the asset manager should be used, which is setup in GFile
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
    // Nothing is needed from this on Android
}

int GSystem::Run () {
    GSystem::Debug("Running Android Application...\n");

    ANDROID_APP->onAppCmd = AndroidCommandHandler;
    int events;
    android_poll_source* source;
    while(!ANDROID_APP->destroyRequested) {
        if (ALooper_pollAll(0, nullptr, &events, (void **) &source) >= 0 && source)
            source->process(ANDROID_APP, source);
        AndroidInputHandler();
        AndroidGraphicsHandler();
    }
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
    if(PROGRAM != 0) {
        GMatrix32_4x4 matrix = PROJECTION_MATRIX * MODEL_MATRIX;
        //glUniformMatrix4fv(SHADER_MATRIX, 1, GL_FALSE, &matrix.numbers[0][0]);
        glUniformMatrix4fv(SHADER_MATRIX, 1, GL_FALSE, &matrix.numbers[0][0]);
    }
}

void GSystem::Print (const char* message, ...) {
    if (message) {
        va_list args;
        va_start(args, message);
        int size = vsnprintf(nullptr, 0, message, args);
        va_end(args);
        if (size > 0) {
            char string[size + 1];
            va_start(args, message);
            vsnprintf(string, size + 1, message, args);
            va_end(args);
            string[size] = '\0';
            __android_log_print(ANDROID_LOG_INFO, "Game-API", "%s", string);
        }
    }
}

void GSystem::Debug (const char* message, ...) {
#if DEBUG
    if (message) {
        va_list args;
        va_start(args, message);
        int size = vsnprintf(nullptr, 0, message, args);
        va_end(args);
        if (size > 0) {
            char string[size + 1];
            va_start(args, message);
            vsnprintf(string, size + 1, message, args);
            va_end(args);
            string[size] = '\0';
            __android_log_print(ANDROID_LOG_DEBUG, "Game-API", "%s", string);
        }
    }
#endif
}

void android_main (struct android_app* app) {
    ANDROID_APP = app;
    main(0, nullptr); // This is done for compatibility with other platforms
}

#endif // __BIONIC__
