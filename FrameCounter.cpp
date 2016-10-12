#include "FrameCounter.hpp"
#include <GLFW/glfw3.h>

FrameCounter::FrameCounter () {
	tlast = glfwGetTime();
	tcurrent = glfwGetTime();
	tdiff=0;
	nframes = 0;
};

double FrameCounter::tick(void){
	tdiff=tcurrent-tlast;
	tlast=tcurrent;
	tcurrent=glfwGetTime();
	nframes++;
	return tdiff*1000;
}

double FrameCounter::getTime(void) {
	return tdiff;
}

double FrameCounter::getFPS(void) {
	return (uint16_t)(1.0f/tdiff);
}
