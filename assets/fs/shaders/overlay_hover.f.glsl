
varying vec3 pos_i;

const vec4 red = vec4(1, 0, 0, 1);
const vec4 yellow = vec4(1, 1, 0, 1);
const vec4 green = vec4(0, 1, 0, 1);

void main(void)
{
    if (pos_i.x < 0)
    {
        gl_FragColor = mix(red, yellow, pos_i.x + 1);
    }
    else
    {
        gl_FragColor = mix(yellow, green, pos_i.x);
    }
}
