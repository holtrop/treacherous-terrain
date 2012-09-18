
uniform mat4 projection;
uniform mat4 modelview;

attribute vec3 pos;
attribute vec3 color;

varying vec3 color_i;

void main(void)
{
    gl_Position = projection * modelview * vec4(pos, 1);
    color_i = color;
}
