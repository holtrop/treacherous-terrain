
#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include "GLShader.h"

class GLProgram
{
    public:
        typedef struct
        {
            GLuint index;
            const char *name;
        } AttributeBinding;
        GLProgram();
        ~GLProgram();
        bool create(const char *v_source, const char *f_source,
                AttributeBinding *bindings = NULL);
        GLuint get_id() { return m_id; }
        GLint get_uniform_location(const char *name);
        void get_uniform_locations(const char **names, int num, GLint *locs);
        void use() { glUseProgram(m_id); }
    protected:
        GLuint m_id;
        GLShader m_v_shader;
        GLShader m_f_shader;
};

#endif
