uniform sampler2D texture;
uniform float selected;
uniform float editing;
uniform float time;
uniform vec4 textColor;

void main()
{
    // lookup the pixel in the texture
    vec2 coord = gl_TexCoord[0].xy;
    vec4 pixel = texture2D(texture, coord);

    // blend between not selected and selected color
    vec4 color = mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 0.2, 0.2, 1.0), selected);

    float sinVal = sin(time * 6);

    color = mix(color, mix(vec4(0.0, 1.0, 1.0, 1.0), vec4(1.0, 0.0, 1.0, 1.0), sinVal), editing);

    // multiply it by the color
    gl_FragColor = gl_Color * pixel * color * textColor;
}
