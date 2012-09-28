
uniform mat4 projection;
uniform mat4 modelview;
uniform float scale;

/* pos.xyz is position, pos.w is offset */
attribute vec4 pos;

void main(void)
{
    vec3 pos3 = pos.xyz * (scale + pos.w);
    gl_Position = projection * modelview * vec4(pos3, 1);
}
