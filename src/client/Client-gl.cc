
#include <math.h>
#include <iostream>
#include GL_INCLUDE_FILE
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "Client.h"
#include "ccfs.h"
#include "HexTile.h"

using namespace std;

#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))
#define OPENGL_CONTEXT_MAJOR 3
#define OPENGL_CONTEXT_MINOR 0

/* points of a horizontal hexagon 1.0 units high */
static const float overlay_hex_attributes[][3] = {
    {0.0, 0.0},
    {HEX_WIDTH_TO_HEIGHT / 2.0, 0.0, 0.0},
    {HEX_WIDTH_TO_HEIGHT / 4.0, 0.5, 0.0},
    {-HEX_WIDTH_TO_HEIGHT / 4.0, 0.5, 0.0},
    {-HEX_WIDTH_TO_HEIGHT / 2.0, 0.0, 0.0},
    {-HEX_WIDTH_TO_HEIGHT / 4.0, -0.5, 0.0},
    {HEX_WIDTH_TO_HEIGHT / 4.0, -0.5, 0.0}
};
static const GLushort overlay_hex_indices[] = {
    0, 1, 2, 3, 4, 5, 6, 1
};

static bool load_file(const char *fname, WFObj::Buffer & buff)
{
    unsigned int length;
    uint8_t *contents = (uint8_t *) CFS.get_file(fname, &length);
    if (contents != NULL)
    {
        buff.data = contents;
        buff.length = length;
        return true;
    }
    return false;
}

bool Client::create_window(bool fullscreen, int width, int height)
{
    sf::VideoMode mode = fullscreen
        ? sf::VideoMode::getDesktopMode()
        : sf::VideoMode(width, height, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    sf::ContextSettings cs = sf::ContextSettings(0, 0, 0,
            OPENGL_CONTEXT_MAJOR, OPENGL_CONTEXT_MINOR);
    m_window = new sf::Window(mode, "Treacherous Terrain", style, cs);
    m_window->setMouseCursorVisible(false);
    if (!initgl())
        return false;
    resize_window(m_window->getSize().x, m_window->getSize().y);
    return true;
}

bool Client::initgl()
{
    if (gl3wInit())
    {
        cerr << "Failed to initialize GL3W" << endl;
        return false;
    }
    if (!gl3wIsSupported(3, 0))
    {
        cerr << "OpenGL 3.0 is not supported!" << endl;
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    GLProgram::AttributeBinding obj_attrib_bindings[] = {
        {0, "pos"},
        {1, "normal"}
    };
    const char *obj_uniforms[] = {
        "ambient",
        "diffuse",
        "specular",
        "shininess",
        "projection",
        "modelview"
    };
    const char *overlay_uniforms[] = {
        "projection",
        "modelview",
        "color"
    };
    const char *obj_v_source = (const char *) CFS.get_file("shaders/obj.v.glsl", NULL);
    const char *obj_f_source = (const char *) CFS.get_file("shaders/obj.f.glsl", NULL);
    const char *overlay_f_source = (const char *) CFS.get_file("shaders/overlay.f.glsl", NULL);
    if (obj_v_source == NULL || obj_f_source == NULL)
    {
        cerr << "Error loading shader sources" << endl;
        return false;
    }
    if (!m_obj_program.create(obj_v_source, obj_f_source,
                obj_attrib_bindings, LEN(obj_attrib_bindings),
                obj_uniforms, LEN(obj_uniforms)))
    {
        cerr << "Error creating obj program" << endl;
        return false;
    }
    if (!m_overlay_program.create(obj_v_source, overlay_f_source,
                obj_attrib_bindings, LEN(obj_attrib_bindings),
                overlay_uniforms, LEN(overlay_uniforms)))
    {
        cerr << "Error creating overlay program" << endl;
        return false;
    }
    if (!m_tank_obj.load("models/tank.obj", load_file))
    {
        cerr << "Error loading tank model" << endl;
        return false;
    }
    if (!m_tile_obj.load("models/hex-tile.obj", load_file))
    {
        cerr << "Error loading hex-tile model" << endl;
        return false;
    }
    if (!m_overlay_hex_attributes.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                overlay_hex_attributes, sizeof(overlay_hex_attributes)))
    {
        cerr << "Error creating overlay hex attribute buffer" << endl;
        return false;
    }
    if (!m_overlay_hex_indices.create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
                overlay_hex_indices, sizeof(overlay_hex_indices)))
    {
        cerr << "Error creating overlay hex indices buffer" << endl;
        return false;
    }
    m_obj_program.use();
    return true;
}

void Client::resize_window(int width, int height)
{
    m_width = width;
    m_height = height;
    sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2), *m_window);
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    m_projection.load_identity();
    m_projection.perspective(60.0f, aspect, 1.0f, 5000.0f);
}

void Client::redraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double dir_x = cos(m_player->direction);
    double dir_y = sin(m_player->direction);
    m_modelview.load_identity();
    m_modelview.look_at(
            m_player->x - dir_x * 25, m_player->y - dir_y * 25, 30,
            m_player->x, m_player->y, 20,
            0, 0, 1);

    draw_players();
    draw_map();
    draw_overlay();

    m_window->display();
}

void Client::draw_players()
{
    m_obj_program.use();
    m_modelview.push();
    m_modelview.translate(m_player->x, m_player->y, 4);
    m_modelview.rotate(m_player->direction * 180.0 / M_PI, 0, 0, 1);
    m_modelview.scale(2, 2, 2);
    m_tank_obj.bindBuffers();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    int stride = m_tank_obj.getStride();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            stride, (GLvoid *) m_tank_obj.getVertexOffset());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            stride, (GLvoid *) m_tank_obj.getNormalOffset());
    m_projection.to_uniform(m_obj_program.uniform("projection"));
    m_modelview.to_uniform(m_obj_program.uniform("modelview"));
    for (map<string, WFObj::Material>::iterator it =
            m_tank_obj.getMaterials().begin();
            it != m_tank_obj.getMaterials().end();
            it++)
    {
        WFObj::Material & m = it->second;
        if (m.flags & WFObj::Material::SHININESS_BIT)
        {
            glUniform1f(m_obj_program.uniform("shininess"), m.shininess);
        }
        if (m.flags & WFObj::Material::AMBIENT_BIT)
        {
            glUniform4fv(m_obj_program.uniform("ambient"), 1, &m.ambient[0]);
        }
        if (m.flags & WFObj::Material::DIFFUSE_BIT)
        {
            glUniform4fv(m_obj_program.uniform("diffuse"), 1, &m.diffuse[0]);
        }
        if (m.flags & WFObj::Material::SPECULAR_BIT)
        {
            glUniform4fv(m_obj_program.uniform("specular"), 1, &m.specular[0]);
        }
        glDrawElements(GL_TRIANGLES, m.num_vertices,
                GL_UNSIGNED_SHORT,
                (GLvoid *) (sizeof(GLushort) * m.first_vertex));
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    m_modelview.pop();
}

void Client::draw_map()
{
    m_obj_program.use();
    m_projection.to_uniform(m_obj_program.uniform("projection"));
    m_tile_obj.bindBuffers();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    int stride = m_tile_obj.getStride();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            stride, (GLvoid *) m_tile_obj.getVertexOffset());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            stride, (GLvoid *) m_tile_obj.getNormalOffset());
    const int width = m_map.get_width();
    const int height = m_map.get_height();
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (m_map.tile_present(x, y))
            {
                refptr<HexTile> tile = m_map.get_tile(x, y);
                float cx = tile->get_x();
                float cy = tile->get_y();
                m_modelview.push();
                m_modelview.translate(cx, cy, 0);
                m_modelview.scale(tile->get_size(), tile->get_size(), tile->get_size());
                m_modelview.to_uniform(m_obj_program.uniform("modelview"));
                for (map<string, WFObj::Material>::iterator it =
                        m_tile_obj.getMaterials().begin();
                        it != m_tile_obj.getMaterials().end();
                        it++)
                {
                    WFObj::Material & m = it->second;
                    if (m.flags & WFObj::Material::SHININESS_BIT)
                    {
                        glUniform1f(m_obj_program.uniform("shininess"), m.shininess);
                    }
                    if (m.flags & WFObj::Material::AMBIENT_BIT)
                    {
                        glUniform4fv(m_obj_program.uniform("ambient"), 1, &m.ambient[0]);
                    }
                    if (m.flags & WFObj::Material::DIFFUSE_BIT)
                    {
                        glUniform4fv(m_obj_program.uniform("diffuse"), 1, &m.diffuse[0]);
                    }
                    if (m.flags & WFObj::Material::SPECULAR_BIT)
                    {
                        glUniform4fv(m_obj_program.uniform("specular"), 1, &m.specular[0]);
                    }
                    glDrawElements(GL_TRIANGLES, m.num_vertices,
                            GL_UNSIGNED_SHORT,
                            (GLvoid *) (sizeof(GLushort) * m.first_vertex));
                }
                m_modelview.pop();
            }
        }
    }
}

void Client::draw_overlay()
{
    int overlay_size = (int)(m_width * 0.15);
    glViewport(m_width - overlay_size - 50, m_height - overlay_size - 50,
            overlay_size, overlay_size);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_overlay_program.use();
    GLMatrix proj;
    const float span = 50 * 8;
    proj.ortho(-span, span, -span, span, -1, 1);
    proj.to_uniform(m_overlay_program.uniform("projection"));
    GLMatrix modelview;
    modelview.rotate(90 - m_player->direction * 180 / M_PI, 0, 0, 1);
    modelview.translate(-m_player->x, -m_player->y, 0);
    m_overlay_hex_attributes.bind();
    m_overlay_hex_indices.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(overlay_hex_attributes[0]), NULL);
    const int width = m_map.get_width();
    const int height = m_map.get_height();
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (m_map.tile_present(x, y))
            {
                refptr<HexTile> tile = m_map.get_tile(x, y);
                float cx = tile->get_x();
                float cy = tile->get_y();
                modelview.push();
                modelview.translate(cx, cy, 0);
                modelview.scale(tile->get_size(), tile->get_size(), tile->get_size());
                modelview.to_uniform(m_overlay_program.uniform("modelview"));
                glUniform4f(m_overlay_program.uniform("color"),
                        0.2, 0.8, 0.8, 0.3);
                glDrawElements(GL_TRIANGLE_FAN, LEN(overlay_hex_indices),
                        GL_UNSIGNED_SHORT, NULL);
                glUniform4f(m_overlay_program.uniform("color"),
                        0.4, 0.9, 0.9, 1.0);
                glDrawElements(GL_LINE_LOOP, LEN(overlay_hex_indices) - 2,
                        GL_UNSIGNED_SHORT, (GLvoid *)(sizeof(GLushort)));
                modelview.pop();
            }
        }
    }
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLMatrix::Identity.to_uniform(m_overlay_program.uniform("modelview"));
    glVertexAttrib3f(0, 0, 0, 0);
    glUniform4f(m_overlay_program.uniform("color"), 1, 0, 0, 1);
    glPointSize(3);
    glDrawArrays(GL_POINTS, 0, 1);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glViewport(0, 0, m_width, m_height);
}
