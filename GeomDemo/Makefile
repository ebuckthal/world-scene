all: linux

remake: clean all

mac:
	g++ Main.cpp GLSL_helper.cpp MStackHelp.cpp GeometryCreator.cpp -DGL_GLEXT_PROTOTYPES -framework OpenGL -framework GLUT -o GeomDemo

linux:
	g++ Main.cpp GLSL_helper.cpp MStackHelp.cpp GeometryCreator.cpp -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lglut -o GeomDemo

clean:
	rm -f *~
	rm -f GeomDemo
	rm -f *.o

