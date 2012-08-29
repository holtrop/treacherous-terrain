
uniform vec4 ambient, specular;
uniform float shininess;
uniform sampler2D tex;

varying vec3 pos_i;
varying vec3 normal_i;
varying vec2 tex_coord_i;

void main(void)
{
    vec3 n, lightDir;
    vec4 color;
    float NdotL, RdotEye;

    lightDir = normalize(vec3(-0.1, 0, -0.9));
    color = vec4(0.2, 0.2, 0.2, 1.0) * ambient;  /* ambient light */
    n = normalize(normal_i);

    NdotL = max(dot(n, -lightDir), 0.0);

    if (NdotL > 0.0)
    {
        /* diffuse component */
        color += texture2D(tex, tex_coord_i) * NdotL;
        /* specular component */
        RdotEye = dot(normalize(-pos_i), normalize(reflect(-lightDir, n)));
        if (RdotEye > 0.0)
        {
            color += clamp(specular * pow(RdotEye, shininess), 0.0, 1.0);
        }
    }

    gl_FragColor = color;
}
