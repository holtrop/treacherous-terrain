
#include "GLBuffer.h"

GLBuffer::GLBuffer()
{
    m_id = 0;
}

GLBuffer::~GLBuffer()
{
    if (m_id > 0)
    {
        glDeleteBuffers(1, &m_id);
    }
}

bool GLBuffer::create(GLenum target, GLenum usage, const void *ptr, size_t sz)
{
    glGenBuffers(1, &m_id);
    if (m_id > 0)
    {
        glBindBuffer(target, m_id);
        glBufferData(target, sz, ptr, usage);
        return true;
    }
    return false;
}
