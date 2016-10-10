all:
	echo "Building app now!"
	gcc main.cpp glfw/src/libglfw3.a glad/libglad.a -o main -I glad/include -I glfw/include -L. -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor -lGL -lstdc++ -lSOIL

clean:
	rm -f main
