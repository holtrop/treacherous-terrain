
#include "GLShader.h"
#include <iostream>

using namespace std;

GLShader::GLShader()
{
    m_id = 0;
}

GLShader::~GLShader()
{
    if (m_id > 0)
    {
        glDeleteShader(m_id);
    }
}

bool GLShader::create(GLenum shaderType, const char *source)
{
    GLint status;

    m_id = glCreateShader(shaderType);
    if (m_id > 0)
    {
        glShaderSource(m_id, 1, &source, NULL);

        glCompileShader(m_id);

        glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
        if (status == GL_TRUE)
        {
            return true;
        }

        GLint log_length;
        cerr << "Error compiling shader" << endl;
        glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0)
        {
            char * log = new char[log_length];
            glGetShaderInfoLog(m_id, log_length, &log_length, log);
            cerr << "Shader Log:" << endl << log << endl;
            delete[] log;
        }
        glDeleteShader(m_id);
    }
    return false;
}
