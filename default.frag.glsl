
uniform sampler2D texture;
attribute vec2 position;
varying vec2 _texcoord;

void main()
{
    gl_FragColor = texture2D(texture,_texcoord);
}
