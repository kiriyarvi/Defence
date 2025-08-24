uniform sampler2D texture;

void main()
{
    vec4 texColor = texture2D(texture, gl_TexCoord[0].xy);
    float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    gl_FragColor = vec4(vec3(gray), texColor.a);
}