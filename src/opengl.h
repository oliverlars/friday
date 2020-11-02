#include <windows.h>
#include <gl/gl.h>
#include "ext/wglext.h"
#include "ext/glext.h"
typedef void APIENTRY type_glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void APIENTRY type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void APIENTRY type_glGenFramebuffers(GLsizei n, GLuint *framebuffers);
typedef void APIENTRY type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum APIENTRY type_glCheckFramebufferStatus(GLenum target);
typedef void APIENTRY type_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void APIENTRY type_glAttachShader(GLuint program, GLuint shader);
typedef void APIENTRY type_glCompileShader(GLuint shader);
typedef GLuint APIENTRY type_glCreateProgram(void);
typedef GLuint APIENTRY type_glCreateShader(GLenum type);
typedef void APIENTRY type_glLinkProgram(GLuint program);
typedef void APIENTRY type_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void APIENTRY type_glUseProgram(GLuint program);
typedef void APIENTRY type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void APIENTRY type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void APIENTRY type_glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
typedef void APIENTRY type_glValidateProgram(GLuint program);
typedef void APIENTRY type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef GLint APIENTRY type_glGetUniformLocation (GLuint program, const GLchar *name);
typedef void APIENTRY type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void APIENTRY type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void APIENTRY type_glUniform1i(GLint location, GLint v0);
typedef void APIENTRY type_glGenerateMipmap(GLenum texture);
typedef void APIENTRY type_glUniform1f(GLint location, GLfloat v0);
typedef void APIENTRY type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void APIENTRY type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);

typedef void APIENTRY type_glEnableVertexAttribArray(GLuint index);
typedef void APIENTRY type_glDisableVertexAttribArray(GLuint index);
typedef GLint APIENTRY type_glGetAttribLocation(GLuint program, const GLchar *name);
typedef void APIENTRY type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void APIENTRY type_glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void APIENTRY type_glBindVertexArray(GLuint array);
typedef void APIENTRY type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void APIENTRY type_glBindBuffer (GLenum target, GLuint buffer);
typedef void APIENTRY type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void APIENTRY type_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void APIENTRY type_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void APIENTRY type_glDeleteProgram (GLuint program);
typedef void APIENTRY type_glDeleteShader (GLuint shader);
typedef void APIENTRY type_glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
typedef void APIENTRY type_glDrawBuffers (GLsizei n, const GLenum *bufs);
typedef void APIENTRY type_glDrawBuffers (GLsizei n, const GLenum *bufs);
typedef void APIENTRY type_glActiveTexture(GLenum texture);

#define make_gl_func(name) type_##name* name

make_gl_func(glTexImage2DMultisample);
make_gl_func(glBindFramebuffer);
make_gl_func(glGenFramebuffers);
make_gl_func(glFramebufferTexture2D);
make_gl_func(glCheckFramebufferStatus);
make_gl_func(glBlitFramebuffer);
make_gl_func(glAttachShader);
make_gl_func(glCompileShader);
make_gl_func(glCreateProgram);
make_gl_func(glCreateShader);
make_gl_func(glLinkProgram);
make_gl_func(glShaderSource);
make_gl_func(glUseProgram);
make_gl_func(glGetProgramInfoLog);
make_gl_func(glGetShaderInfoLog);
make_gl_func(glGetShaderiv);
make_gl_func(glValidateProgram);
make_gl_func(glGetProgramiv);
make_gl_func(glGetUniformLocation);
make_gl_func(glUniform4fv);
make_gl_func(glUniformMatrix4fv);
make_gl_func(glUniform1i);
make_gl_func(glUniform1f);
make_gl_func(glUniform2fv);
make_gl_func(glUniform3fv);
make_gl_func(glGenerateMipmap);
make_gl_func(glEnableVertexAttribArray);
make_gl_func(glDisableVertexAttribArray);
make_gl_func(glGetAttribLocation);
make_gl_func(glVertexAttribPointer);
make_gl_func(glVertexAttribIPointer);
make_gl_func(glBindVertexArray);
make_gl_func(glGenVertexArrays);
make_gl_func(glBindBuffer);
make_gl_func(glGenBuffers);
make_gl_func(glBufferData);
make_gl_func(glBufferSubData);
make_gl_func(glDeleteProgram);
make_gl_func(glDeleteShader);
make_gl_func(glDeleteFramebuffers);
make_gl_func(glDrawBuffers);
make_gl_func(glActiveTexture);

internal void
load_all_opengl_procs(void) {
#define get_gl_func(name) name = (type_##name *)platform->load_opengl_procedure(#name);
    get_gl_func(glTexImage2DMultisample);
    get_gl_func(glBlitFramebuffer);
    get_gl_func(glAttachShader);
    get_gl_func(glCompileShader);
    get_gl_func(glCreateProgram);
    get_gl_func(glCreateShader);
    get_gl_func(glLinkProgram);
    get_gl_func(glShaderSource);
    get_gl_func(glUseProgram);
    get_gl_func(glGetProgramInfoLog);
    get_gl_func(glGetShaderInfoLog);
    get_gl_func(glGetShaderiv);
    get_gl_func(glValidateProgram);
    get_gl_func(glGetProgramiv);
    get_gl_func(glGetUniformLocation);
    get_gl_func(glUniform4fv);
    get_gl_func(glUniformMatrix4fv);
    get_gl_func(glUniform1i);
    get_gl_func(glGenerateMipmap);
    get_gl_func(glEnableVertexAttribArray);
    get_gl_func(glDisableVertexAttribArray);
    get_gl_func(glGetAttribLocation);
    get_gl_func(glVertexAttribPointer);
    get_gl_func(glVertexAttribIPointer);
    get_gl_func(glBindVertexArray);
    get_gl_func(glGenVertexArrays);
    get_gl_func(glBindBuffer);
    get_gl_func(glGenBuffers);
    get_gl_func(glBufferData);
    get_gl_func(glBufferSubData);
    get_gl_func(glDeleteProgram);
    get_gl_func(glDeleteShader);
    get_gl_func(glDeleteFramebuffers);
    get_gl_func(glDrawBuffers);
    get_gl_func(glUniform1f);
    get_gl_func(glUniform2fv);
    get_gl_func(glUniform3fv);
    get_gl_func(glActiveTexture);
    
}