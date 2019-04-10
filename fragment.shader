#version 330 core
out vec4 color;
in vec4 vertexColor;
in vec2 vertexTexc;
uniform sampler2D tex;
void main()
{
   color = vertexColor;
   //color = texture(tex, vertexTexc);
}
