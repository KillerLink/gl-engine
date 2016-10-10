SOURCES= main.cpp shader.cpp
LIBRARIES=
INCLUDEDIRS= -I glad/include -I glfw/include -L. -L glad -L glfw/src
LINKS= -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor -lGL -lstdc++ -lSOIL -lglad -lglfw3
EXECUTABLE= main

all:
	echo "Building app now!"
	gcc $(SOURCES) $(LIBRARIES) -o $(EXECUTABLE) $(INCLUDEDIRS) $(LINKS)
clean:
	rm -f $(EXECUTABLE)
