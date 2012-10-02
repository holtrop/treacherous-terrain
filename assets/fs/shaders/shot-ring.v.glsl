
uniform mat4 projection;
uniform mat4 modelview;
uniform float scale;
uniform float width;

/* pos.xyz is position, pos.w is 0/1 for inner/outer ring */
attribute vec4 pos;

void main(void)
{
    vec3 pos3 = pos.xyz * (scale + width * pos.w);
    gl_Position = projection * modelview * vec4(pos3, 1);
}
