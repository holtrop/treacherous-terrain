
uniform sampler2D tex;
uniform float shift;

varying vec2 tex_coord_i;

void main(void)
{
    vec2 tex_coord = vec2(tex_coord_i.s + shift, tex_coord_i.t);
    gl_FragColor = texture2D(tex, tex_coord);
}
