uniform sampler2D texture;
uniform float offset;

void main()
{
    // ������� UV �� X
    vec2 uv = gl_TexCoord[0].xy;
    uv.x += offset;

    gl_FragColor = texture2D(texture, uv);
}