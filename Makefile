linux: 
	g++ world-scene.cpp -o world-scene GLSL_helper.cpp MStackHelp.cpp GeometryCreator.cpp -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lglut

old: 
	g++ Main.cpp -o world-scene GLSL_helper.cpp MStackHelp.cpp GeometryCreator.cpp -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lglut
