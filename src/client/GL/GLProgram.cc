
#include "GLProgram.h"
#include <stdint.h>
#include <iostream>

using namespace std;

GLProgram::GLProgram()
{
    m_id = 0;
}

GLProgram::~GLProgram()
{
    if (m_id > 0)
    {
        glDeleteProgram(m_id);
    }
}

bool GLProgram::create(const char *v_source, const char *f_source, ...)
{
    va_list va;
    va_start(va, f_source);
    bool rv = createv(v_source, f_source, va);
    va_end(va);
    return rv;
}

bool GLProgram::create(const uint8_t *v_source, const uint8_t *f_source, ...)
{
    va_list va;
    va_start(va, f_source);
    bool rv = createv((const char *) v_source, (const char *) f_source, va);
    va_end(va);
    return rv;
}

bool GLProgram::createv(const char *v_source, const char *f_source, va_list va)
{
    if (v_source == NULL || f_source == NULL)
        return false;
    if (!m_v_shader.create(GL_VERTEX_SHADER, v_source))
    {
        cerr << "Failed vertex shader source:" << endl << v_source << endl;
        return false;
    }
    if (!m_f_shader.create(GL_FRAGMENT_SHADER, f_source))
    {
        cerr << "Failed fragment shader source:" << endl << f_source << endl;
        return false;
    }
    m_id = glCreateProgram();
    if (m_id <= 0)
    {
        cerr << "Error allocating GL program object" << endl;
        return false;
    }
    glAttachShader(m_id, m_v_shader.get_id());
    glAttachShader(m_id, m_f_shader.get_id());

    for (;;)
    {
        const char *attribute_name = va_arg(va, const char *);
        if (attribute_name == NULL)
            break;
        GLuint attribute_index = va_arg(va, uint32_t);
        glBindAttribLocation(m_id, attribute_index, attribute_name);
    }

    glLinkProgram(m_id);

    GLint link_status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0)
        {
            char *log = new char[log_length];
            glGetProgramInfoLog(m_id, log_length, &log_length, log);
            cerr << "Program log:" << endl << log << endl;
            delete[] log;
        }
        return false;
    }

    for (;;)
    {
        const char *uniform_name = va_arg(va, const char *);
        if (uniform_name == NULL)
            break;
        GLint loc = glGetUniformLocation(m_id, uniform_name);
        m_uniform_locations.push_back(loc);
        m_uniform_location_names[uniform_name] = loc;
    }

    return true;
}
