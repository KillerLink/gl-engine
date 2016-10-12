#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/types.h>
#include <linux/input.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <SOIL/SOIL.h>

#include "shader.hpp"
#include "FrameCounter.hpp"
#include "SpaceNavigator.hpp"


typedef struct {
    GLfloat x, y, z;
    GLfloat r, g, b;
    GLfloat alpha;
    GLfloat tx, ty;
} vertice_t;

typedef struct {
  GLint width, height;
  unsigned char* image;
} image_t;


vertice_t cube[] = {
  {-1.0f, -1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f,   -0.1f, -0.1f},
  {-1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,   1.1f, -0.1f},
  {1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,   1.1f, 1.1f},
  {1.0f, -1.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,   -0.1f, 1.1f},
  {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   1.1f, 1.1f}, //
  {-1.0f, 1.0f, -1.0f,  0.0f, 1.0f, 1.0f, 1.0f,   -0.1f, 1.1f},
  {1.0f, 1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   -0.1f, -0.1f},
  {1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f,   1.1f, -0.1f},
  {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.f,   0.0f, 0.0f}
};

vertice_t tvert[] = {
  { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
  { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
  { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
  { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
  { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
  { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
  
  { -0.5f,  -0.5f,  0.1f, 1.0f, 0.0f, 0.0f, 1.0f},
  { 0.5f,  -0.5f,  0.1f, 0.0f, 1.0f, 0.0f, 1.0f},
  { 0.0f,  0.5f,  0.1f, 0.0f, 0.0f, 1.0f, 1.0f},
  
  cube[1],cube[2],cube[5],cube[6],
  cube[7],cube[2],cube[3],cube[1],
  cube[0],cube[5],cube[4],cube[7],
  cube[0],cube[3]
  
};

GLuint indices[] = {
  0,1,2,3,4,5
};

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


int width, height;
float ratio;

int main(int argc, char *argv[]) {
	printf("Welcome to gl-engine!\n");
	printf("Currently the programm is looking for an 6-axis controller input devices and needs permission to open it!\n");
	printf("This hardcoded feature will become optional later!\n");
    
    SpaceNavigator snav;
    snav.init();
    
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window = glfwCreateWindow(1280,960,"LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
      printf("Failed to create GLFW window!\n");
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glViewport(0, 0, width, height);
    glfwSetKeyCallback(window, key_callback);
    
    GLuint vertexBuffer;
    GLuint VAO, EBO;
    GLfloat MVPlocation, VertexLocation, ColorLocation, TexcLocation;
    
	Shader myShader("vertex.shader","fragment.shader");
    myShader.use();
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    MVPlocation = glGetUniformLocation(myShader.program, "MVP");
    VertexLocation = glGetAttribLocation(myShader.program,"position");
    ColorLocation = glGetAttribLocation(myShader.program,"color");
    TexcLocation = glGetAttribLocation(myShader.program,"texc");
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tvert), tvert, GL_STATIC_DRAW);
    glVertexAttribPointer(VertexLocation, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLfloat*)0 );
    glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLfloat*)12 );
    glVertexAttribPointer(TexcLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLfloat*)28 );
	glEnableVertexAttribArray(VertexLocation);
	glEnableVertexAttribArray(ColorLocation);
	glEnableVertexAttribArray(TexcLocation);
    glBindVertexArray(0);
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);
    
    image_t img;
    img.image = SOIL_load_image("resources/container.jpg",&img.width, &img.height, 0, SOIL_LOAD_RGB);
    printf ("Image loaded: %dx%d\n", img.width, img.height);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(img.image);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    float translations[3] = {0.0f, 0.0f, -5.0f};
    float rotations[3] = {0.0f, 10.0f, 20.0f};
    float threshold = 180.0f;
    float prescaler = 0.017f;
    float postscaler = 0.017f;
    
    FrameCounter fps;
    
    
    while (!glfwWindowShouldClose(window)) {

		char title[128];
		sprintf(title,"%4.2f ms <--> %4.2f FPS",fps.tick(),fps.getFPS());
		glfwSetWindowTitle (window, title);

        snav.poll();
        glfwPollEvents();
        
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        
        for (int i=0; i<3; i++) {
          if (snav.axes[i]>threshold) {
            translations[i]+= postscaler*pow(prescaler * (snav.axes[i] - threshold),3);
          }
          if (snav.axes[i]<-threshold) {
            translations[i]-= postscaler*pow(prescaler * (-snav.axes[i] - threshold),3);
		  }
        }
        for (int i=0; i<3; i++) {
          if (snav.axes[i+3]>threshold) {
            rotations[i]+= postscaler*pow(prescaler * (snav.axes[i+3] - threshold),3);
          }
          if (snav.axes[i+3]<-threshold) {
            rotations[i]-= postscaler*pow(prescaler * (-snav.axes[i+3] - threshold),3);
          }
        }
        if (snav.buttons[0]) {
			snav.reset_state();
			translations[0]=translations[1]=translations[2]=0.0f;
			rotations[0]=rotations[1]=rotations[2]=0;
        }
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe mode
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //normal mode
        
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.f);
        glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(translations[0],-translations[1],translations[2]));
        View = glm::rotate(View, rotations[2], glm::vec3(0.0f, 0.0f, 1.0f));
        View = glm::rotate(View, rotations[1], glm::vec3(-1.0f, 0.0f, 0.0f));
        View = glm::rotate(View, rotations[0], glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        //View = lookAt(glm::vec3(translations[0],-translations[1],translations[2]),glm::vec3(0.0,0.0,0.0),glm::vec3(0.0,0.0,1.0));
        glm::mat4 mvp = Projection * View * Model ;
        glUniformMatrix4fv(MVPlocation, 1, GL_FALSE, &mvp[0][0]);

        
        glClearColor(0.5f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glViewport(0, 0, width, height);
    
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        glBindVertexArray(VAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 6);
        glLineWidth(1.0f);
        glDrawArrays(GL_TRIANGLES, 6, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 9, 14);
        glBindVertexArray(0);
            
        glfwSwapBuffers(window);
    }
    

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
