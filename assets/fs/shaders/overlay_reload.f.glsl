
varying vec3 pos_i;

void main(void)
{
    gl_FragColor = mix(vec4(.7, .7, 1, 1), vec4(0, 0, 1, 1), (pos_i.x + 1) / 2);
}
