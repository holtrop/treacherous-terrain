
#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include "GLShader.h"
#include <map>
#include <string>

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
                AttributeBinding *bindings = NULL, int n_bindings = 0,
                const char **uniforms = NULL, int n_uniforms = 0);
        GLuint get_id() { return m_id; }
        GLint get_uniform_location(const char *name);
        void get_uniform_locations(const char **names, int num, GLint *locs);
        void use() { glUseProgram(m_id); }
        GLint uniform(int i) { return m_uniform_locations[i]; }
        GLint uniform(const std::string & s) { return m_uniform_location_names[s]; }
    protected:
        GLuint m_id;
        GLShader m_v_shader;
        GLShader m_f_shader;
        GLint * m_uniform_locations;
        std::map<std::string, GLint> m_uniform_location_names;
};

#endif
