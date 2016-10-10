#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texc;
out vec4 vertexColor;
out vec2 vertexTexc;
uniform mat4 MVP;
void main()
{
   gl_Position = MVP * vec4(position, 1.0);
   vertexColor = color;
   vertexTexc = texc;
}
