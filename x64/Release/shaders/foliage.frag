uniform sampler2D texture;
uniform float time;

void main()
{
    // lookup the pixel in the texture
    vec2 coord = gl_TexCoord[0].xy + vec2(cos(gl_TexCoord[0].y*30.0+time*4.2831)/60.0,0);
    vec4 pixel = texture2D(texture, coord);

    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
}
