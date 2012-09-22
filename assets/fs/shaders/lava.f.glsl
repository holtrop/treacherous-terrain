
uniform sampler2D tex;

varying vec2 tex_coord_i;

void main(void)
{
    gl_FragColor = texture2D(tex, tex_coord_i);
}
