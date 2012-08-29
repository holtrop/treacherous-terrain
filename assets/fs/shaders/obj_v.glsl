
attribute vec3 pos;
attribute vec3 normal;

varying vec3 pos_i;
varying vec3 normal_i;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1);
    pos_i = gl_Position.xyz;
    normal_i = gl_NormalMatrix * normal;
}
