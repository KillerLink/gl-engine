SOURCES= main.cpp shader.cpp FrameCounter.cpp SpaceNavigator.cpp
LIBRARIES=
INCLUDEDIRS= -I glad/include -I glfw/include -L glad -L glfw/src
LINKS= -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor -lGL -lstdc++ -lSOIL -lglad -lglfw3
EXECUTABLE= main

all:
	echo "Building app now!"
	gcc $(SOURCES) $(LIBRARIES) -o $(EXECUTABLE) $(INCLUDEDIRS) $(LINKS)
clean:
	rm -f $(EXECUTABLE)
