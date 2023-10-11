#ifndef GPU_GLES2_GLES2_H_
#define GPU_GLES2_GLES2_H_

#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY *
#endif

/*
 * GLES2 Base functions
 */

typedef void(GL_APIENTRYP _PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void(GL_APIENTRYP _PFNGLATTACHSHADERPROC)(GLuint program,
                                                  GLuint shader);
typedef void(GL_APIENTRYP _PFNGLBINDATTRIBLOCATIONPROC)(GLuint program,
                                                        GLuint index,
                                                        const GLchar *name);
typedef void(GL_APIENTRYP _PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void(GL_APIENTRYP _PFNGLBINDFRAMEBUFFERPROC)(GLenum target,
                                                     GLuint framebuffer);
typedef void(GL_APIENTRYP _PFNGLBINDRENDERBUFFERPROC)(GLenum target,
                                                      GLuint renderbuffer);
typedef void(GL_APIENTRYP _PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void(GL_APIENTRYP _PFNGLBLENDCOLORPROC)(GLfloat red, GLfloat green,
                                                GLfloat blue, GLfloat alpha);
typedef void(GL_APIENTRYP _PFNGLBLENDEQUATIONPROC)(GLenum mode);
typedef void(GL_APIENTRYP _PFNGLBLENDEQUATIONSEPARATEPROC)(GLenum modeRGB,
                                                           GLenum modeAlpha);
typedef void(GL_APIENTRYP _PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void(GL_APIENTRYP _PFNGLBLENDFUNCSEPARATEPROC)(GLenum sfactorRGB,
                                                       GLenum dfactorRGB,
                                                       GLenum sfactorAlpha,
                                                       GLenum dfactorAlpha);
typedef void(GL_APIENTRYP _PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size,
                                                const void *data, GLenum usage);
typedef void(GL_APIENTRYP _PFNGLBUFFERSUBDATAPROC)(GLenum target,
                                                   GLintptr offset,
                                                   GLsizeiptr size,
                                                   const void *data);
typedef GLenum(GL_APIENTRYP _PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void(GL_APIENTRYP _PFNGLCLEARPROC)(GLbitfield mask);
typedef void(GL_APIENTRYP _PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green,
                                                GLfloat blue, GLfloat alpha);
typedef void(GL_APIENTRYP _PFNGLCLEARDEPTHFPROC)(GLfloat d);
typedef void(GL_APIENTRYP _PFNGLCLEARSTENCILPROC)(GLint s);
typedef void(GL_APIENTRYP _PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green,
                                               GLboolean blue, GLboolean alpha);
typedef void(GL_APIENTRYP _PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void(GL_APIENTRYP _PFNGLCOMPRESSEDTEXIMAGE2DPROC)(
    GLenum target, GLint level, GLenum internalformat, GLsizei width,
    GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void(GL_APIENTRYP _PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)(
    GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
    GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void(GL_APIENTRYP _PFNGLCOPYTEXIMAGE2DPROC)(
    GLenum target, GLint level, GLenum internalformat, GLint x, GLint y,
    GLsizei width, GLsizei height, GLint border);
typedef void(GL_APIENTRYP _PFNGLCOPYTEXSUBIMAGE2DPROC)(
    GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
    GLsizei width, GLsizei height);
typedef GLuint(GL_APIENTRYP _PFNGLCREATEPROGRAMPROC)(void);
typedef GLuint(GL_APIENTRYP _PFNGLCREATESHADERPROC)(GLenum type);
typedef void(GL_APIENTRYP _PFNGLCULLFACEPROC)(GLenum mode);
typedef void(GL_APIENTRYP _PFNGLDELETEBUFFERSPROC)(GLsizei n,
                                                   const GLuint *buffers);
typedef void(GL_APIENTRYP _PFNGLDELETEFRAMEBUFFERSPROC)(
    GLsizei n, const GLuint *framebuffers);
typedef void(GL_APIENTRYP _PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void(GL_APIENTRYP _PFNGLDELETERENDERBUFFERSPROC)(
    GLsizei n, const GLuint *renderbuffers);
typedef void(GL_APIENTRYP _PFNGLDELETESHADERPROC)(GLuint shader);
typedef void(GL_APIENTRYP _PFNGLDELETETEXTURESPROC)(GLsizei n,
                                                    const GLuint *textures);
typedef void(GL_APIENTRYP _PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void(GL_APIENTRYP _PFNGLDEPTHMASKPROC)(GLboolean flag);
typedef void(GL_APIENTRYP _PFNGLDEPTHRANGEFPROC)(GLfloat n, GLfloat f);
typedef void(GL_APIENTRYP _PFNGLDETACHSHADERPROC)(GLuint program,
                                                  GLuint shader);
typedef void(GL_APIENTRYP _PFNGLDISABLEPROC)(GLenum cap);
typedef void(GL_APIENTRYP _PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void(GL_APIENTRYP _PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first,
                                                GLsizei count);
typedef void(GL_APIENTRYP _PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count,
                                                  GLenum type,
                                                  const void *indices);
typedef void(GL_APIENTRYP _PFNGLENABLEPROC)(GLenum cap);
typedef void(GL_APIENTRYP _PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void(GL_APIENTRYP _PFNGLFINISHPROC)(void);
typedef void(GL_APIENTRYP _PFNGLFLUSHPROC)(void);
typedef void(GL_APIENTRYP _PFNGLFRAMEBUFFERRENDERBUFFERPROC)(
    GLenum target, GLenum attachment, GLenum renderbuffertarget,
    GLuint renderbuffer);
typedef void(GL_APIENTRYP _PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target,
                                                          GLenum attachment,
                                                          GLenum textarget,
                                                          GLuint texture,
                                                          GLint level);
typedef void(GL_APIENTRYP _PFNGLFRONTFACEPROC)(GLenum mode);
typedef void(GL_APIENTRYP _PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void(GL_APIENTRYP _PFNGLGENERATEMIPMAPPROC)(GLenum target);
typedef void(GL_APIENTRYP _PFNGLGENFRAMEBUFFERSPROC)(GLsizei n,
                                                     GLuint *framebuffers);
typedef void(GL_APIENTRYP _PFNGLGENRENDERBUFFERSPROC)(GLsizei n,
                                                      GLuint *renderbuffers);
typedef void(GL_APIENTRYP _PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
typedef void(GL_APIENTRYP _PFNGLGETACTIVEATTRIBPROC)(
    GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
    GLenum *type, GLchar *name);
typedef void(GL_APIENTRYP _PFNGLGETACTIVEUNIFORMPROC)(
    GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
    GLenum *type, GLchar *name);
typedef void(GL_APIENTRYP _PFNGLGETATTACHEDSHADERSPROC)(GLuint program,
                                                        GLsizei maxCount,
                                                        GLsizei *count,
                                                        GLuint *shaders);
typedef GLint(GL_APIENTRYP _PFNGLGETATTRIBLOCATIONPROC)(GLuint program,
                                                        const GLchar *name);
typedef void(GL_APIENTRYP _PFNGLGETBOOLEANVPROC)(GLenum pname, GLboolean *data);
typedef void(GL_APIENTRYP _PFNGLGETBUFFERPARAMETERIVPROC)(GLenum target,
                                                          GLenum pname,
                                                          GLint *params);
typedef GLenum(GL_APIENTRYP _PFNGLGETERRORPROC)(void);
typedef void(GL_APIENTRYP _PFNGLGETFLOATVPROC)(GLenum pname, GLfloat *data);
typedef void(GL_APIENTRYP _PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(
    GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETINTEGERVPROC)(GLenum pname, GLint *data);
typedef void(GL_APIENTRYP _PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname,
                                                  GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETPROGRAMINFOLOGPROC)(GLuint program,
                                                       GLsizei bufSize,
                                                       GLsizei *length,
                                                       GLchar *infoLog);
typedef void(GL_APIENTRYP _PFNGLGETRENDERBUFFERPARAMETERIVPROC)(GLenum target,
                                                                GLenum pname,
                                                                GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname,
                                                 GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETSHADERINFOLOGPROC)(GLuint shader,
                                                      GLsizei bufSize,
                                                      GLsizei *length,
                                                      GLchar *infoLog);
typedef void(GL_APIENTRYP _PFNGLGETSHADERPRECISIONFORMATPROC)(
    GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void(GL_APIENTRYP _PFNGLGETSHADERSOURCEPROC)(GLuint shader,
                                                     GLsizei bufSize,
                                                     GLsizei *length,
                                                     GLchar *source);
typedef const GLubyte *(GL_APIENTRYP _PFNGLGETSTRINGPROC)(GLenum name);
typedef void(GL_APIENTRYP _PFNGLGETTEXPARAMETERFVPROC)(GLenum target,
                                                       GLenum pname,
                                                       GLfloat *params);
typedef void(GL_APIENTRYP _PFNGLGETTEXPARAMETERIVPROC)(GLenum target,
                                                       GLenum pname,
                                                       GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETUNIFORMFVPROC)(GLuint program,
                                                  GLint location,
                                                  GLfloat *params);
typedef void(GL_APIENTRYP _PFNGLGETUNIFORMIVPROC)(GLuint program,
                                                  GLint location,
                                                  GLint *params);
typedef GLint(GL_APIENTRYP _PFNGLGETUNIFORMLOCATIONPROC)(GLuint program,
                                                         const GLchar *name);
typedef void(GL_APIENTRYP _PFNGLGETVERTEXATTRIBFVPROC)(GLuint index,
                                                       GLenum pname,
                                                       GLfloat *params);
typedef void(GL_APIENTRYP _PFNGLGETVERTEXATTRIBIVPROC)(GLuint index,
                                                       GLenum pname,
                                                       GLint *params);
typedef void(GL_APIENTRYP _PFNGLGETVERTEXATTRIBPOINTERVPROC)(GLuint index,
                                                             GLenum pname,
                                                             void **pointer);
typedef void(GL_APIENTRYP _PFNGLHINTPROC)(GLenum target, GLenum mode);
typedef GLboolean(GL_APIENTRYP _PFNGLISBUFFERPROC)(GLuint buffer);
typedef GLboolean(GL_APIENTRYP _PFNGLISENABLEDPROC)(GLenum cap);
typedef GLboolean(GL_APIENTRYP _PFNGLISFRAMEBUFFERPROC)(GLuint framebuffer);
typedef GLboolean(GL_APIENTRYP _PFNGLISPROGRAMPROC)(GLuint program);
typedef GLboolean(GL_APIENTRYP _PFNGLISRENDERBUFFERPROC)(GLuint renderbuffer);
typedef GLboolean(GL_APIENTRYP _PFNGLISSHADERPROC)(GLuint shader);
typedef GLboolean(GL_APIENTRYP _PFNGLISTEXTUREPROC)(GLuint texture);
typedef void(GL_APIENTRYP _PFNGLLINEWIDTHPROC)(GLfloat width);
typedef void(GL_APIENTRYP _PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void(GL_APIENTRYP _PFNGLPIXELSTOREIPROC)(GLenum pname, GLint param);
typedef void(GL_APIENTRYP _PFNGLPOLYGONOFFSETPROC)(GLfloat factor,
                                                   GLfloat units);
typedef void(GL_APIENTRYP _PFNGLREADPIXELSPROC)(GLint x, GLint y, GLsizei width,
                                                GLsizei height, GLenum format,
                                                GLenum type, void *pixels);
typedef void(GL_APIENTRYP _PFNGLRELEASESHADERCOMPILERPROC)(void);
typedef void(GL_APIENTRYP _PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target,
                                                         GLenum internalformat,
                                                         GLsizei width,
                                                         GLsizei height);
typedef void(GL_APIENTRYP _PFNGLSAMPLECOVERAGEPROC)(GLfloat value,
                                                    GLboolean invert);
typedef void(GL_APIENTRYP _PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width,
                                             GLsizei height);
typedef void(GL_APIENTRYP _PFNGLSHADERBINARYPROC)(GLsizei count,
                                                  const GLuint *shaders,
                                                  GLenum binaryFormat,
                                                  const void *binary,
                                                  GLsizei length);
typedef void(GL_APIENTRYP _PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count,
                                                  const GLchar *const *string,
                                                  const GLint *length);
typedef void(GL_APIENTRYP _PFNGLSTENCILFUNCPROC)(GLenum func, GLint ref,
                                                 GLuint mask);
typedef void(GL_APIENTRYP _PFNGLSTENCILFUNCSEPARATEPROC)(GLenum face,
                                                         GLenum func, GLint ref,
                                                         GLuint mask);
typedef void(GL_APIENTRYP _PFNGLSTENCILMASKPROC)(GLuint mask);
typedef void(GL_APIENTRYP _PFNGLSTENCILMASKSEPARATEPROC)(GLenum face,
                                                         GLuint mask);
typedef void(GL_APIENTRYP _PFNGLSTENCILOPPROC)(GLenum fail, GLenum zfail,
                                               GLenum zpass);
typedef void(GL_APIENTRYP _PFNGLSTENCILOPSEPARATEPROC)(GLenum face,
                                                       GLenum sfail,
                                                       GLenum dpfail,
                                                       GLenum dppass);
typedef void(GL_APIENTRYP _PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level,
                                                GLint internalformat,
                                                GLsizei width, GLsizei height,
                                                GLint border, GLenum format,
                                                GLenum type,
                                                const void *pixels);
typedef void(GL_APIENTRYP _PFNGLTEXPARAMETERFPROC)(GLenum target, GLenum pname,
                                                   GLfloat param);
typedef void(GL_APIENTRYP _PFNGLTEXPARAMETERFVPROC)(GLenum target, GLenum pname,
                                                    const GLfloat *params);
typedef void(GL_APIENTRYP _PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname,
                                                   GLint param);
typedef void(GL_APIENTRYP _PFNGLTEXPARAMETERIVPROC)(GLenum target, GLenum pname,
                                                    const GLint *params);
typedef void(GL_APIENTRYP _PFNGLTEXSUBIMAGE2DPROC)(
    GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
    GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void(GL_APIENTRYP _PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void(GL_APIENTRYP _PFNGLUNIFORM1FVPROC)(GLint location, GLsizei count,
                                                const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void(GL_APIENTRYP _PFNGLUNIFORM1IVPROC)(GLint location, GLsizei count,
                                                const GLint *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0,
                                               GLfloat v1);
typedef void(GL_APIENTRYP _PFNGLUNIFORM2FVPROC)(GLint location, GLsizei count,
                                                const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM2IPROC)(GLint location, GLint v0,
                                               GLint v1);
typedef void(GL_APIENTRYP _PFNGLUNIFORM2IVPROC)(GLint location, GLsizei count,
                                                const GLint *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0,
                                               GLfloat v1, GLfloat v2);
typedef void(GL_APIENTRYP _PFNGLUNIFORM3FVPROC)(GLint location, GLsizei count,
                                                const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM3IPROC)(GLint location, GLint v0,
                                               GLint v1, GLint v2);
typedef void(GL_APIENTRYP _PFNGLUNIFORM3IVPROC)(GLint location, GLsizei count,
                                                const GLint *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0,
                                               GLfloat v1, GLfloat v2,
                                               GLfloat v3);
typedef void(GL_APIENTRYP _PFNGLUNIFORM4FVPROC)(GLint location, GLsizei count,
                                                const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORM4IPROC)(GLint location, GLint v0,
                                               GLint v1, GLint v2, GLint v3);
typedef void(GL_APIENTRYP _PFNGLUNIFORM4IVPROC)(GLint location, GLsizei count,
                                                const GLint *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORMMATRIX2FVPROC)(GLint location,
                                                      GLsizei count,
                                                      GLboolean transpose,
                                                      const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORMMATRIX3FVPROC)(GLint location,
                                                      GLsizei count,
                                                      GLboolean transpose,
                                                      const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUNIFORMMATRIX4FVPROC)(GLint location,
                                                      GLsizei count,
                                                      GLboolean transpose,
                                                      const GLfloat *value);
typedef void(GL_APIENTRYP _PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void(GL_APIENTRYP _PFNGLVALIDATEPROGRAMPROC)(GLuint program);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB1FPROC)(GLuint index, GLfloat x);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB1FVPROC)(GLuint index,
                                                     const GLfloat *v);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB2FPROC)(GLuint index, GLfloat x,
                                                    GLfloat y);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB2FVPROC)(GLuint index,
                                                     const GLfloat *v);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB3FPROC)(GLuint index, GLfloat x,
                                                    GLfloat y, GLfloat z);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB3FVPROC)(GLuint index,
                                                     const GLfloat *v);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB4FPROC)(GLuint index, GLfloat x,
                                                    GLfloat y, GLfloat z,
                                                    GLfloat w);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIB4FVPROC)(GLuint index,
                                                     const GLfloat *v);
typedef void(GL_APIENTRYP _PFNGLVERTEXATTRIBPOINTERPROC)(
    GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
    const void *pointer);
typedef void(GL_APIENTRYP _PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width,
                                              GLsizei height);

/*
 * OES ARB_vertex_arrays
 */

typedef void(GL_APIENTRYP _PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void(GL_APIENTRYP _PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n,
                                                        const GLuint *arrays);
typedef void(GL_APIENTRYP _PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef GLboolean(GL_APIENTRYP _PFNGLISVERTEXARRAYPROC)(GLuint array);

#endif  // GPU_GLES2_GLES2_H_
