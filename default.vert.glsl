
uniform mat4 pmatrix;

attribute vec2 position;
attribute vec2 texcoord;
varying vec2 _texcoord;

void main()
{
    gl_Position = pmatrix * vec4(position,0,1) + vec4(texcoord*0.01,0,0);
    _texcoord = texcoord;
}

