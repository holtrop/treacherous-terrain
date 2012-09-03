
#ifndef GLBUFFER_H
#define GLBUFFER_H

#include <GL/glew.h>

class GLBuffer
{
    public:
        GLBuffer(GLenum target, GLenum usage, const void *ptr, size_t sz);
        ~GLBuffer();
        GLuint get_id() { return m_id; }
    protected:
        GLuint m_id;
};

#endif
