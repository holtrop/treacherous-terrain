
uniform float scale;
uniform mat4 projection;
uniform mat4 modelview;

attribute vec3 pos;
attribute vec3 normal;

varying vec3 pos_i;
varying vec3 normal_i;

void main(void)
{
    gl_Position = projection * modelview * vec4(scale * pos, 1);
    pos_i = gl_Position.xyz;
    normal_i = modelview * vec4(normal, 1);
}
