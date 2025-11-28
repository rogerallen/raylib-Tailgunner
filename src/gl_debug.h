#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#ifdef DEBUG_OPENGL

#include "raylib.h"
#include <GL/gl.h>
#include <stdio.h>

static void _gl_check_error(const char *file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char *error_str;
        switch (err) {
        case GL_INVALID_ENUM:
            error_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_str = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error_str = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error_str = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error_str = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error_str = "UNKNOWN_ERROR";
            break;
        }
        printf("OpenGL Error: %s at %s:%d\n", error_str, file, line);
    }
}

#define CHECK_GL_ERRORS() _gl_check_error(__FILE__, __LINE__)

#define FPS_INTERVAL 1.0

#else

#define CHECK_GL_ERRORS()

#define FPS_INTERVAL 10.0

#endif // DEBUG_OPENGL

#endif // GL_DEBUG_H
