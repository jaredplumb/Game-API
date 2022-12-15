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
#include <unordered_map>



static GLint			SCREEN_NATIVE_WIDTH = 0;
static GLint			SCREEN_NATIVE_HEIGHT = 0;
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
                GSystem::Debug("Failed to compile shader: %s\n", infoLog);
                delete [] infoLog;
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}



static void AndroidStartupOpenGL () {

    //ANativeWindow_setFrameRate(ANDROID_APP->window, (float)FPS, ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_DEFAULT);

    DISPLAY = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(DISPLAY == EGL_NO_DISPLAY) {
        GSystem::Debug("Could not find OpenGL display!\n");
        return;
    }

    EGLBoolean result = eglInitialize(DISPLAY, nullptr, nullptr);
    if(result != EGL_TRUE) {
        GSystem::Debug("Could not initialize OpenGL display!\n");
        return;
    }

    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
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
        GSystem::Debug("Could not choose OpenGL configuration!\n");
        return;
    }

    SURFACE = eglCreateWindowSurface(DISPLAY, config, ANDROID_APP->window, nullptr);
    if(SURFACE == EGL_NO_SURFACE) {
        GSystem::Debug("Could not create OpenGL window surface!\n");
        return;
    }

    constexpr EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    CONTEXT = eglCreateContext(DISPLAY, config, EGL_NO_CONTEXT, contextAttribs);
    if(CONTEXT == EGL_NO_CONTEXT) {
        GSystem::Debug("Could not create OpenGL context!\n");
        return;
    }

    result = eglMakeCurrent(DISPLAY, SURFACE, SURFACE, CONTEXT);
    if(result != EGL_TRUE) {
        GSystem::Debug("Could not make OpenGL display, surface, and context current!\n");
        return;
    }

#if DEBUG
    GSystem::Debug("OpenGL Vendor=%s Renderer=%s Version=%s\n", (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER), (const char*)glGetString(GL_VERSION));
#endif

    // Find the screen size
    eglQuerySurface(DISPLAY, SURFACE, EGL_WIDTH, &SCREEN_NATIVE_WIDTH);
    eglQuerySurface(DISPLAY, SURFACE, EGL_HEIGHT, &SCREEN_NATIVE_HEIGHT);
    SCREEN_RECT.x = 0;
    SCREEN_RECT.y = 0;
    SCREEN_RECT.width = SCREEN_NATIVE_WIDTH;
    SCREEN_RECT.height = SCREEN_NATIVE_HEIGHT;

    // Find the safe area within the screen
    ARect windowInsets;
    GameActivity_getWindowInsets(ANDROID_APP->activity, GAMECOMMON_INSETS_TYPE_SYSTEM_BARS, &windowInsets);
    SAFE_RECT.x = windowInsets.left;
    SAFE_RECT.y = windowInsets.top;
    SAFE_RECT.width = SCREEN_RECT.width - windowInsets.left - windowInsets.right;
    SAFE_RECT.height = SCREEN_RECT.height - windowInsets.top - windowInsets.bottom;

    // If no preferred rect is set, use the safe rect as the preferred rect
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







    // Create the main shader object
    PROGRAM = glCreateProgram();
    if(PROGRAM == 0) {
        GSystem::Debug("Could not create OpenGL program object!\n");
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
    glViewport(0, 0, SCREEN_NATIVE_WIDTH, SCREEN_NATIVE_HEIGHT);
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



static bool AndroidMotionEventFilter (const GameActivityMotionEvent* event) {
    if(event) {
        const GameActivityPointerAxes& pointer = event->pointers[(event->action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT];
        int x = (int)GameActivityPointerAxes_getX(&pointer) * SCREEN_RECT.width / SCREEN_NATIVE_WIDTH;
        int y = (int)GameActivityPointerAxes_getY(&pointer) * SCREEN_RECT.height / SCREEN_NATIVE_HEIGHT;
        switch(event->action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                GSystem::RunTouchCallbacks(x - PREFERRED_RECT.x, y - PREFERRED_RECT.y);
                break;
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_CANCEL:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                GSystem::RunTouchUpCallbacks(x - PREFERRED_RECT.x, y - PREFERRED_RECT.y);
                break;
            case AMOTION_EVENT_ACTION_MOVE:
            case AMOTION_EVENT_ACTION_HOVER_MOVE:
                GSystem::RunTouchMoveCallbacks(x - PREFERRED_RECT.x, y - PREFERRED_RECT.y);
                break;
            default:
                break;
        }
    }
    return false;
}



static void AndroidGraphicsHandler () {
    if(DISPLAY != EGL_NO_DISPLAY && SURFACE != EGL_NO_SURFACE && CONTEXT != EGL_NO_CONTEXT) {
        eglMakeCurrent(DISPLAY, SURFACE, SURFACE, CONTEXT);
        // TODO: These two queries should be moved to the updated screen size function
        eglQuerySurface(DISPLAY, SURFACE, EGL_WIDTH, &SCREEN_NATIVE_WIDTH);
        eglQuerySurface(DISPLAY, SURFACE, EGL_HEIGHT, &SCREEN_NATIVE_HEIGHT);
        glViewport(0, 0, SCREEN_NATIVE_WIDTH, SCREEN_NATIVE_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(PROGRAM);
        GSystem::MatrixSetModelDefault();
        GSystem::MatrixSetProjectionDefault();
        GSystem::MatrixUpdate();
        GSystem::RunDrawCallbacks();
        eglSwapBuffers(DISPLAY, SURFACE);
    }
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



static FILE* PACKAGE_FILE = nullptr;
static std::unordered_map<std::string, std::pair<int64_t, int64_t>> PACKAGE_RESOURCES;



static FILE* AndroidAssetOpen (const char* path) {
    AAsset* asset = AAssetManager_open(ANDROID_APP->activity->assetManager, path, 0);
    return asset != nullptr ? funopen(asset,
        [](void* cookie, char* buf, int size) { return AAsset_read((AAsset*)cookie, buf, size); },
        [](void* cookie, const char* buf, int size) { return 0; },
        [](void* cookie, fpos_t offset, int whence) { return AAsset_seek((AAsset*)cookie, offset, whence); },
        [](void* cookie) { AAsset_close((AAsset*)cookie); return 0; }
        ) : nullptr;
}



bool GSystem::PackageOpen (const GString& resource) {
	if(PACKAGE_FILE != nullptr)
		PackageClose();
	
	GSystem::Debug("Opening package \"%s\"... ", (const char*)resource);
	
	PACKAGE_FILE = AndroidAssetOpen(resource);
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
	
	if(fseeko(PACKAGE_FILE, headerOffset, SEEK_SET) != 0) { // The header is actually at the end of the file
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
	/*
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
	 */
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
	/*
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
	 */
	return true;
}



int64_t GSystem::ResourceSize (const GString& name) {
	if(PACKAGE_FILE == nullptr) // Open default package if none exists
		PackageOpen();
	auto i = PACKAGE_RESOURCES.find((const char*)name);
	return i != PACKAGE_RESOURCES.end() ? i->second.first : ResourceSizeFromFile(name);
}



int64_t GSystem::ResourceSizeFromFile (const GString& path) {
	FILE* file = AndroidAssetOpen(path);
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
	if(fseeko(PACKAGE_FILE, resource->second.second, SEEK_SET) != 0 || fread(&archiveSize, sizeof(archiveSize), 1, PACKAGE_FILE) != 1) {
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
	FILE* file = AndroidAssetOpen(path);
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
	/*
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
	 */
	return true;
}



static const char* GetSaveDirectory () {
	static std::string SAVE_DIRECTORY;
	//if(SAVE_DIRECTORY.empty()) {
	//	NSURL* support = [[[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:[[NSBundle mainBundle] bundleIdentifier]];
	//	if([[NSFileManager defaultManager] fileExistsAtPath:[support path]] == NO)
	//		[[NSFileManager defaultManager] createDirectoryAtPath:[support path] withIntermediateDirectories:YES attributes:nil error:nil];
	//	SAVE_DIRECTORY = std::string([support fileSystemRepresentation]);
	//	GSystem::Debug("Saves: \"%s\"\n", SAVE_DIRECTORY.c_str());
	//}
	return SAVE_DIRECTORY.c_str();
}



bool GSystem::SaveRead (const GString& name, void* data, int64_t size) {
	/*
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
	*/
	return true;
}



bool GSystem::SaveWrite (const GString& name, const void* data, int64_t size) {
	/*
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
	*/
	return true;
}





std::vector<GString> GSystem::GetFileNamesInDirectory (const GString& path) {
	return std::vector<GString>{};
}







void GSystem::RunPreferredSize (int width, int height) {
	PREFERRED_RECT.Set(0, 0, width, height);
}

void GSystem::RunPreferredFPS (int fps) {
	FPS = fps;
}

void GSystem::RunPreferredArgs (int argc, char* argv[]) {
    // Ths function does nothing on Android
}

//onNativeWindowCreated)(GameActivity *activity, ANativeWindow *window)
//onNativeWindowDestroyed)(GameActivity *activity, ANativeWindow *window)
//onNativeWindowResized)(GameActivity *activity, ANativeWindow *window, int32_t newWidth, int32_t newHeight)
//onWindowInsetsChanged)(GameActivity *activity)
//onTouchEvent)(GameActivity *activity, const GameActivityMotionEvent *event)
//onTextInputEvent)(GameActivity *activity, const GameTextInputState *state)
//onSaveInstanceState)(GameActivity *activity, SaveInstanceStateRecallback recallback, void *context)

int GSystem::Run () {
    GSystem::Debug("Running Android Application...\n");

    ANDROID_APP->onAppCmd = AndroidCommandHandler;
    android_app_set_motion_event_filter(ANDROID_APP, AndroidMotionEventFilter);

    int events;
    android_poll_source* source;
    while(!ANDROID_APP->destroyRequested) {
        if(ALooper_pollAll(0, nullptr, &events, (void **)&source) >= 0 && source)
            source->process(ANDROID_APP, source);

        //static int64_t last = 0;
        //if(GetMilliseconds() - last >= 1000 / FPS) {
        //    last = GetMilliseconds();
        AndroidGraphicsHandler();
        //}
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
