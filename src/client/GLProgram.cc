
#include "GLProgram.h"
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

bool GLProgram::create(const char *v_source, const char *f_source,
                GLProgram::AttributeBinding *bindings)
{
    if (!m_v_shader.create(GL_VERTEX_SHADER, v_source))
        return false;
    if (!m_f_shader.create(GL_FRAGMENT_SHADER, f_source))
        return false;
    m_id = glCreateProgram();
    if (m_id <= 0)
        return false;
    glAttachShader(m_id, m_v_shader.get_id());
    glAttachShader(m_id, m_f_shader.get_id());

    if (bindings != NULL)
    {
        for (int i = 0; bindings[i].name != NULL; i++)
        {
            glBindAttribLocation(m_id, bindings[i].index, bindings[i].name);
        }
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

    return true;
}

GLint GLProgram::get_uniform_location(const char *name)
{
    return glGetUniformLocation(m_id, name);
}

void GLProgram::get_uniform_locations(const char **names, int num, GLint *locs)
{
    for (int i = 0; i < num; i++)
    {
        locs[i] = get_uniform_location(names[i]);
    }
}
