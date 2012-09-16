
uniform mat4 projection;
uniform mat4 modelview;

attribute vec3 pos;
attribute vec3 normal;
attribute vec2 tex_coord;

varying vec3 pos_i;
varying vec3 normal_i;
varying vec2 tex_coord_i;

void main(void)
{
    gl_Position = projection * modelview * vec4(pos, 1);
    pos_i = gl_Position.xyz;
    tex_coord_i = tex_coord;
    normal_i = modelview * vec4(normal, 0);
}
