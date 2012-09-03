
#ifndef WFOBJ_H
#define WFOBJ_H

#ifdef GL_INCLUDE_FILE
#include GL_INCLUDE_FILE
#else
#include <GL/gl.h>
#endif

#include <vector>
#include <string>
#include <map>

class WFObj
{
public:
    /* types */
    class Buffer
    {
        public:
            Buffer() : m_alloc(false) {data = NULL; length = 0;}
            ~Buffer() { if (m_alloc) delete data; }
            uint8_t *data;
            size_t length;
            void alloc(size_t sz)
            {
                length = sz;
                data = new uint8_t[sz];
                m_alloc = true;
            }
        protected:
            bool m_alloc;
    };

    class Material
    {
        public:
            enum {
                SHININESS_BIT   = 0x01,
                AMBIENT_BIT     = 0x02,
                DIFFUSE_BIT     = 0x04,
                SPECULAR_BIT    = 0x08,
                TEXTURE_BIT     = 0x10
            };
            Material() : flags(0) {}
            GLfloat shininess;
            GLfloat ambient[4];
            GLfloat diffuse[4];
            GLfloat specular[4];
            GLuint texture;
            int flags;
            int first_vertex;
            int num_vertices;
    };

    typedef bool (*loadfile_t)(const char *fname, Buffer & buff);
    typedef GLuint (*loadtexture_t)(const char *fname);

    enum { VERTEX, VERTEX_TEXTURE, VERTEX_NORMAL, VERTEX_TYPES };

    /* constructors */
    WFObj();
    ~WFObj();

    /* methods */
    bool load(const char *fname, loadfile_t lf = NULL, loadtexture_t lt = NULL);
    bool load(const Buffer &buff);
    const float * const getAABB() { return m_aabb; }
    size_t getStride() { return sizeof(GLfloat) * m_n_floats_per_vref; }
    size_t getVertexOffset() { return 0; }
    size_t getNormalOffset() { return 3 * sizeof(GLfloat); }
    size_t getTextureCoordOffset() { return 6 * sizeof(GLfloat); }
    void bindBuffers();
    size_t getNumMaterials() { return m_num_materials; }
    std::map<std::string, Material> & getMaterials() { return m_materials; }
    bool doTextures() { return m_do_textures; }

protected:
    /* types */
    class Vertex
    {
        public:
            float operator[](int idx) const { return data[idx]; }
            float & operator[](int idx) { return data[idx]; }
            float * getData() { return data; }
        private:
            float data[4];
    };

    class VertexRef
    {
        public:
            VertexRef() : vertex(0), texture(0), normal(0) {}
            size_t vertex;
            size_t texture;
            size_t normal;
            bool operator<(const VertexRef & other) const;
    };

    class Face
    {
        public:
            VertexRef vertices[3];
    };

    /* methods */
    void clear();
    void processInputLine(const std::string & input);
    Vertex readVertex(const std::vector<std::string> & parts);
    std::vector<Face> readFaces(const std::vector<std::string> & parts);
    VertexRef readVertexRef(const std::string ref);
    void updateAABB();
    static bool loadfile(const char *path, Buffer & buff);
    std::string getLine(const Buffer & buff, size_t idx, size_t *update_idx);
    void loadMaterial(const std::string & name);
    bool buildVBO();
    GLuint loadTexture(const std::string & path);
    std::string resolvePath(const std::string & name);

    /* variables */
    std::vector<Vertex> m_vertices[VERTEX_TYPES];
    std::map< std::string, std::vector< Face > > m_faces;
    std::map< std::string, Material > m_materials;
    float m_aabb[6];
    std::string m_path;
    loadfile_t m_loadfile;
    loadtexture_t m_loadtexture;
    std::string m_current_material_name;
    bool m_valid;
    GLuint m_data_vbo, m_index_vbo;
    bool m_do_textures;
    size_t m_n_floats_per_vref;
    size_t m_num_materials;
    std::map<std::string, GLuint> m_textures;
};

#endif
