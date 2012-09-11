
#ifndef GLBUFFER_H
#define GLBUFFER_H

#include <GL/glew.h>

class GLBuffer
{
    public:
        GLBuffer();
        ~GLBuffer();
        bool create(GLenum target, GLenum usage, const void *ptr, size_t sz);
        GLuint get_id() { return m_id; }
        void bind() { glBindBuffer(m_target, m_id); }
    protected:
        GLuint m_id;
        GLenum m_target;
};

#endif
