
#include "GLBuffer.h"

GLBuffer::GLBuffer(GLenum target, GLenum usage, const void *ptr, size_t sz)
{
    glGenBuffers(1, &m_id);
    glBindBuffer(target, m_id);
    glBufferData(target, sz, ptr, usage);
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &m_id);
}
