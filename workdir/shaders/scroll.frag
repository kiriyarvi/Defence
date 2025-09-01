uniform sampler2D texture;
uniform float offset_x = 0;
uniform float offset_y = 0;

void main()
{
    // Смещаем UV по X
    vec2 uv = gl_TexCoord[0].xy;
    uv.x += offset_x;
	uv.y += offset_y;
    gl_FragColor = texture2D(texture, uv);
}