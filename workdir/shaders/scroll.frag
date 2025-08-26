uniform sampler2D texture;
uniform float offset;

void main()
{
    // Смещаем UV по X
    vec2 uv = gl_TexCoord[0].xy;
    uv.x += offset;

    gl_FragColor = texture2D(texture, uv);
}