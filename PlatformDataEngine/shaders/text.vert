uniform float selected;
uniform float editing;
uniform float time;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
    // transform the vertex position
    //float scale = map((sin(time) * editing), -1.0, 1.0, 1.0, 1.1);
    vec4 vertex = gl_Vertex;

    gl_Position = gl_ModelViewProjectionMatrix * vertex;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}