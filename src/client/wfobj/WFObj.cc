
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>      /* isspace() */
#include <string.h>     /* strlen() */
#include <stdlib.h>     /* atoi */

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "WFObj.h"

using namespace std;

#define WHITESPACE " \n\r\t\v"
//#define DEBUGGL

/****** static functions ******/

static string trimString(const string in)
{
    size_t firstpos = in.find_first_not_of(WHITESPACE);
    if (firstpos == string::npos)
        return "";
    return string(in, firstpos, in.find_last_not_of(WHITESPACE) - firstpos + 1);
}

static string stripFirstToken(string & input)
{
    size_t firstnonspace = input.find_first_not_of(WHITESPACE);
    if (firstnonspace == string::npos)
        return "";
    size_t spaceafter = input.find_first_of(WHITESPACE, firstnonspace);
    string token = input.substr(firstnonspace, spaceafter - firstnonspace);
    input.erase(0, spaceafter);
    return token;
}

vector<string> tokenize(const string & input)
{
    vector<string> tokens;
    string in = input;
    for (;;)
    {
        string token = stripFirstToken(in);
        if (token == "")
            break;
        tokens.push_back(token);
    }
    return tokens;
}

static vector<string> splitString(const string & str, char delim)
{
    vector<string> ret;
    string s = str;
    size_t pos;
    while ( (pos = s.find(delim)) != string::npos )
    {
        string t = s.substr(0, pos);
        ret.push_back(t);
        s.erase(0, pos + 1);
    }
    if (s != "")
        ret.push_back(s);
    return ret;
}

static string basePath(const string & str)
{
    size_t pos = str.find_last_of("/\\");
    return pos != string::npos ? string(str, 0, pos + 1) : "./";
}

//#define DEBUG_GL_ERROR
#ifdef DEBUG_GL_ERROR
#define checkGLError() checkGLErrorLine(__FUNCTION__, __LINE__)
static void checkGLErrorLine(const char * function, int line)
{
    GLenum err = glGetError();
    if (err != 0)
    {
        cerr << "gl error in " << function
            << ": " << err << " (0x" << hex << err << ") at line "
            << dec << line << endl;
    }
}
#else
#define checkGLError()
#endif


/****** WFObj functions ******/

WFObj::WFObj()
{
    if (m_valid)
    {
        glDeleteBuffers(1, &m_data_vbo);
        glDeleteBuffers(1, &m_index_vbo);
    }
    m_valid = false;
    clear();
}

WFObj::~WFObj()
{
}

void WFObj::clear()
{
    for (size_t i = 0; i < sizeof(m_vertices)/sizeof(m_vertices[0]); i++)
        m_vertices[i].clear();
    m_faces.clear();
    m_materials.clear();
    m_valid = false;
    m_path = "";
    m_current_material_name = "";
}

bool WFObj::load(const char *fname, loadfile_t lf, loadtexture_t lt)
{
    m_loadfile = lf;
    m_loadtexture = lt;
    if (m_loadfile == NULL)
    {
        m_loadfile = loadfile;
    }

    clear();

    Buffer buff;
    if (!m_loadfile(fname, buff))
        return false;

    m_path = fname;

    return load(buff);
}

bool WFObj::load(const WFObj::Buffer &buff)
{
    string buildup;
    size_t idx = 0;
    while (idx < buff.length)
    {
        string input = trimString(getLine(buff, idx, &idx));
        int sz = input.size();
        if (sz == 0 || input[0] == '#')
            continue;
        if (input[sz-1] == '\\')
        {
            input[sz-1] = ' ';
            buildup += input;
            continue;
        }
        if (buildup != "")
        {
            input = buildup + input;
            buildup = "";
        }
        processInputLine(input);
    }

    updateAABB();

    m_valid = buildVBO();
    return m_valid;
}

string WFObj::getLine(const Buffer & buff, size_t idx, size_t *update_idx)
{
    size_t len = 0;
    while (idx + len < buff.length)
    {
        uint8_t ch = buff.data[idx + len];
        if (ch == 0)
        {
            *update_idx = idx + len + 1;
            break;
        }
        if (ch == '\r' || ch == '\n')
        {
            *update_idx = idx + len + 1;
            uint8_t nextch = buff.data[*update_idx];
            if (ch == '\r' && nextch == '\n')
                (*update_idx)++;
            break;
        }
        len++;
    }
    return string((const char *) &buff.data[idx], len);
}

void WFObj::processInputLine(const std::string & input)
{
    vector<string> tokens = tokenize(input);

    if (tokens.size() == 0)
        return;

    string type = tokens[0];
    if (type == "v")
        m_vertices[VERTEX].push_back(readVertex(tokens));
    else if (type == "vt")
        m_vertices[VERTEX_TEXTURE].push_back(readVertex(tokens));
    else if (type == "vn")
        m_vertices[VERTEX_NORMAL].push_back(readVertex(tokens));
    else if (type == "f")
    {
        if (m_faces.find(m_current_material_name) == m_faces.end())
        {
            m_faces[m_current_material_name] = vector<Face>();
            m_num_materials = m_faces.size();
        }
        vector<Face> faces = readFaces(tokens);
        for (vector<Face>::iterator it = faces.begin(); it != faces.end(); it++)
            m_faces[m_current_material_name].push_back(*it);
    }
    else if (type == "usemtl")
    {
        if (tokens.size() >= 2)
            m_current_material_name = tokens[1];
    }
    else if (type == "mtllib")
    {
        if (tokens.size() >= 2)
            loadMaterial(tokens[1]);
    }
    else if (type == "s")
    {
        /* ignore smoothing */
    }
    else if (type == "g")
    {
        /* ignore group name */
    }
    else
    {
        cerr << "WFObj: warning: unhandled command '" << type << "'" << endl;
    }
}

void WFObj::updateAABB()
{
    bool firstVertex = true;
    for (int i = 0, sz = m_vertices[VERTEX].size(); i < sz; i++)
    {
        Vertex & v = m_vertices[VERTEX][i];
        if (firstVertex)
        {
            m_aabb[0] = m_aabb[3] = v[0];
            m_aabb[1] = m_aabb[4] = v[1];
            m_aabb[2] = m_aabb[5] = v[2];
            firstVertex = false;
        }
        else
        {
            if (v[0] < m_aabb[0])
                m_aabb[0] = v[0];
            else if (v[0] > m_aabb[3])
                m_aabb[3] = v[0];
            if (v[1] < m_aabb[1])
                m_aabb[1] = v[1];
            else if (v[1] > m_aabb[4])
                m_aabb[4] = v[1];
            if (v[2] < m_aabb[2])
                m_aabb[2] = v[2];
            else if (v[2] > m_aabb[5])
                m_aabb[5] = v[2];
        }
    }
}

WFObj::Vertex WFObj::readVertex(const vector<string> & parts)
{
    int partslen = parts.size();
    Vertex v;
    for (int i = 1; i < partslen && i <= 4; i++)
    {
        v[i - 1] = atof(parts[i].c_str());
    }
    return v;
}

vector<WFObj::Face> WFObj::readFaces(const std::vector<std::string> & parts)
{
    vector<Face> faces;
    VertexRef refs[4];
    int parts_len = parts.size();
    if (parts_len < 4 || parts_len > 5)
    {
        cerr << "WFObj: error: faces can only have 3 or 4 vertices!" << endl;
        return faces;
    }
    for (int i = 1; i < parts_len; i++)
        refs[i - 1] = readVertexRef(parts[i]);
    Face f;
    f.vertices[0] = refs[0];
    f.vertices[1] = refs[1];
    f.vertices[2] = refs[2];
    faces.push_back(f);
    if (parts_len == 5)             /* 4 vertex refs */
    {
        f.vertices[0] = refs[2];
        f.vertices[1] = refs[3];
        f.vertices[2] = refs[0];
        faces.push_back(f);
    }
    return faces;
}

WFObj::VertexRef WFObj::readVertexRef(const std::string ref)
{
    vector<string> parts = splitString(ref, '/');
    VertexRef fr;
    for (int i = 0, sz = parts.size(); i < sz; i++)
    {
        string idx_str = trimString(parts[i]);
        if (idx_str.size() > 0)
        {
            int idx = atoi(idx_str.c_str());
            switch (i)
            {
                case 0: fr.vertex = idx; break;
                case 1: fr.texture = idx; break;
                case 2: fr.normal = idx; break;
            }
        }
    }
    return fr;
}

void WFObj::loadMaterial(const std::string & name)
{
    Buffer buff;
    string path = resolvePath(name);
    if (!m_loadfile(path.c_str(), buff))
    {
        cerr << "WFObj: error: couldn't open material file '" << path << "'"
            << endl;
        return;
    }

    size_t idx = 0;
    string mat_name;
    while (idx < buff.length)
    {
        string input = trimString(getLine(buff, idx, &idx));
        if (input.size() == 0 || input[0] == '#')
            continue;
        vector<string> tokens = tokenize(input);
        if (tokens.size() < 2)
            continue;
        if (tokens[0] == "newmtl")
        {
            mat_name = tokens[1];
            m_materials[mat_name] = Material();
        }
        else if (mat_name == "")
        {
            cerr << "WFObj: error: material directive"
                " with no 'newmtl' statement" << endl;
        }
        else if (tokens[0] == "Ns")
        {
            m_materials[mat_name].shininess = atof(tokens[1].c_str());
            m_materials[mat_name].flags |= Material::SHININESS_BIT;
        }
        else if (tokens[0] == "Ka")
        {
            if (tokens.size() >= 4)
            {
                for (int i = 0; i < 3; i++)
                    m_materials[mat_name].ambient[i] =
                        atof(tokens[i + 1].c_str());
                m_materials[mat_name].ambient[3] = 1.0;
                m_materials[mat_name].flags |= Material::AMBIENT_BIT;
            }
            else
                cerr << "WFObj: error: 'Ka' material directive requires"
                    " 3 parameters" << endl;
        }
        else if (tokens[0] == "Kd")
        {
            if (tokens.size() >= 4)
            {
                for (int i = 0; i < 3; i++)
                    m_materials[mat_name].diffuse[i] =
                        atof(tokens[i + 1].c_str());
                m_materials[mat_name].diffuse[3] = 1.0;
                m_materials[mat_name].flags |= Material::DIFFUSE_BIT;
            }
            else
                cerr << "WFObj: error: 'Kd' material directive requires"
                    " 3 parameters" << endl;
        }
        else if (tokens[0] == "Ks")
        {
            if (tokens.size() >= 4)
            {
                for (int i = 0; i < 3; i++)
                    m_materials[mat_name].specular[i] =
                        atof(tokens[i + 1].c_str());
                m_materials[mat_name].specular[3] = 1.0;
                m_materials[mat_name].flags |= Material::SPECULAR_BIT;
            }
            else
                cerr << "WFObj: error: 'Ks' material directive requires"
                    " 3 parameters" << endl;
        }
        else if (tokens[0] == "Ni" || tokens[0] == "d" || tokens[0] == "illum")
        {
            /* ignore these parameters */
        }
        else if (tokens[0] == "map_Kd")
        {
            m_materials[mat_name].texture = loadTexture(resolvePath(tokens[1]));
            if (m_materials[mat_name].texture != 0)
                m_materials[mat_name].flags |= Material::TEXTURE_BIT;
        }
        else
        {
            cerr << "WFObj: warning: unrecognized material directive: '"
                << tokens[0] << "'" << endl;
        }
    }
}

bool WFObj::loadfile(const char *path, Buffer & buff)
{
    struct stat st;

    if ( (stat(path, &st) == 0) && (st.st_size > 0) )
    {
        int fd = open(path, O_RDONLY);
        if (fd > 0)
        {
            buff.alloc(st.st_size);
            int num_read = read(fd, buff.data, st.st_size);
            close(fd);
            if (num_read > 0)
                return true;
        }
    }

    return false;
}

bool WFObj::buildVBO()
{
    map<VertexRef, int> flat_vertices;
    int vid = 0, texture_ref_count = 0, n_vrefs = 0;
    m_do_textures = m_loadtexture != NULL
        && m_vertices[VERTEX_TEXTURE].size() > 0;
    for (map<string, vector<Face> >::iterator it = m_faces.begin();
            it != m_faces.end();
            it++)
    {
        if (m_materials.find(it->first) == m_materials.end())
        {
            cerr << "Material '" << it->first << "' referenced in '"
                << m_path << "' is not defined." << endl;
            return false;
        }
        for (vector<Face>::iterator fit = it->second.begin();
                fit != it->second.end();
                fit++)
        {
            for (int i = 0; i < 3; i++)
            {
                VertexRef vf = fit->vertices[i];
                if ( (vf.vertex < 1)
                        || (vf.texture < 0)
                        || (vf.normal < 1)
                        || (vf.vertex > m_vertices[VERTEX].size())
                        || (vf.normal > m_vertices[VERTEX_NORMAL].size())
                        || (vf.texture > m_vertices[VERTEX_TEXTURE].size()) )
                {
                    cerr << "WFObj: error: invalid vertex reference (<"
                        << vf.vertex << ", " << vf.texture << ", "
                        << vf.normal << ">)" << endl;
                    return false;
                }
                if (vf.texture != 0)
                    texture_ref_count++;
                if (flat_vertices.find(vf) == flat_vertices.end())
                {
                    flat_vertices[vf] = vid++;
                }
                n_vrefs++;
            }
        }
    }
    if (texture_ref_count == 0)
        m_do_textures = false;
    m_n_floats_per_vref =
        3 + /* vertex coordinates */
        3 + /* normal coordinates */
        (m_do_textures ? 2 : 0);  /* texture coordinates */
    size_t n_data_elements = m_n_floats_per_vref * flat_vertices.size();
    GLfloat * data = new GLfloat[n_data_elements];
    for (map<VertexRef, int>::iterator it = flat_vertices.begin();
            it != flat_vertices.end();
            it++)
    {
        int base = m_n_floats_per_vref * it->second;
        VertexRef vr = it->first;
        data[base + 0] = m_vertices[VERTEX][vr.vertex - 1][0];
        data[base + 1] = m_vertices[VERTEX][vr.vertex - 1][1];
        data[base + 2] = m_vertices[VERTEX][vr.vertex - 1][2];
        data[base + 3] = m_vertices[VERTEX_NORMAL][vr.normal - 1][0];
        data[base + 4] = m_vertices[VERTEX_NORMAL][vr.normal - 1][1];
        data[base + 5] = m_vertices[VERTEX_NORMAL][vr.normal - 1][2];
        if (m_do_textures && vr.texture > 0)
        {
            data[base + 6] = m_vertices[VERTEX_TEXTURE][vr.texture - 1][0];
            data[base + 7] = m_vertices[VERTEX_TEXTURE][vr.texture - 1][1];
        }
    }
    GLshort * indices = new GLshort[n_vrefs];
    vid = 0;
    for (map<string, vector<Face> >::iterator it = m_faces.begin();
            it != m_faces.end();
            it++)
    {
        int first = vid;
        for (vector<Face>::iterator fit = it->second.begin();
                fit != it->second.end();
                fit++)
        {
            for (int i = 0; i < 3; i++)
            {
                VertexRef vf = fit->vertices[i];
                indices[vid] = flat_vertices[vf];
                vid++;
            }
        }
        m_materials[it->first].first_vertex = first;
        m_materials[it->first].num_vertices = vid - first;
    }
    glGenBuffers(1, &m_data_vbo);
    glGenBuffers(1, &m_index_vbo);
    /* move data from client side to GL */
    glBindBuffer(GL_ARRAY_BUFFER, m_data_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * n_data_elements, data,
            GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLshort) * n_vrefs, indices,
            GL_STATIC_DRAW);
    delete[] data;
    delete[] indices;
    return true;
}

void WFObj::bindBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_data_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_vbo);
}

GLuint WFObj::loadTexture(const std::string & path)
{
    if (m_loadtexture == NULL)
        return 0;
    if (m_textures.find(path) != m_textures.end())
        return m_textures[path];
    GLuint id = m_loadtexture(path.c_str());
    m_textures[path] = id;
    return id;
}

string WFObj::resolvePath(const string & name)
{
    return (name[0] != '/') ? basePath(m_path) + name : name;
}

bool WFObj::VertexRef::operator<(const VertexRef & other) const
{
    if (vertex != other.vertex)
        return vertex < other.vertex;
    if (texture != other.texture)
        return texture < other.texture;
    return normal < other.normal;
}
