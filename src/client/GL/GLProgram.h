
#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include <stdarg.h>
#include "GLShader.h"
#include <map>
#include <string>
#include <vector>

class GLProgram
{
    public:
        GLProgram();
        ~GLProgram();
        bool create(const char *v_source, const char *f_source, ...);
        bool create(const uint8_t *v_source, const uint8_t *f_source, ...);
        bool createv(const char *v_source, const char *f_source, va_list va);
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
        std::vector<GLint> m_uniform_locations;
        std::map<std::string, GLint> m_uniform_location_names;
};

#endif
