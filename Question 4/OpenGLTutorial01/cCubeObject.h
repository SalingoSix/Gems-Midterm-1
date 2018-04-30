#ifndef _HG_cCubeObject_
#define _HG_cCubeObject_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class cCubeObject
{
public:
	cCubeObject();

	unsigned int VAO;

private:
	unsigned int VBO, EBO;
};

#endif
