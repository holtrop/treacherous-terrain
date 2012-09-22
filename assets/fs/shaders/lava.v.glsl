
uniform mat4 projection;
uniform mat4 modelview;

attribute vec3 pos;
attribute vec2 tex_coord;

varying vec2 tex_coord_i;

void main(void)
{
    gl_Position = projection * modelview * vec4(pos, 1);
    tex_coord_i = tex_coord;
}
