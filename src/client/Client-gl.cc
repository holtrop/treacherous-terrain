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

#define SKY_DIST 2000
#define NUM_SKY_STEPS 9
#define LAVA_SIZE 100
#define SHOT_RING_WIDTH 10.0f
#define NUM_SHOT_RING_STEPS 24

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
static const struct
{
    float pos[3];
    float tex_coord[2];
} tex_quad_attributes[] = {
    {{0.5, 0.5, 0.0}, {1.0, 1.0}},
    {{-0.5, 0.5, 0.0}, {0.0, 1.0}},
    {{-0.5, -0.5, 0.0}, {0.0, 0.0}},
    {{0.5, -0.5, 0.0}, {1.0, 0.0}}
};
static const float quad_attributes[][3] = {
    {0.5, 0.5, 0.0},
    {-0.5, 0.5, 0.0},
    {-0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0}
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
    if (!m_obj_program.create(
                CFS.get_file("shaders/obj.v.glsl"),
                CFS.get_file("shaders/obj.f.glsl"),
                "pos", 0, "normal", 1, NULL,
                "ambient", "diffuse", "specular", "shininess",
                "projection", "modelview", NULL))
        return false;
    if (!m_overlay_program.create(
                CFS.get_file("shaders/obj.v.glsl"),
                CFS.get_file("shaders/overlay.f.glsl"),
                "pos", 0, "normal", 1, NULL,
                "projection", "modelview", "color", NULL))
        return false;
    if (!m_overlay_hover_program.create(
                CFS.get_file("shaders/obj.v.glsl"),
                CFS.get_file("shaders/overlay_hover.f.glsl"),
                "pos", 0, "normal", 1, NULL,
                "projection", "modelview", NULL))
        return false;
    if (!m_sky_program.create(
                CFS.get_file("shaders/sky.v.glsl"),
                CFS.get_file("shaders/sky.f.glsl"),
                "pos", 0, "color", 1, NULL,
                "projection", "modelview", NULL))
        return false;
    if (!m_lava_program.create(
                CFS.get_file("shaders/lava.v.glsl"),
                CFS.get_file("shaders/lava.f.glsl"),
                "pos", 0, "tex_coord", 1, NULL,
                "projection", "modelview", "tex", "shift", NULL))
        return false;
    if (!m_shot_ring_program.create(
                CFS.get_file("shaders/shot-ring.v.glsl"),
                CFS.get_file("shaders/shot-ring.f.glsl"),
                "pos", 0, NULL,
                "projection", "modelview", "scale", NULL))
        return false;
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
    if (!m_tex_quad_attributes.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                tex_quad_attributes, sizeof(tex_quad_attributes)))
    {
        cerr << "Error creating tex quad attributes buffer" << endl;
        return false;
    }
    if (!m_quad_attributes.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                quad_attributes, sizeof(quad_attributes)))
    {
        cerr << "Error creating quad attributes buffer" << endl;
        return false;
    }
    vector<GLfloat> sky_attributes((NUM_SKY_STEPS + 1) * 2 * (3 + 3));
    for (int i = 0, idx = 0; i <= NUM_SKY_STEPS; i++)
    {
        GLfloat x = SKY_DIST * sin(M_PI_4 + i * M_PI_2 / NUM_SKY_STEPS);
        GLfloat y = SKY_DIST * cos(M_PI - M_PI_4 - i * M_PI_2 / NUM_SKY_STEPS);
        sky_attributes[idx++] = x;
        sky_attributes[idx++] = y;
        sky_attributes[idx++] = -10.0;
        sky_attributes[idx++] = 0.6;
        sky_attributes[idx++] = 0.6;
        sky_attributes[idx++] = 1.0;

        sky_attributes[idx++] = x;
        sky_attributes[idx++] = y;
        sky_attributes[idx++] = 1000.0;
        sky_attributes[idx++] = 0.1;
        sky_attributes[idx++] = 0.1;
        sky_attributes[idx++] = 1.0;
    }
    if (!m_sky_attributes.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                &sky_attributes[0],
                sizeof(sky_attributes[0]) * sky_attributes.size()))
    {
        cerr << "Error creating sky attribute buffer" << endl;
        return false;
    }
    vector<GLfloat> shot_ring_attributes((NUM_SHOT_RING_STEPS + 1) * 2 * 4);
    for (int i = 0, idx = 0; i <= NUM_SHOT_RING_STEPS; i++)
    {
        double angle = i * M_PI * 2.0 / NUM_SHOT_RING_STEPS;
        GLfloat x = sin(angle);
        GLfloat y = cos(angle);
        shot_ring_attributes[idx++] = x;
        shot_ring_attributes[idx++] = y;
        shot_ring_attributes[idx++] = 0.0f;
        shot_ring_attributes[idx++] = 0.0f;

        shot_ring_attributes[idx++] = x;
        shot_ring_attributes[idx++] = y;
        shot_ring_attributes[idx++] = 0.0f;
        shot_ring_attributes[idx++] = SHOT_RING_WIDTH;
    }
    if (!m_shot_ring_attributes.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                &shot_ring_attributes[0],
                sizeof(shot_ring_attributes[0]) * shot_ring_attributes.size()))
    {
        cerr << "Error creating shot ring attributes buffer" << endl;
        return false;
    }
    unsigned int lava_texture_length;
    const uint8_t *lava_texture = CFS.get_file("textures/lava.jpg",
            &lava_texture_length);
    if (lava_texture == NULL)
    {
        cerr << "Error loading lava texture" << endl;
        return false;
    }
    if (!m_lava_texture.loadFromMemory(lava_texture, lava_texture_length))
    {
        cerr << "Error creating lava texture" << endl;
        return false;
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

    if (m_players.size() > 0)
    {
        double dir_x = cos(m_players[current_player]->direction);
        double dir_y = sin(m_players[current_player]->direction);
        m_modelview.load_identity();
        m_modelview.look_at(
                m_players[current_player]->x - dir_x * 25, m_players[current_player]->y - dir_y * 25, 30,
                m_players[current_player]->x, m_players[current_player]->y, 20,
                0, 0, 1);

        for(std::map<sf::Uint8, refptr<Player> >::iterator piter = m_players.begin(); piter != m_players.end(); piter++)
        {
            draw_player(piter->second);
        }

        draw_map();
        draw_sky();
        draw_lava();
        draw_shot_ring();

        draw_overlay();
    }

    m_window->display();
}

void Client::draw_player(refptr<Player> player)
{
    m_obj_program.use();
    m_modelview.push();
    m_modelview.translate(player->x, player->y, 4);
    m_modelview.rotate(player->direction * 180.0 / M_PI, 0, 0, 1);
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
    /* draw overlay map */
    int overlay_size = (int)(m_width * 0.15);
    glViewport(m_width - overlay_size - 50, m_height - overlay_size - 50,
            overlay_size, overlay_size);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    m_overlay_program.use();
    GLMatrix proj;
    const float span = 50 * 8;
    proj.ortho(-span, span, -span, span, -1, 1);
    proj.to_uniform(m_overlay_program.uniform("projection"));
    GLMatrix modelview;
    modelview.rotate(90 - m_players[current_player]->direction * 180 / M_PI, 0, 0, 1);
    modelview.translate(-m_players[current_player]->x, -m_players[current_player]->y, 0);
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

    /* draw hover bar */
    glViewport(m_width - 200, 100, 150, 25);
    m_overlay_hover_program.use();
    m_quad_attributes.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
    GLMatrix::Identity.to_uniform(
            m_overlay_hover_program.uniform("projection"));
    modelview.load_identity();
    modelview.translate(m_players[current_player]->hover - 1, 0, 0);
    modelview.scale(m_players[current_player]->hover * 2, 2.0, 1.0);
    modelview.to_uniform(m_overlay_hover_program.uniform("modelview"));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    /* draw map border */
    glViewport(0, 0, m_width, m_height);
    m_overlay_program.use();
    m_quad_attributes.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
    GLMatrix::Identity.to_uniform(m_overlay_program.uniform("projection"));
    modelview.load_identity();
    modelview.ortho(0, m_width, 0, m_height, -1, 1);
    modelview.translate(m_width - overlay_size / 2 - 50,
            m_height - overlay_size / 2 - 50, 0);
    modelview.scale(overlay_size + 0.1, overlay_size + 0.1, 1);
    modelview.to_uniform(m_overlay_program.uniform("modelview"));
    glUniform4f(m_overlay_program.uniform("color"), 1, 1, 1, 1);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    /* draw hover bar border */
    modelview.load_identity();
    modelview.ortho(0, m_width, 0, m_height, -1, 1);
    modelview.translate(m_width - 200 + 150 / 2, 100 + 25 / 2, 0);
    modelview.scale(150.1, 25.1, 1);
    modelview.to_uniform(m_overlay_program.uniform("modelview"));
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    /* reset GL to normal state */
    glDisableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Client::draw_sky()
{
    m_sky_program.use();
    m_sky_attributes.bind();
    m_projection.to_uniform(m_sky_program.uniform("projection"));
    m_modelview.push();
    m_modelview.translate(m_players[current_player]->x, m_players[current_player]->y, 0);
    m_modelview.rotate(m_players[current_player]->direction * 180.0 / M_PI, 0, 0, 1);
    m_modelview.to_uniform(m_sky_program.uniform("modelview"));
    m_modelview.pop();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (NUM_SKY_STEPS + 1) * 2);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Client::draw_lava()
{
    m_lava_program.use();
    m_tex_quad_attributes.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    m_projection.to_uniform(m_lava_program.uniform("projection"));
    m_lava_texture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1i(m_lava_program.uniform("tex"), 0);
    double shift = m_clock.getElapsedTime().asSeconds() / 40;
    glUniform1f(m_lava_program.uniform("shift"), shift);
    const int n_lavas = 2 * SKY_DIST / LAVA_SIZE;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    for (int i = 0; i < n_lavas; i++)
    {
        for (int j = 0; j < n_lavas; j++)
        {
            m_modelview.push();
            m_modelview.translate((i - n_lavas / 2) * LAVA_SIZE,
                    (j - n_lavas / 2) * LAVA_SIZE, -2);
            m_modelview.scale(LAVA_SIZE, LAVA_SIZE, 1);
            m_modelview.to_uniform(m_lava_program.uniform("modelview"));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            m_modelview.pop();
        }
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Client::draw_shot_ring()
{
    if (m_drawing_shot)
    {
        m_shot_ring_program.use();
        m_shot_ring_attributes.bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), NULL);
        glEnable(GL_BLEND);
        m_modelview.push();
        m_modelview.translate(m_players[current_player]->x,
                m_players[current_player]->y, 0.4);
        m_projection.to_uniform(m_shot_ring_program.uniform("projection"));
        m_modelview.to_uniform(m_shot_ring_program.uniform("modelview"));
        glUniform1f(m_shot_ring_program.uniform("scale"), m_drawing_shot_distance);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, (NUM_SHOT_RING_STEPS + 1) * 2);
        m_modelview.pop();
        glDisableVertexAttribArray(0);
        glDisable(GL_BLEND);
    }
}
