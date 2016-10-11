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

#define test_bit(bit, array)  (array [bit / 8] & (1 << (bit % 8)))

int snavi;
int axes[6];
int buttons[2];
int led_state = 0;

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




//*
static const char* vertexShaderText =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec4 color;\n"
"layout (location = 2) in vec2 texc;\n"
"out vec4 vertexColor;\n"
"out vec2 vertexTexc;\n"
"uniform mat4 MVP;\n "
"void main()\n"
"{\n"
"   gl_Position = MVP * vec4(position, 1.0);\n"
"   vertexColor = color;\n"
"   vertexTexc = texc;\n"
"}\n";

static const char* fragmentShaderText =
"#version 330 core\n"
"out vec4 color;\n"
"in vec4 vertexColor;\n"
"in vec2 vertexTexc;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"   color = vertexColor;\n"
"   color = texture(tex, vertexTexc);"
"}\n";


static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int snavi_set_led (int snavi_dev, int led_state) {
  /* this does not seem to work with 2.6.19, it apparently
   * has worked with 2.6.8. Needs further investigation  */
  struct input_event event;
  int ret;

  event.type  = EV_LED;
  event.code  = LED_MISC;
  event.value = led_state;

  ret = write (snavi_dev, &event, sizeof (struct input_event));

  if (ret < 0)
    perror ("setting led state failed");

  return ret < sizeof (struct input_event);
}

void init_snavi(int argc, char *argv[]) {
  int i;
  char name[256]= "Unknown";
  u_int8_t evtype_bitmask[(EV_MAX + 7) / 8];
  u_int8_t rel_bitmask[(REL_MAX + 7) / 8];
  u_int8_t led_bitmask[(LED_MAX + 7) / 8];

  snavi = open (argc > 1 ? argv[1] : "/dev/input/by-id/usb-3Dconnexion_SpaceNavigator_for_Notebooks-event-if00", O_RDWR | O_NONBLOCK);

  if (snavi < 0)
    {
      perror ("opening the device failed");
      exit (1);
    }

    
  if (ioctl (snavi, EVIOCGNAME (sizeof (name)), name) < 0)
    {
      perror ("EVIOCGNAME ioctl failed");
      exit (1);
    }

  printf ("found \"%s\" on %s\n", name, argv[1]);

  if (ioctl (snavi, EVIOCGBIT (0, sizeof (evtype_bitmask)), evtype_bitmask) < 0)
    {
      perror ("EVIOCGBIT ioctl failed");
      exit (1);
    }

  printf ("Supported event types:\n");
  for (i = 0; i < EV_MAX; i++)
    {
      if (test_bit (i, evtype_bitmask))
        {
          /* this means that the bit is set in the event types list */
          printf("  Event type 0x%02x ", i);
          switch (i)
              {
              case EV_SYN :
                  printf(" (Sync?)\n");
                  break;
              case EV_KEY :
                  printf(" (Keys or Buttons)\n");
                  break;
              case EV_REL :
                  printf(" (Relative Axes)\n");
                  break;
              case EV_ABS :
                  printf(" (Absolute Axes)\n");
                  break;
              case EV_LED :
                  printf(" (LEDs)\n");
                  break;
              case EV_REP :
                  printf(" (Repeat)\n");
                  break;
              default:
                  printf(" (Unknown event type)\n");
              }
        }
    }

  memset (rel_bitmask, 0, sizeof (rel_bitmask));

  memset (led_bitmask, 0, sizeof (led_bitmask));
  if (ioctl (snavi, EVIOCGBIT (EV_LED, sizeof (led_bitmask)), led_bitmask) < 0)
    {
      perror ("EVIOCGBIT ioctl failed");
      exit (1);
    }

  printf ("detected leds:\n  ");
  for (i = 0; i < LED_MAX; i++)
    {
      if (test_bit (i, led_bitmask))
        printf ("%d, ", i);
    }
  printf ("\n");


  axes[0] = axes[1] = axes[2] = axes[3] = axes[4] = axes[5] = 0;
  buttons[0] = buttons[1] = 0;
}

void poll_snavi(void) {
  struct input_event event;
  for (int i=0; i<100; i++) {

    if (read (snavi, &event, sizeof (struct input_event)) < 0) {
      //perror ("my read error");
      return;
    }

    switch (event.type) {
      case EV_REL:
        if (event.code <= REL_RZ)
          axes[event.code - REL_X] = event.value;
        break;

      case EV_KEY:
        if (event.code >= BTN_0 && event.code <= BTN_1)
          buttons[event.code - BTN_0] = event.value;

        if (event.code - BTN_0 == 1) {
          led_state = 1 - led_state;
          snavi_set_led (snavi, led_state);
        }
      break;

      case EV_SYN:
        /* if multiple axes change simultaneously the linux
        * input system sends multiple EV_REL events. EV_SYN
        * then indicates that all changes have been reported.
        */
        fprintf (stderr,
        "\rState: %4d %4d %4d %4d %4d %4d - %3s %3s",
        axes[0], axes[1], axes[2], axes[3], axes[4], axes[5],
        buttons[0] ? "on" : "off",
        buttons[1] ? "on" : "off");
        //axes[0] = axes[1] = axes[2] = axes[3] = axes[4] = axes[5] = 0;
      break;

      case EV_LED:
      break;

      default:
        fprintf (stderr, "\nunknown event type %d\n", event.code);
    }
  }
}


int width, height;
float ratio;

int main(int argc, char *argv[]) {
	printf("Welcome to gl-engine!\n");
	printf("Currently the programm is looking for an 6-axis controller input devices and needs permission to open it!\n");
	printf("This hardcoded feature will become optional later!\n");
    init_snavi(argc,argv);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);
    
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
    
    while (!glfwWindowShouldClose(window)) {
        poll_snavi();
        glfwPollEvents();
        
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        
        for (int i=0; i<3; i++) {
          if (abs(axes[i]) > threshold) {
          if (axes[i]>threshold) {
            translations[i]+= postscaler*pow(prescaler * (axes[i] - threshold),3);
          }
          if (axes[i]<-threshold) {
            translations[i]-= postscaler*pow(prescaler * (-axes[i] - threshold),3);
            }
          }
        }
        for (int i=0; i<3; i++) {
          if (axes[i+3]>threshold) {
            rotations[i]+= postscaler*pow(prescaler * (axes[i+3] - threshold),3);
          }
          if (axes[i+3]<-threshold) {
            rotations[i]-= postscaler*pow(prescaler * (-axes[i+3] - threshold),3);
          }
        }
        if (buttons[0]) {
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
        float borderColor[] = {0.1f, 0.1f, 0.1f, 0.2f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        glBindVertexArray(VAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 6);
        /*
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
        */
        glLineWidth(1.0f);
        glDrawArrays(GL_TRIANGLES, 6, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 9, 14);
        glBindVertexArray(0);
            
        glfwSwapBuffers(window);
    }
    

    glfwDestroyWindow(window);
    glfwTerminate();
    close (snavi);
    return 0;
}
