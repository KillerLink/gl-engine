#ifndef __SHADER_HPP
#define __SHADER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader {
	public:
		GLuint program;
		Shader(const GLchar* vertexSourcePath, const GLchar* fragmentSourcePath);
		void use(void);
};

#endif
