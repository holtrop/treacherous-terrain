
#ifndef GLSHADER_H
#define GLSHADER_H

#include GL_INCLUDE_FILE

class GLShader
{
    public:
        GLShader();
        ~GLShader();
        bool create(GLenum shaderType, const char *source);
        GLuint get_id() { return m_id; }
        bool valid() { return m_id > 0; }
    protected:
        GLuint m_id;
};

#endif
