#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	ObjekPetikemasOrange();
	ObjekPetikemasYellow();
	ObjekPetikemasGreen();
	road();
	//BuildTexturedCube();
	BuildTexturedPlane();
	InitCamera();

}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glDeleteBuffers(1, &depthMapFBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO2);
	glDeleteVertexArrays(1, &VAO3);
	glDeleteBuffers(1, &VBO3);
	glDeleteBuffers(1, &EBO3);
	glDeleteVertexArrays(1, &VAO4);
	glDeleteBuffers(1, &VBO4);
	glDeleteBuffers(1, &EBO4);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}
	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}
	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;
	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}
	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 1000;
	angleZ = (float)((midY - mouseY)) / 1000;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 8) {
		viewCamY = posCamY + 8;
	}
	if ((viewCamY - posCamY) < -8) {
		viewCamY = posCamY - 8;
	}
	RotateCamera(-angleY);
}



void Demo::Update(double deltaTime) {
}

void Demo::Render() {
	
	glEnable(GL_DEPTH_TEST);

	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearColor(0.0f, 0.5f, 0.875f, 1.0f);
	
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawTexturedCube(this->depthmapShader);
	DrawTexturedPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	

	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(fovy, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(this->shadowmapShader, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	
	//glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	//glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	// LookAt camera (position, target/direction, up)

	// LookAt camera (position, target/direction, up)
	glm::mat4 view = glm::lookAt(glm::vec3(posCamX, posCamY, posCamZ), glm::vec3(viewCamX, viewCamY, viewCamZ), glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shadowmapShader, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//glm::vec3 cameraPos = glm::vec3(0, 5, 2);
	//glm::vec3 cameraFront = glm::vec3(0, 0, 0);
	//glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(0, 1, 0));
	//glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), posCamX, posCamY, posCamZ);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedPlane(this->shadowmapShader);
	
	// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedCube(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, stexture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredCubeOrange(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredCubeYellow(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredCubeGreen(this->shadowmapShader);

	glDisable(GL_DEPTH_TEST);
}

void Demo::ObjekPetikemasOrange() {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("petikemas0.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	float vertices[] = {
		//PETIKEMAS 1
		// front
		-1.5, -0.5, 0.5, 0, 0,  // 0
		1.5, -0.5, 0.5, 1, 0,   // 1
		1.5, 0.5, 0.5, 1, 1,   // 2
		-1.5, 0.5, 0.5, 0, 1,  // 3
		// right
		1.5, 0.5,  0.5, 0, 0,  // 4
		1.5, 0.5, -0.5, 1, 0,  // 5
		1.5, -0.5, -0.5, 1, 1,  // 6
		1.5, -0.5,  0.5, 0, 1,  // 7
		// back
		-1.5, -0.5, -0.5, 0, 0, // 8 
		1.5, -0.5, -0.5, 1, 0, // 9
		1.5, 0.5, -0.5, 1, 1, // 10
		-1.5, 0.5, -0.5, 0, 1, // 11
		// left
		-1.5, -0.5, -0.5, 0, 0, // 12
		-1.5, -0.5, 0.5, 1, 0, // 13
		-1.5, 0.5, 0.5, 1, 1, // 14
		-1.5, 0.5, -0.5, 0, 1, // 15
		// upper
		1.5, 0.5, 0.5, 0, 0,   // 16
		-1.5, 0.5, 0.5, 1, 0,  // 17
		-1.5, 0.5, -0.5, 1, 1,  // 18
		1.5, 0.5, -0.5, 0, 1,   // 19
		// bottom
		-1.5, -0.5, -0.5, 0, 0, // 20
		1.5, -0.5, -0.5, 1, 0,  // 21
		1.5, -0.5, 0.5, 1, 1,  // 22
		-1.5, -0.5, 0.5, 0, 1, // 23

		//PETIKEMAS 2
		// front
		-1.5, -0.5, 1.6, 0, 0,  // 24
		1.5, -0.5, 1.6, 1, 0,   // 25
		1.5, 0.5, 1.6, 1, 1,   // 26
		-1.5, 0.5, 1.6, 0, 1,  // 27
		// right
		1.5, 0.5,  1.6, 0, 0,  // 28
		1.5, 0.5, 0.6, 1, 0,  // 29
		1.5, -0.5, 0.6, 1, 1,  // 30
		1.5, -0.5,  1.6, 0, 1,  // 31
		// back
		-1.5, -0.5, 0.6, 0, 0, // 32
		1.5, -0.5, 0.6, 1, 0, // 33
		1.5, 0.5, 0.6, 1, 1, // 34
		-1.5, 0.5, 0.6, 0, 1, // 35
		// left
		-1.5, -0.5, 0.6, 0, 0, // 36
		-1.5, -0.5, 1.6, 1, 0, // 37
		-1.5, 0.5, 1.6, 1, 1, // 38
		-1.5, 0.5, 0.6, 0, 1, // 39
		// upper
		1.5, 0.5, 1.6, 0, 0,   // 40
		-1.5, 0.5, 1.6, 1, 0,  // 41
		-1.5, 0.5, 0.6, 1, 1,  // 42
		1.5, 0.5, 0.6, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 0.6, 0, 0, // 44
		1.5, -0.5, 0.6, 1, 0,  // 45
		1.5, -0.5, 1.6, 1, 1,  // 46
		-1.5, -0.5, 1.6, 0, 1, // 47

		//PETIKEMAS 3
		// front
		-1.5, -0.5, 2.7, 0, 0,  // 48
		1.5, -0.5, 2.7, 1, 0,   // 25
		1.5, 0.5, 2.7, 1, 1,   // 26
		-1.5, 0.5, 2.7, 0, 1,  // 27
		// right
		1.5, 0.5,  2.7, 0, 0,  // 28
		1.5, 0.5, 1.7, 1, 0,  // 29
		1.5, -0.5, 1.7, 1, 1,  // 30
		1.5, -0.5,  2.7, 0, 1,  // 31
		// back
		-1.5, -0.5, 1.7, 0, 0, // 32
		1.5, -0.5, 1.7, 1, 0, // 33
		1.5, 0.5, 1.7, 1, 1, // 34
		-1.5, 0.5, 1.7, 0, 1, // 35
		// left
		-1.5, -0.5, 1.7, 0, 0, // 36
		-1.5, -0.5, 2.7, 1, 0, // 37
		-1.5, 0.5, 2.7, 1, 1, // 38
		-1.5, 0.5, 1.7, 0, 1, // 39
		// upper
		1.5, 0.5, 2.7, 0, 0,   // 40
		-1.5, 0.5, 2.7, 1, 0,  // 41
		-1.5, 0.5, 1.7, 1, 1,  // 42
		1.5, 0.5, 1.7, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 1.7, 0, 0, // 44
		1.5, -0.5, 1.7, 1, 0,  // 45
		1.5, -0.5, 2.7, 1, 1,  // 46
		-1.5, -0.5, 2.7, 0, 1, // 47

		//PETIKEMAS 4
		// front
		-1.5, -0.5, 3.8, 0, 0,  // 48
		1.5, -0.5, 3.8, 1, 0,   // 25
		1.5, 0.5, 3.8, 1, 1,   // 26
		-1.5, 0.5, 3.8, 0, 1,  // 27
		// right
		1.5, 0.5, 3.8, 0, 0,  // 28
		1.5, 0.5, 2.8, 1, 0,  // 29
		1.5, -0.5, 2.8, 1, 1,  // 30
		1.5, -0.5, 3.8, 0, 1,  // 31
		// back
		-1.5, -0.5, 2.8, 0, 0, // 32
		1.5, -0.5, 2.8, 1, 0, // 33
		1.5, 0.5, 2.8, 1, 1, // 34
		-1.5, 0.5, 2.8, 0, 1, // 35
		// left
		-1.5, -0.5, 2.8, 0, 0, // 36
		-1.5, -0.5, 3.8, 1, 0, // 37
		-1.5, 0.5, 3.8, 1, 1, // 38
		-1.5, 0.5, 2.8, 0, 1, // 39
		// upper
		1.5, 0.5, 3.8, 0, 0,   // 40
		-1.5, 0.5, 3.8, 1, 0,  // 41
		-1.5, 0.5, 2.8, 1, 1,  // 42
		1.5, 0.5, 2.8, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 2.8, 0, 0, // 44
		1.5, -0.5, 2.8, 1, 0,  // 45
		1.5, -0.5, 3.8, 1, 1,  // 46
		-1.5, -0.5, 3.8, 0, 1, // 47

		//PETIKEMAS 5
		// front
		-1.5, -0.5, 4.9, 0, 0,  // 48
		1.5, -0.5, 4.9, 1, 0,   // 25
		1.5, 0.5, 4.9, 1, 1,   // 26
		-1.5, 0.5, 4.9, 0, 1,  // 27
		// right
		1.5, 0.5, 4.9, 0, 0,  // 28
		1.5, 0.5, 3.9, 1, 0,  // 29
		1.5, -0.5, 3.9, 1, 1,  // 30
		1.5, -0.5, 4.9, 0, 1,  // 31
		// back
		-1.5, -0.5, 3.9, 0, 0, // 32
		1.5, -0.5, 3.9, 1, 0, // 33
		1.5, 0.5, 3.9, 1, 1, // 34
		-1.5, 0.5, 3.9, 0, 1, // 35
		// left
		-1.5, -0.5, 3.9, 0, 0, // 36
		-1.5, -0.5, 4.9, 1, 0, // 37
		-1.5, 0.5, 4.9, 1, 1, // 38
		-1.5, 0.5, 3.9, 0, 1, // 39
		// upper
		1.5, 0.5, 3.9, 0, 0,   // 40
		-1.5, 0.5, 3.9, 1, 0,  // 41
		-1.5, 0.5, 4.9, 1, 1,  // 42
		1.5, 0.5, 4.9, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 3.9, 0, 0, // 44
		1.5, -0.5, 3.9, 1, 0,  // 45
		1.5, -0.5, 4.9, 1, 1,  // 46
		-1.5, -0.5, 4.9, 0, 1, // 47

		//PETIKEMAS 6
		// front
		-1.5, -0.5, 8.0, 0, 0,  // 48
		1.5, -0.5, 8.0, 1, 0,   // 25
		1.5, 0.5, 8.0, 1, 1,   // 26
		-1.5, 0.5, 8.0, 0, 1,  // 27
		// right
		1.5, 0.5, 8.0, 0, 0,  // 28
		1.5, 0.5, 7.0, 1, 0,  // 29
		1.5, -0.5, 7.0, 1, 1,  // 30
		1.5, -0.5, 8.0, 0, 1,  // 31
		// back
		-1.5, -0.5, 7.0, 0, 0, // 32
		1.5, -0.5, 7.0, 1, 0, // 33
		1.5, 0.5, 7.0, 1, 1, // 34
		-1.5, 0.5, 7.0, 0, 1, // 35
		// left
		-1.5, -0.5, 7.0, 0, 0, // 36
		-1.5, -0.5, 8.0, 1, 0, // 37
		-1.5, 0.5, 8.0, 1, 1, // 38
		-1.5, 0.5, 7.0, 0, 1, // 39
		// upper
		1.5, 0.5, 7.0, 0, 0,   // 40
		-1.5, 0.5, 7.0, 1, 0,  // 41
		-1.5, 0.5, 8.0, 1, 1,  // 42
		1.5, 0.5, 8.0, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 7.0, 0, 0, // 44
		1.5, -0.5, 7.0, 1, 0,  // 45
		1.5, -0.5, 8.0, 1, 1,  // 46
		-1.5, -0.5, 8.0, 0, 1, // 47

		//PETIKEMAS 7
		// front
		-1.5, -0.5, 9.1, 0, 0,  // 48
		1.5, -0.5, 9.1, 1, 0,   // 25
		1.5, 0.5, 9.1, 1, 1,   // 26
		-1.5, 0.5, 9.1, 0, 1,  // 27
		// right
		1.5, 0.5, 9.1, 0, 0,  // 28
		1.5, 0.5, 8.1, 1, 0,  // 29
		1.5, -0.5, 8.1, 1, 1,  // 30
		1.5, -0.5, 9.1, 0, 1,  // 31
		// back
		-1.5, -0.5, 8.1, 0, 0, // 32
		1.5, -0.5, 8.1, 1, 0, // 33
		1.5, 0.5, 8.1, 1, 1, // 34
		-1.5, 0.5, 8.1, 0, 1, // 35
		// left
		-1.5, -0.5, 8.1, 0, 0, // 36
		-1.5, -0.5, 9.1, 1, 0, // 37
		-1.5, 0.5, 9.1, 1, 1, // 38
		-1.5, 0.5, 8.1, 0, 1, // 39
		// upper
		1.5, 0.5, 8.1, 0, 0,   // 40
		-1.5, 0.5, 8.1, 1, 0,  // 41
		-1.5, 0.5, 9.1, 1, 1,  // 42
		1.5, 0.5, 9.1, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 8.1, 0, 0, // 44
		1.5, -0.5, 8.1, 1, 0,  // 45
		1.5, -0.5, 9.1, 1, 1,  // 46
		-1.5, -0.5, 9.1, 0, 1, // 47

		//PETIKEMAS 8
		// front
		-1.5, -0.5, 10.2, 0, 0,  // 48
		1.5, -0.5, 10.2, 1, 0,   // 25
		1.5, 0.5, 10.2, 1, 1,   // 26
		-1.5, 0.5, 10.2, 0, 1,  // 27
		// right
		1.5, 0.5, 10.2, 0, 0,  // 28
		1.5, 0.5, 9.2, 1, 0,  // 29
		1.5, -0.5, 9.2, 1, 1,  // 30
		1.5, -0.5, 10.2, 0, 1,  // 31
		// back
		-1.5, -0.5, 9.2, 0, 0, // 32
		1.5, -0.5, 9.2, 1, 0, // 33
		1.5, 0.5, 9.2, 1, 1, // 34
		-1.5, 0.5, 9.2, 0, 1, // 35
		// left
		-1.5, -0.5, 9.2, 0, 0, // 36
		-1.5, -0.5, 10.2, 1, 0, // 37
		-1.5, 0.5, 10.2, 1, 1, // 38
		-1.5, 0.5, 9.2, 0, 1, // 39
		// upper
		1.5, 0.5, 9.2, 0, 0,   // 40
		-1.5, 0.5, 9.2, 1, 0,  // 41
		-1.5, 0.5, 10.2, 1, 1,  // 42
		1.5, 0.5, 10.2, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 9.2, 0, 0, // 44
		1.5, -0.5, 9.2, 1, 0,  // 45
		1.5, -0.5, 10.2, 1, 1,  // 46
		-1.5, -0.5, 10.2, 0, 1, // 47

		//PETIKEMAS 9
		// front
		-1.5, -0.5, 11.3, 0, 0,  // 48
		1.5, -0.5, 11.3, 1, 0,   // 25
		1.5, 0.5, 11.3, 1, 1,   // 26
		-1.5, 0.5, 11.3, 0, 1,  // 27
		// right
		1.5, 0.5, 11.3, 0, 0,  // 28
		1.5, 0.5, 10.3, 1, 0,  // 29
		1.5, -0.5, 10.3, 1, 1,  // 30
		1.5, -0.5, 11.3, 0, 1,  // 31
		// back
		-1.5, -0.5, 10.3, 0, 0, // 32
		1.5, -0.5, 10.3, 1, 0, // 33
		1.5, 0.5, 10.3, 1, 1, // 34
		-1.5, 0.5, 10.3, 0, 1, // 35
		// left
		-1.5, -0.5, 10.3, 0, 0, // 36
		-1.5, -0.5, 11.3, 1, 0, // 37
		-1.5, 0.5, 11.3, 1, 1, // 38
		-1.5, 0.5, 10.3, 0, 1, // 39
		// upper
		1.5, 0.5, 10.3, 0, 0,   // 40
		-1.5, 0.5, 10.3, 1, 0,  // 41
		-1.5, 0.5, 11.3, 1, 1,  // 42
		1.5, 0.5, 11.3, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 10.3, 0, 0, // 44
		1.5, -0.5, 10.3, 1, 0,  // 45
		1.5, -0.5, 11.3, 1, 1,  // 46
		-1.5, -0.5, 11.3, 0, 1, // 47

		//PETIKEMAS 10
		// front
		-1.5, -0.5, 12.4, 0, 0,  // 48
		1.5, -0.5, 12.4, 1, 0,   // 25
		1.5, 0.5, 12.4, 1, 1,   // 26
		-1.5, 0.5, 12.4, 0, 1,  // 27
		// right
		1.5, 0.5, 12.4, 0, 0,  // 28
		1.5, 0.5, 11.4, 1, 0,  // 29
		1.5, -0.5, 11.4, 1, 1,  // 30
		1.5, -0.5, 12.4, 0, 1,  // 31
		// back
		-1.5, -0.5, 11.4, 0, 0, // 32
		1.5, -0.5, 11.4, 1, 0, // 33
		1.5, 0.5, 11.4, 1, 1, // 34
		-1.5, 0.5, 11.4, 0, 1, // 35
		// left
		-1.5, -0.5, 11.4, 0, 0, // 36
		-1.5, -0.5, 12.4, 1, 0, // 37
		-1.5, 0.5, 12.4, 1, 1, // 38
		-1.5, 0.5, 11.4, 0, 1, // 39
		// upper
		1.5, 0.5, 11.4, 0, 0,   // 40
		-1.5, 0.5, 11.4, 1, 0,  // 41
		-1.5, 0.5, 12.4, 1, 1,  // 42
		1.5, 0.5, 12.4, 0, 1,   // 43
		// bottom
		-1.5, -0.5, 11.4, 0, 0, // 44
		1.5, -0.5, 11.4, 1, 0,  // 45
		1.5, -0.5, 12.4, 1, 1,  // 46
		-1.5, -0.5, 12.4, 0, 1, // 47

		//DIATAS KUNING
		//PETIKEMAS 11
		// front
		-4.6, 0.5, 0.5, 0, 0,  // 0
		-1.6, 0.5, 0.5, 1, 0,   // 1
		-1.6, 1.5, 0.5, 1, 1,   // 2
		-4.6, 1.5, 0.5, 0, 1,  // 3
		// right
		-1.6, 1.5,  0.5, 0, 0,  // 4
		-1.6, 1.5, -0.5, 1, 0,  // 5
		-1.6, 0.5, -0.5, 1, 1,  // 6
		-1.6, 0.5,  0.5, 0, 1,  // 7
		// back
		-4.6, 0.5, -0.5, 0, 0, // 8 
		-1.6, 0.5, -0.5, 1, 0, // 9
		-1.6, 1.5, -0.5, 1, 1, // 10
		-4.6, 1.5, -0.5, 0, 1, // 11
		// left
		-4.6, 0.5, -0.5, 0, 0, // 12
		-4.6, 0.5, 0.5, 1, 0, // 13
		-4.6, 1.5, 0.5, 1, 1, // 14
		-4.6, 1.5, -0.5, 0, 1, // 15
		// upper
		-1.6, 1.5, 0.5, 0, 0,   // 16
		-4.6, 1.5, 0.5, 1, 0,  // 17
		-4.6, 1.5, -0.5, 1, 1,  // 18
		-1.6, 1.5, -0.5, 0, 1,   // 19
		// bottom
		-4.6, 0.5, -0.5, 0, 0, // 20
		-1.6, 0.5, -0.5, 1, 0,  // 21
		-1.6, 0.5, 0.5, 1, 1,  // 22
		-4.6, 0.5, 0.5, 0, 1, // 23

		//PETIKEMAS 12
		// front
		-4.6, 0.5, 1.6, 0, 0,  // 24
		-1.6, 0.5, 1.6, 1, 0,   // 25
		-1.6, 1.5, 1.6, 1, 1,   // 26
		-4.6, 1.5, 1.6, 0, 1,  // 27
		// right
		-1.6, 1.5,  1.6, 0, 0,  // 28
		-1.6, 1.5, 0.6, 1, 0,  // 29
		-1.6, 0.5, 0.6, 1, 1,  // 30
		-1.6, 0.5,  1.6, 0, 1,  // 31
		// back
		-4.6, 0.5, 0.6, 0, 0, // 32
		-1.6, 0.5, 0.6, 1, 0, // 33
		-1.6, 1.5, 0.6, 1, 1, // 34
		-4.6, 1.5, 0.6, 0, 1, // 35
		// left
		-4.6, 0.5, 0.6, 0, 0, // 36
		-4.6, 0.5, 1.6, 1, 0, // 37
		-4.6, 1.5, 1.6, 1, 1, // 38
		-4.6, 1.5, 0.6, 0, 1, // 39
		// upper
		-1.6, 1.5, 1.6, 0, 0,   // 40
		-4.6, 1.5, 1.6, 1, 0,  // 41
		-4.6, 1.5, 0.6, 1, 1,  // 42
		-1.6, 1.5, 0.6, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 0.6, 0, 0, // 44
		-1.6, 0.5, 0.6, 1, 0,  // 45
		-1.6, 0.5, 1.6, 1, 1,  // 46
		-4.6, 0.5, 1.6, 0, 1, // 47

		//PETIKEMAS 13
		// front
		-4.6, 0.5, 2.7, 0, 0,  // 48
		-1.6, 0.5, 2.7, 1, 0,   // 25
		-1.6, 1.5, 2.7, 1, 1,   // 26
		-4.6, 1.5, 2.7, 0, 1,  // 27
		// right
		-1.6, 1.5,  2.7, 0, 0,  // 28
		-1.6, 1.5, 1.7, 1, 0,  // 29
		-1.6, 0.5, 1.7, 1, 1,  // 30
		-1.6, 0.5,  2.7, 0, 1,  // 31
		// back
		-4.6, 0.5, 1.7, 0, 0, // 32
		-1.6, 0.5, 1.7, 1, 0, // 33
		-1.6, 1.5, 1.7, 1, 1, // 34
		-4.6, 1.5, 1.7, 0, 1, // 35
		// left
		-4.6, 0.5, 1.7, 0, 0, // 36
		-4.6, 0.5, 2.7, 1, 0, // 37
		-4.6, 1.5, 2.7, 1, 1, // 38
		-4.6, 1.5, 1.7, 0, 1, // 39
		// upper
		-1.6, 1.5, 2.7, 0, 0,   // 40
		-4.6, 1.5, 2.7, 1, 0,  // 41
		-4.6, 1.5, 1.7, 1, 1,  // 42
		-1.6, 1.5, 1.7, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 1.7, 0, 0, // 44
		-1.6, 0.5, 1.7, 1, 0,  // 45
		-1.6, 0.5, 2.7, 1, 1,  // 46
		-4.6, 0.5, 2.7, 0, 1, // 47

		//PETIKEMAS 14
		// front
		-4.6, 0.5, 3.8, 0, 0,  // 48
		-1.6, 0.5, 3.8, 1, 0,   // 25
		-1.6, 1.5, 3.8, 1, 1,   // 26
		-4.6, 1.5, 3.8, 0, 1,  // 27
		// right
		-1.6, 1.5, 3.8, 0, 0,  // 28
		-1.6, 1.5, 2.8, 1, 0,  // 29
		-1.6, 0.5, 2.8, 1, 1,  // 30
		-1.6, 0.5, 3.8, 0, 1,  // 31
		// back
		-4.6, 0.5, 2.8, 0, 0, // 32
		-1.6, 0.5, 2.8, 1, 0, // 33
		-1.6, 1.5, 2.8, 1, 1, // 34
		-4.6, 1.5, 2.8, 0, 1, // 35
		// left
		-4.6, 0.5, 2.8, 0, 0, // 36
		-4.6, 0.5, 3.8, 1, 0, // 37
		-4.6, 1.5, 3.8, 1, 1, // 38
		-4.6, 1.5, 2.8, 0, 1, // 39
		// upper
		-1.6, 1.5, 3.8, 0, 0,   // 40
		-4.6, 1.5, 3.8, 1, 0,  // 41
		-4.6, 1.5, 2.8, 1, 1,  // 42
		-1.6, 1.5, 2.8, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 2.8, 0, 0, // 44
		-1.6, 0.5, 2.8, 1, 0,  // 45
		-1.6, 0.5, 3.8, 1, 1,  // 46
		-4.6, 0.5, 3.8, 0, 1, // 47

		//PETIKEMAS 15
		// front
		-4.6, 0.5, 4.9, 0, 0,  // 48
		-1.6, 0.5, 4.9, 1, 0,   // 25
		-1.6, 1.5, 4.9, 1, 1,   // 26
		-4.6, 1.5, 4.9, 0, 1,  // 27
		// right
		-1.6, 1.5, 4.9, 0, 0,  // 28
		-1.6, 1.5, 3.9, 1, 0,  // 29
		-1.6, 0.5, 3.9, 1, 1,  // 30
		-1.6, 0.5, 4.9, 0, 1,  // 31
		// back
		-4.6, 0.5, 3.9, 0, 0, // 32
		-1.6, 0.5, 3.9, 1, 0, // 33
		-1.6, 1.5, 3.9, 1, 1, // 34
		-4.6, 1.5, 3.9, 0, 1, // 35
		// left
		-4.6, 0.5, 3.9, 0, 0, // 36
		-4.6, 0.5, 4.9, 1, 0, // 37
		-4.6, 1.5, 4.9, 1, 1, // 38
		-4.6, 1.5, 3.9, 0, 1, // 39
		// upper
		-1.6, 1.5, 3.9, 0, 0,   // 40
		-4.6, 1.5, 3.9, 1, 0,  // 41
		-4.6, 1.5, 4.9, 1, 1,  // 42
		-1.6, 1.5, 4.9, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 3.9, 0, 0, // 44
		-1.6, 0.5, 3.9, 1, 0,  // 45
		-1.6, 0.5, 4.9, 1, 1,  // 46
		-4.6, 0.5, 4.9, 0, 1, // 47

		//PETIKEMAS 16
		// front
		-4.6, 0.5, 8.0, 0, 0,  // 48
		-1.6, 0.5, 8.0, 1, 0,   // 25
		-1.6, 1.5, 8.0, 1, 1,   // 26
		-4.6, 1.5, 8.0, 0, 1,  // 27
		// right
		-1.6, 1.5, 8.0, 0, 0,  // 28
		-1.6, 1.5, 7.0, 1, 0,  // 29
		-1.6, 0.5, 7.0, 1, 1,  // 30
		-1.6, 0.5, 8.0, 0, 1,  // 31
		// back
		-4.6, 0.5, 7.0, 0, 0, // 32
		-1.6, 0.5, 7.0, 1, 0, // 33
		-1.6, 1.5, 7.0, 1, 1, // 34
		-4.6, 1.5, 7.0, 0, 1, // 35
		// left
		-4.6, 0.5, 7.0, 0, 0, // 36
		-4.6, 0.5, 8.0, 1, 0, // 37
		-4.6, 1.5, 8.0, 1, 1, // 38
		-4.6, 1.5, 7.0, 0, 1, // 39
		// upper
		-1.6, 1.5, 7.0, 0, 0,   // 40
		-4.6, 1.5, 7.0, 1, 0,  // 41
		-4.6, 1.5, 8.0, 1, 1,  // 42
		-1.6, 1.5, 8.0, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 7.0, 0, 0, // 44
		-1.6, 0.5, 7.0, 1, 0,  // 45
		-1.6, 0.5, 8.0, 1, 1,  // 46
		-4.6, 0.5, 8.0, 0, 1, // 47

		//PETIKEMAS 17
		// front
		-4.6, 0.5, 9.1, 0, 0,  // 48
		-1.6, 0.5, 9.1, 1, 0,   // 25
		-1.6, 1.5, 9.1, 1, 1,   // 26
		-4.6, 1.5, 9.1, 0, 1,  // 27
		// right
		-1.6, 1.5, 9.1, 0, 0,  // 28
		-1.6, 1.5, 8.1, 1, 0,  // 29
		-1.6, 0.5, 8.1, 1, 1,  // 30
		-1.6, 0.5, 9.1, 0, 1,  // 31
		// back
		-4.6, 0.5, 8.1, 0, 0, // 32
		-1.6, 0.5, 8.1, 1, 0, // 33
		-1.6, 1.5, 8.1, 1, 1, // 34
		-4.6, 1.5, 8.1, 0, 1, // 35
		// left
		-4.6, 0.5, 8.1, 0, 0, // 36
		-4.6, 0.5, 9.1, 1, 0, // 37
		-4.6, 1.5, 9.1, 1, 1, // 38
		-4.6, 1.5, 8.1, 0, 1, // 39
		// upper
		-1.6, 1.5, 8.1, 0, 0,   // 40
		-4.6, 1.5, 8.1, 1, 0,  // 41
		-4.6, 1.5, 9.1, 1, 1,  // 42
		-1.6, 1.5, 9.1, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 8.1, 0, 0, // 44
		-1.6, 0.5, 8.1, 1, 0,  // 45
		-1.6, 0.5, 9.1, 1, 1,  // 46
		-4.6, 0.5, 9.1, 0, 1, // 47

		//PETIKEMAS 18
		// front
		-4.6, 0.5, 10.2, 0, 0,  // 48
		-1.6, 0.5, 10.2, 1, 0,   // 25
		-1.6, 1.5, 10.2, 1, 1,   // 26
		-4.6, 1.5, 10.2, 0, 1,  // 27
		// right
		-1.6, 1.5, 10.2, 0, 0,  // 28
		-1.6, 1.5, 9.2, 1, 0,  // 29
		-1.6, 0.5, 9.2, 1, 1,  // 30
		-1.6, 0.5, 10.2, 0, 1,  // 31
		// back
		-4.6, 0.5, 9.2, 0, 0, // 32
		-1.6, 0.5, 9.2, 1, 0, // 33
		-1.6, 1.5, 9.2, 1, 1, // 34
		-4.6, 1.5, 9.2, 0, 1, // 35
		// left
		-4.6, 0.5, 9.2, 0, 0, // 36
		-4.6, 0.5, 10.2, 1, 0, // 37
		-4.6, 1.5, 10.2, 1, 1, // 38
		-4.6, 1.5, 9.2, 0, 1, // 39
		// upper
		-1.6, 1.5, 9.2, 0, 0,   // 40
		-4.6, 1.5, 9.2, 1, 0,  // 41
		-4.6, 1.5, 10.2, 1, 1,  // 42
		-1.6, 1.5, 10.2, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 9.2, 0, 0, // 44
		-1.6, 0.5, 9.2, 1, 0,  // 45
		-1.6, 0.5, 10.2, 1, 1,  // 46
		-4.6, 0.5, 10.2, 0, 1, // 47

		//PETIKEMAS 19
		// front
		-4.6, 0.5, 11.3, 0, 0,  // 48
		-1.6, 0.5, 11.3, 1, 0,   // 25
		-1.6, 1.5, 11.3, 1, 1,   // 26
		-4.6, 1.5, 11.3, 0, 1,  // 27
		// right
		-1.6, 1.5, 11.3, 0, 0,  // 28
		-1.6, 1.5, 10.3, 1, 0,  // 29
		-1.6, 0.5, 10.3, 1, 1,  // 30
		-1.6, 0.5, 11.3, 0, 1,  // 31
		// back
		-4.6, 0.5, 10.3, 0, 0, // 32
		-1.6, 0.5, 10.3, 1, 0, // 33
		-1.6, 1.5, 10.3, 1, 1, // 34
		-4.6, 1.5, 10.3, 0, 1, // 35
		// left
		-4.6, 0.5, 10.3, 0, 0, // 36
		-4.6, 0.5, 11.3, 1, 0, // 37
		-4.6, 1.5, 11.3, 1, 1, // 38
		-4.6, 1.5, 10.3, 0, 1, // 39
		// upper
		-1.6, 1.5, 10.3, 0, 0,   // 40
		-4.6, 1.5, 10.3, 1, 0,  // 41
		-4.6, 1.5, 11.3, 1, 1,  // 42
		-1.6, 1.5, 11.3, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 10.3, 0, 0, // 44
		-1.6, 0.5, 10.3, 1, 0,  // 45
		-1.6, 0.5, 11.3, 1, 1,  // 46
		-4.6, 0.5, 11.3, 0, 1, // 47

		//PETIKEMAS 20
		// front
		-4.6, 0.5, 12.4, 0, 0,  // 48
		-1.6, 0.5, 12.4, 1, 0,   // 25
		-1.6, 1.5, 12.4, 1, 1,   // 26
		-4.6, 1.5, 12.4, 0, 1,  // 27
		// right
		-1.6, 1.5, 12.4, 0, 0,  // 28
		-1.6, 1.5, 11.4, 1, 0,  // 29
		-1.6, 0.5, 11.4, 1, 1,  // 30
		-1.6, 0.5, 12.4, 0, 1,  // 31
		// back
		-4.6, 0.5, 11.4, 0, 0, // 32
		-1.6, 0.5, 11.4, 1, 0, // 33
		-1.6, 1.5, 11.4, 1, 1, // 34
		-4.6, 1.5, 11.4, 0, 1, // 35
		// left
		-4.6, 0.5, 11.4, 0, 0, // 36
		-4.6, 0.5, 12.4, 1, 0, // 37
		-4.6, 1.5, 12.4, 1, 1, // 38
		-4.6, 1.5, 11.4, 0, 1, // 39
		// upper
		-1.6, 1.5, 11.4, 0, 0,   // 40
		-4.6, 1.5, 11.4, 1, 0,  // 41
		-4.6, 1.5, 12.4, 1, 1,  // 42
		-1.6, 1.5, 12.4, 0, 1,   // 43
		// bottom
		-4.6, 0.5, 11.4, 0, 0, // 44
		-1.6, 0.5, 11.4, 1, 0,  // 45
		-1.6, 0.5, 12.4, 1, 1,  // 46
		-4.6, 0.5, 12.4, 0, 1, // 47

	};

	unsigned int indices[] = {
		//PETIKEMAS 1
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,   // bottom

		//PETIKEMAS 2
		24,  25,  26,  24,  26,  27,   // front
		28,  29,  30,  28,  30,  31,   // right
		32,  33,  34,  32,  34,  35,  // back
		36,  38,  37,  36,  39,  38,  // left
		40,  42,  41,  40,  43,  42,  // upper
		44,  46,  45,  44,  47,  46,   // bottom

		//PETIKEMAS 3
		48,  49,  50,  48,  50,  51,   // front
		52,  53,  54,  52,  54,  55,   // right
		56,  57,  58,  56,  58,  59,  // back
		60,  62,  61,  60,  63,  62,  // left
		64,  66,  65,  64,  67,  66,  // upper
		68,  70,  69,  68,  71,  70,   // bottom

		//PETIKEMAS 4
		72,  73,  74,  72,  74,  75,   // front
		76,  77,  78,  76,  78,  79,   // right
		80,  81,  82,  80,  82,  83,  // back
		84,  86,  85,  84,  87,  86,  // left
		88,  90,  89,  88,  91,  90,  // upper
		92,  94,  93,  92,  95,  94,   // bottom

		//PETIKEMAS 5
		96,    97,   98,   96,   98,   99,   // front
		100,  101,  102,  100,  102,  103,   // right
		104,  105,  106,  104,  106,  107,  // back
		108,  110,  109,  108,  111,  110,  // left
		112,  114,  113,  112,  115,  114,  // upper
		116,  118,  117,  116,  119,  118,   // bottom

		//PETIKEMAS 6
		120,  121,  122,  120,  122,  123,   // front
		124,  125,  126,  124,  126,  127,   // right
		128,  129,  130,  128,  130,  131,  // back
		132,  134,  133,  132,  135,  134,  // left
		136,  138,  137,  136,  139,  138,  // upper
		140,  142,  141,  140,  143,  142,   // bottom

		//PETIKEMAS 7
		144,  145,  146,  144,  146,  147,   // front
		148,  149,  150,  148,  150,  151,   // right
		152,  153,  154,  152,  154,  155,  // back
		156,  158,  157,  156,  159,  158,  // left
		160,  162,  161,  160,  163,  162,  // upper
		164,  166,  165,  164,  167,  166,   // bottom

		//PETIKEMAS 8
		168,  169,  170,  168,  170,  171,   // front
		172,  173,  174,  172,  174,  175,   // right
		176,  177,  178,  176,  178,  179,  // back
		180,  182,  181,  180,  183,  182,  // left
		184,  186,  185,  184,  187,  186,  // upper
		188,  190,  189,  188,  191,  190,   // bottom

		//PETIKEMAS 9
		192,  193,  194,  192,  194,  195,   // front
		196,  197,  198,  196,  198,  199,   // right
		200,  201,  202,  200,  202,  203,  // back
		204,  206,  205,  204,  207,  206,  // left
		208,  210,  209,  208,  211,  210,  // upper
		212,  214,  213,  212,  215,  214,   // bottom

		//PETIKEMAS 10
		216,  217,  218,  216,  218,  219,   // front
		220,  221,  222,  220,  222,  223,   // right
		224,  225,  226,  224,  226,  227,  // back
		228,  230,  229,  228,  231,  230,  // left
		232,  234,  233,  232,  235,  234,  // upper
		236,  238,  237,  236,  239,  238,   // bottom

		//DIATAS KUNING
		//PETIKEMAS 11
		240,  241,  242,  240,  242,  243,   // front
		244,  245,  246,  244,  246,  247,   // right
		248,  249,  250,  248,  250,  251,  // back
		252,  254,  253,  252,  255,  254,  // left
		256,  258,  257,  256,  259,  258,  // upper
		260,  262,  261,  260,  263,  262,   // bottom

		//PETIKEMAS 12
		264,  265,  266,  264,  266,  267,   // front
		268,  269,  270,  268,  270,  271,   // right
		272,  273,  274,  272,  274,  275,  // back
		276,  278,  277,  276,  279,  278,  // left
		280,  282,  281,  280,  283,  282,  // upper
		284,  286,  285,  284,  287,  286,   // bottom

		//PETIKEMAS 13
		288,  289,  290,  288,  290,  291,   // front
		292,  293,  294,  292,  294,  295,   // right
		296,  297,  298,  296,  298,  299,  // back
		300,  302,  301,  300,  303,  302,  // left
		304,  306,  305,  304,  307,  306,  // upper
		308,  310,  309,  308,  311,  310,   // bottom

		//PETIKEMAS 14
		312,  313,  314,  312,  314,  315,   // front
		316,  317,  318,  316,  318,  319,   // right
		320,  321,  322,  320,  322,  323,  // back
		324,  326,  325,  324,  327,  326,  // left
		328,  330,  329,  328,  331,  330,  // upper
		332,  334,  333,  332,  335,  334,   // bottom

		//PETIKEMAS 15
		336,  337,  338,  336,  338,  339,   // front
		340,  341,  342,  340,  342,  343,   // right
		344,  345,  346,  344,  346,  347,  // back
		348,  350,  349,  348,  351,  350,  // left
		352,  354,  353,  352,  355,  354,  // upper
		356,  358,  357,  356,  359,  356,   // bottom

		//PETIKEMAS 16
		360,  361,  362,  360,  362,  363,   // front
		364,  365,  366,  364,  366,  367,   // right
		368,  369,  370,  368,  370,  371,  // back
		372,  374,  373,  372,  375,  374,  // left
		376,  378,  377,  376,  379,  378,  // upper
		380,  382,  381,  380,  383,  382,   // bottom

		//PETIKEMAS 17
		384,  385,  386,  384,  386,  387,   // front
		388,  389,  390,  388,  390,  391,   // right
		392,  393,  394,  392,  394,  395,  // back
		396,  398,  397,  396,  399,  398,  // left
		400,  402,  401,  400,  403,  402,  // upper
		404,  406,  405,  404,  407,  406,   // bottom

		//PETIKEMAS 18
		408,  409,  410,  408,  410,  411,   // front
		412,  413,  414,  412,  414,  415,   // right
		416,  417,  418,  416,  418,  419,  // back
		420,  422,  421,  420,  423,  422,  // left
		424,  426,  425,  424,  427,  426,  // upper
		428,  430,  429,  428,  431,  430,   // bottom

		//PETIKEMAS 19
		432,  433,  434,  432,  434,  435,   // front
		436,  437,  438,  436,  438,  439,   // right
		440,  441,  442,  440,  442,  443,  // back
		444,  446,  445,  444,  447,  446,  // left
		448,  450,  449,  448,  451,  450,  // upper
		452,  454,  453,  452,  455,  454,   // bottom

		//PETIKEMAS 20
		456,  457,  458,  456,  458,  459,   // front
		460,  461,  462,  460,  462,  463,   // right
		464,  465,  466,  464,  466,  467,  // back
		468,  470,  469,  468,  471,  470,  // left
		472,  474,  473,  472,  475,  474,  // upper
		476,  478,  477,  476,  479,  478,   // bottom
	};


	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::DrawColoredCubeOrange(GLuint shader)
{
	//glUseProgram(shaderProgram);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "model"), 0);

	UseShader(shader);
	glBindVertexArray(VAO2);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO2); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 1500, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::ObjekPetikemasYellow() {
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("petikemas1.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	float vertices[] = {
		//PETIKEMAS 1
		// front
		-4.6, -0.5, 0.5, 0, 0,  // 0
		-1.6, -0.5, 0.5, 1, 0,   // 1
		-1.6, 0.5, 0.5, 1, 1,   // 2
		-4.6, 0.5, 0.5, 0, 1,  // 3
		// right
		-1.6, 0.5,  0.5, 0, 0,  // 4
		-1.6, 0.5, -0.5, 1, 0,  // 5
		-1.6, -0.5, -0.5, 1, 1,  // 6
		-1.6, -0.5,  0.5, 0, 1,  // 7
		// back
		-4.6, -0.5, -0.5, 0, 0, // 8 
		-1.6, -0.5, -0.5, 1, 0, // 9
		-1.6, 0.5, -0.5, 1, 1, // 10
		-4.6, 0.5, -0.5, 0, 1, // 11
		// left
		-4.6, -0.5, -0.5, 0, 0, // 12
		-4.6, -0.5, 0.5, 1, 0, // 13
		-4.6, 0.5, 0.5, 1, 1, // 14
		-4.6, 0.5, -0.5, 0, 1, // 15
		// upper
		-1.6, 0.5, 0.5, 0, 0,   // 16
		-4.6, 0.5, 0.5, 1, 0,  // 17
		-4.6, 0.5, -0.5, 1, 1,  // 18
		-1.6, 0.5, -0.5, 0, 1,   // 19
		// bottom
		-4.6, -0.5, -0.5, 0, 0, // 20
		-1.6, -0.5, -0.5, 1, 0,  // 21
		-1.6, -0.5, 0.5, 1, 1,  // 22
		-4.6, -0.5, 0.5, 0, 1, // 23

		//PETIKEMAS 2
		// front
		-4.6, -0.5, 1.6, 0, 0,  // 24
		-1.6, -0.5, 1.6, 1, 0,   // 25
		-1.6, 0.5, 1.6, 1, 1,   // 26
		-4.6, 0.5, 1.6, 0, 1,  // 27
		// right
		-1.6, 0.5,  1.6, 0, 0,  // 28
		-1.6, 0.5, 0.6, 1, 0,  // 29
		-1.6, -0.5, 0.6, 1, 1,  // 30
		-1.6, -0.5,  1.6, 0, 1,  // 31
		// back
		-4.6, -0.5, 0.6, 0, 0, // 32
		-1.6, -0.5, 0.6, 1, 0, // 33
		-1.6, 0.5, 0.6, 1, 1, // 34
		-4.6, 0.5, 0.6, 0, 1, // 35
		// left
		-4.6, -0.5, 0.6, 0, 0, // 36
		-4.6, -0.5, 1.6, 1, 0, // 37
		-4.6, 0.5, 1.6, 1, 1, // 38
		-4.6, 0.5, 0.6, 0, 1, // 39
		// upper
		-1.6, 0.5, 1.6, 0, 0,   // 40
		-4.6, 0.5, 1.6, 1, 0,  // 41
		-4.6, 0.5, 0.6, 1, 1,  // 42
		-1.6, 0.5, 0.6, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 0.6, 0, 0, // 44
		-1.6, -0.5, 0.6, 1, 0,  // 45
		-1.6, -0.5, 1.6, 1, 1,  // 46
		-4.6, -0.5, 1.6, 0, 1, // 47

		//PETIKEMAS 3
		// front
		-4.6, -0.5, 2.7, 0, 0,  // 48
		-1.6, -0.5, 2.7, 1, 0,   // 25
		-1.6, 0.5, 2.7, 1, 1,   // 26
		-4.6, 0.5, 2.7, 0, 1,  // 27
		// right
		-1.6, 0.5,  2.7, 0, 0,  // 28
		-1.6, 0.5, 1.7, 1, 0,  // 29
		-1.6, -0.5, 1.7, 1, 1,  // 30
		-1.6, -0.5,  2.7, 0, 1,  // 31
		// back
		-4.6, -0.5, 1.7, 0, 0, // 32
		-1.6, -0.5, 1.7, 1, 0, // 33
		-1.6, 0.5, 1.7, 1, 1, // 34
		-4.6, 0.5, 1.7, 0, 1, // 35
		// left
		-4.6, -0.5, 1.7, 0, 0, // 36
		-4.6, -0.5, 2.7, 1, 0, // 37
		-4.6, 0.5, 2.7, 1, 1, // 38
		-4.6, 0.5, 1.7, 0, 1, // 39
		// upper
		-1.6, 0.5, 2.7, 0, 0,   // 40
		-4.6, 0.5, 2.7, 1, 0,  // 41
		-4.6, 0.5, 1.7, 1, 1,  // 42
		-1.6, 0.5, 1.7, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 1.7, 0, 0, // 44
		-1.6, -0.5, 1.7, 1, 0,  // 45
		-1.6, -0.5, 2.7, 1, 1,  // 46
		-4.6, -0.5, 2.7, 0, 1, // 47

		//PETIKEMAS 4
		// front
		-4.6, -0.5, 3.8, 0, 0,  // 48
		-1.6, -0.5, 3.8, 1, 0,   // 25
		-1.6, 0.5, 3.8, 1, 1,   // 26
		-4.6, 0.5, 3.8, 0, 1,  // 27
		// right
		-1.6, 0.5, 3.8, 0, 0,  // 28
		-1.6, 0.5, 2.8, 1, 0,  // 29
		-1.6, -0.5, 2.8, 1, 1,  // 30
		-1.6, -0.5, 3.8, 0, 1,  // 31
		// back
		-4.6, -0.5, 2.8, 0, 0, // 32
		-1.6, -0.5, 2.8, 1, 0, // 33
		-1.6, 0.5, 2.8, 1, 1, // 34
		-4.6, 0.5, 2.8, 0, 1, // 35
		// left
		-4.6, -0.5, 2.8, 0, 0, // 36
		-4.6, -0.5, 3.8, 1, 0, // 37
		-4.6, 0.5, 3.8, 1, 1, // 38
		-4.6, 0.5, 2.8, 0, 1, // 39
		// upper
		-1.6, 0.5, 3.8, 0, 0,   // 40
		-4.6, 0.5, 3.8, 1, 0,  // 41
		-4.6, 0.5, 2.8, 1, 1,  // 42
		-1.6, 0.5, 2.8, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 2.8, 0, 0, // 44
		-1.6, -0.5, 2.8, 1, 0,  // 45
		-1.6, -0.5, 3.8, 1, 1,  // 46
		-4.6, -0.5, 3.8, 0, 1, // 47

		//PETIKEMAS 5
		// front
		-4.6, -0.5, 4.9, 0, 0,  // 48
		-1.6, -0.5, 4.9, 1, 0,   // 25
		-1.6, 0.5, 4.9, 1, 1,   // 26
		-4.6, 0.5, 4.9, 0, 1,  // 27
		// right
		-1.6, 0.5, 4.9, 0, 0,  // 28
		-1.6, 0.5, 3.9, 1, 0,  // 29
		-1.6, -0.5, 3.9, 1, 1,  // 30
		-1.6, -0.5, 4.9, 0, 1,  // 31
		// back
		-4.6, -0.5, 3.9, 0, 0, // 32
		-1.6, -0.5, 3.9, 1, 0, // 33
		-1.6, 0.5, 3.9, 1, 1, // 34
		-4.6, 0.5, 3.9, 0, 1, // 35
		// left
		-4.6, -0.5, 3.9, 0, 0, // 36
		-4.6, -0.5, 4.9, 1, 0, // 37
		-4.6, 0.5, 4.9, 1, 1, // 38
		-4.6, 0.5, 3.9, 0, 1, // 39
		// upper
		-1.6, 0.5, 3.9, 0, 0,   // 40
		-4.6, 0.5, 3.9, 1, 0,  // 41
		-4.6, 0.5, 4.9, 1, 1,  // 42
		-1.6, 0.5, 4.9, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 3.9, 0, 0, // 44
		-1.6, -0.5, 3.9, 1, 0,  // 45
		-1.6, -0.5, 4.9, 1, 1,  // 46
		-4.6, -0.5, 4.9, 0, 1, // 47

		//PETIKEMAS 6
		// front
		-4.6, -0.5, 8.0, 0, 0,  // 48
		-1.6, -0.5, 8.0, 1, 0,   // 25
		-1.6, 0.5, 8.0, 1, 1,   // 26
		-4.6, 0.5, 8.0, 0, 1,  // 27
		// right
		-1.6, 0.5, 8.0, 0, 0,  // 28
		-1.6, 0.5, 7.0, 1, 0,  // 29
		-1.6, -0.5, 7.0, 1, 1,  // 30
		-1.6, -0.5, 8.0, 0, 1,  // 31
		// back
		-4.6, -0.5, 7.0, 0, 0, // 32
		-1.6, -0.5, 7.0, 1, 0, // 33
		-1.6, 0.5, 7.0, 1, 1, // 34
		-4.6, 0.5, 7.0, 0, 1, // 35
		// left
		-4.6, -0.5, 7.0, 0, 0, // 36
		-4.6, -0.5, 8.0, 1, 0, // 37
		-4.6, 0.5, 8.0, 1, 1, // 38
		-4.6, 0.5, 7.0, 0, 1, // 39
		// upper
		-1.6, 0.5, 7.0, 0, 0,   // 40
		-4.6, 0.5, 7.0, 1, 0,  // 41
		-4.6, 0.5, 8.0, 1, 1,  // 42
		-1.6, 0.5, 8.0, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 7.0, 0, 0, // 44
		-1.6, -0.5, 7.0, 1, 0,  // 45
		-1.6, -0.5, 8.0, 1, 1,  // 46
		-4.6, -0.5, 8.0, 0, 1, // 47

		//PETIKEMAS 7
		// front
		-4.6, -0.5, 9.1, 0, 0,  // 48
		-1.6, -0.5, 9.1, 1, 0,   // 25
		-1.6, 0.5, 9.1, 1, 1,   // 26
		-4.6, 0.5, 9.1, 0, 1,  // 27
		// right
		-1.6, 0.5, 9.1, 0, 0,  // 28
		-1.6, 0.5, 8.1, 1, 0,  // 29
		-1.6, -0.5, 8.1, 1, 1,  // 30
		-1.6, -0.5, 9.1, 0, 1,  // 31
		// back
		-4.6, -0.5, 8.1, 0, 0, // 32
		-1.6, -0.5, 8.1, 1, 0, // 33
		-1.6, 0.5, 8.1, 1, 1, // 34
		-4.6, 0.5, 8.1, 0, 1, // 35
		// left
		-4.6, -0.5, 8.1, 0, 0, // 36
		-4.6, -0.5, 9.1, 1, 0, // 37
		-4.6, 0.5, 9.1, 1, 1, // 38
		-4.6, 0.5, 8.1, 0, 1, // 39
		// upper
		-1.6, 0.5, 8.1, 0, 0,   // 40
		-4.6, 0.5, 8.1, 1, 0,  // 41
		-4.6, 0.5, 9.1, 1, 1,  // 42
		-1.6, 0.5, 9.1, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 8.1, 0, 0, // 44
		-1.6, -0.5, 8.1, 1, 0,  // 45
		-1.6, -0.5, 9.1, 1, 1,  // 46
		-4.6, -0.5, 9.1, 0, 1, // 47

		//PETIKEMAS 8
		// front
		-4.6, -0.5, 10.2, 0, 0,  // 48
		-1.6, -0.5, 10.2, 1, 0,   // 25
		-1.6, 0.5, 10.2, 1, 1,   // 26
		-4.6, 0.5, 10.2, 0, 1,  // 27
		// right
		-1.6, 0.5, 10.2, 0, 0,  // 28
		-1.6, 0.5, 9.2, 1, 0,  // 29
		-1.6, -0.5, 9.2, 1, 1,  // 30
		-1.6, -0.5, 10.2, 0, 1,  // 31
		// back
		-4.6, -0.5, 9.2, 0, 0, // 32
		-1.6, -0.5, 9.2, 1, 0, // 33
		-1.6, 0.5, 9.2, 1, 1, // 34
		-4.6, 0.5, 9.2, 0, 1, // 35
		// left
		-4.6, -0.5, 9.2, 0, 0, // 36
		-4.6, -0.5, 10.2, 1, 0, // 37
		-4.6, 0.5, 10.2, 1, 1, // 38
		-4.6, 0.5, 9.2, 0, 1, // 39
		// upper
		-1.6, 0.5, 9.2, 0, 0,   // 40
		-4.6, 0.5, 9.2, 1, 0,  // 41
		-4.6, 0.5, 10.2, 1, 1,  // 42
		-1.6, 0.5, 10.2, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 9.2, 0, 0, // 44
		-1.6, -0.5, 9.2, 1, 0,  // 45
		-1.6, -0.5, 10.2, 1, 1,  // 46
		-4.6, -0.5, 10.2, 0, 1, // 47

		//PETIKEMAS 9
		// front
		-4.6, -0.5, 11.3, 0, 0,  // 48
		-1.6, -0.5, 11.3, 1, 0,   // 25
		-1.6, 0.5, 11.3, 1, 1,   // 26
		-4.6, 0.5, 11.3, 0, 1,  // 27
		// right
		-1.6, 0.5, 11.3, 0, 0,  // 28
		-1.6, 0.5, 10.3, 1, 0,  // 29
		-1.6, -0.5, 10.3, 1, 1,  // 30
		-1.6, -0.5, 11.3, 0, 1,  // 31
		// back
		-4.6, -0.5, 10.3, 0, 0, // 32
		-1.6, -0.5, 10.3, 1, 0, // 33
		-1.6, 0.5, 10.3, 1, 1, // 34
		-4.6, 0.5, 10.3, 0, 1, // 35
		// left
		-4.6, -0.5, 10.3, 0, 0, // 36
		-4.6, -0.5, 11.3, 1, 0, // 37
		-4.6, 0.5, 11.3, 1, 1, // 38
		-4.6, 0.5, 10.3, 0, 1, // 39
		// upper
		-1.6, 0.5, 10.3, 0, 0,   // 40
		-4.6, 0.5, 10.3, 1, 0,  // 41
		-4.6, 0.5, 11.3, 1, 1,  // 42
		-1.6, 0.5, 11.3, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 10.3, 0, 0, // 44
		-1.6, -0.5, 10.3, 1, 0,  // 45
		-1.6, -0.5, 11.3, 1, 1,  // 46
		-4.6, -0.5, 11.3, 0, 1, // 47

		//PETIKEMAS 10
		// front
		-4.6, -0.5, 12.4, 0, 0,  // 48
		-1.6, -0.5, 12.4, 1, 0,   // 25
		-1.6, 0.5, 12.4, 1, 1,   // 26
		-4.6, 0.5, 12.4, 0, 1,  // 27
		// right
		-1.6, 0.5, 12.4, 0, 0,  // 28
		-1.6, 0.5, 11.4, 1, 0,  // 29
		-1.6, -0.5, 11.4, 1, 1,  // 30
		-1.6, -0.5, 12.4, 0, 1,  // 31
		// back
		-4.6, -0.5, 11.4, 0, 0, // 32
		-1.6, -0.5, 11.4, 1, 0, // 33
		-1.6, 0.5, 11.4, 1, 1, // 34
		-4.6, 0.5, 11.4, 0, 1, // 35
		// left
		-4.6, -0.5, 11.4, 0, 0, // 36
		-4.6, -0.5, 12.4, 1, 0, // 37
		-4.6, 0.5, 12.4, 1, 1, // 38
		-4.6, 0.5, 11.4, 0, 1, // 39
		// upper
		-1.6, 0.5, 11.4, 0, 0,   // 40
		-4.6, 0.5, 11.4, 1, 0,  // 41
		-4.6, 0.5, 12.4, 1, 1,  // 42
		-1.6, 0.5, 12.4, 0, 1,   // 43
		// bottom
		-4.6, -0.5, 11.4, 0, 0, // 44
		-1.6, -0.5, 11.4, 1, 0,  // 45
		-1.6, -0.5, 12.4, 1, 1,  // 46
		-4.6, -0.5, 12.4, 0, 1, // 47

		//DIATAS MERAH
		//PETIKEMAS 11
		// front
		4.5, 0.5, 0.5, 0, 0,  // 0
		7.5, 0.5, 0.5, 1, 0,   // 1
		7.5, 1.5, 0.5, 1, 1,   // 2
		4.5, 1.5, 0.5, 0, 1,  // 3
		// right
		7.5, 1.5,  0.5, 0, 0,  // 4
		7.5, 1.5, -0.5, 1, 0,  // 5
		7.5, 0.5, -0.5, 1, 1,  // 6
		7.5, 0.5,  0.5, 0, 1,  // 7
		// back
		4.5, 0.5, -0.5, 0, 0, // 8 
		7.5, 0.5, -0.5, 1, 0, // 9
		7.5, 1.5, -0.5, 1, 1, // 10
		4.5, 1.5, -0.5, 0, 1, // 11
		// left
		4.5, 0.5, -0.5, 0, 0, // 12
		4.5, 0.5, 0.5, 1, 0, // 13
		4.5, 1.5, 0.5, 1, 1, // 14
		4.5, 1.5, -0.5, 0, 1, // 15
		// upper
		7.5, 1.5, 0.5, 0, 0,   // 16
		4.5, 1.5, 0.5, 1, 0,  // 17
		4.5, 1.5, -0.5, 1, 1,  // 18
		7.5, 1.5, -0.5, 0, 1,   // 19
		// bottom
		4.5, 0.5, -0.5, 0, 0, // 20
		7.5, 0.5, -0.5, 1, 0,  // 21
		7.5, 0.5, 0.5, 1, 1,  // 22
		4.5, 0.5, 0.5, 0, 1, // 23

		//PETIKEMAS 12
		// front
		4.5, 0.5, 1.6, 0, 0,  // 24
		7.5, 0.5, 1.6, 1, 0,   // 25
		7.5, 1.5, 1.6, 1, 1,   // 26
		4.5, 1.5, 1.6, 0, 1,  // 27
		// right
		7.5, 1.5,  1.6, 0, 0,  // 28
		7.5, 1.5, 0.6, 1, 0,  // 29
		7.5, 0.5, 0.6, 1, 1,  // 30
		7.5, 0.5,  1.6, 0, 1,  // 31
		// back
		4.5, 0.5, 0.6, 0, 0, // 32
		7.5, 0.5, 0.6, 1, 0, // 33
		7.5, 1.5, 0.6, 1, 1, // 34
		4.5, 1.5, 0.6, 0, 1, // 35
		// left
		4.5, 0.5, 0.6, 0, 0, // 36
		4.5, 0.5, 1.6, 1, 0, // 37
		4.5, 1.5, 1.6, 1, 1, // 38
		4.5, 1.5, 0.6, 0, 1, // 39
		// upper
		7.5, 1.5, 1.6, 0, 0,   // 40
		4.5, 1.5, 1.6, 1, 0,  // 41
		4.5, 1.5, 0.6, 1, 1,  // 42
		7.5, 1.5, 0.6, 0, 1,   // 43
		// bottom
		4.5, 0.5, 0.6, 0, 0, // 44
		7.5, 0.5, 0.6, 1, 0,  // 45
		7.5, 0.5, 1.6, 1, 1,  // 46
		4.5, 0.5, 1.6, 0, 1, // 47

		//PETIKEMAS 13
		// front
		4.5, 0.5, 2.7, 0, 0,  // 48
		7.5, 0.5, 2.7, 1, 0,   // 25
		7.5, 1.5, 2.7, 1, 1,   // 26
		4.5, 1.5, 2.7, 0, 1,  // 27
		// right
		7.5, 1.5,  2.7, 0, 0,  // 28
		7.5, 1.5, 1.7, 1, 0,  // 29
		7.5, 0.5, 1.7, 1, 1,  // 30
		7.5, 0.5,  2.7, 0, 1,  // 31
		// back
		4.5, 0.5, 1.7, 0, 0, // 32
		7.5, 0.5, 1.7, 1, 0, // 33
		7.5, 1.5, 1.7, 1, 1, // 34
		4.5, 1.5, 1.7, 0, 1, // 35
		// left
		4.5, 0.5, 1.7, 0, 0, // 36
		4.5, 0.5, 2.7, 1, 0, // 37
		4.5, 1.5, 2.7, 1, 1, // 38
		4.5, 1.5, 1.7, 0, 1, // 39
		// upper
		7.5, 1.5, 2.7, 0, 0,   // 40
		4.5, 1.5, 2.7, 1, 0,  // 41
		4.5, 1.5, 1.7, 1, 1,  // 42
		7.5, 1.5, 1.7, 0, 1,   // 43
		// bottom
		4.5, 0.5, 1.7, 0, 0, // 44
		7.5, 0.5, 1.7, 1, 0,  // 45
		7.5, 0.5, 2.7, 1, 1,  // 46
		4.5, 0.5, 2.7, 0, 1, // 47

		//PETIKEMAS 14
		// front
		4.5, 0.5, 3.8, 0, 0,  // 48
		7.5, 0.5, 3.8, 1, 0,   // 25
		7.5, 1.5, 3.8, 1, 1,   // 26
		4.5, 1.5, 3.8, 0, 1,  // 27
		// right
		7.5, 1.5, 3.8, 0, 0,  // 28
		7.5, 1.5, 2.8, 1, 0,  // 29
		7.5, 0.5, 2.8, 1, 1,  // 30
		7.5, 0.5, 3.8, 0, 1,  // 31
		// back
		4.5, 0.5, 2.8, 0, 0, // 32
		7.5, 0.5, 2.8, 1, 0, // 33
		7.5, 1.5, 2.8, 1, 1, // 34
		4.5, 1.5, 2.8, 0, 1, // 35
		// left
		4.5, 0.5, 2.8, 0, 0, // 36
		4.5, 0.5, 3.8, 1, 0, // 37
		4.5, 1.5, 3.8, 1, 1, // 38
		4.5, 1.5, 2.8, 0, 1, // 39
		// upper
		7.5, 1.5, 3.8, 0, 0,   // 40
		4.5, 1.5, 3.8, 1, 0,  // 41
		4.5, 1.5, 2.8, 1, 1,  // 42
		7.5, 1.5, 2.8, 0, 1,   // 43
		// bottom
		4.5, 0.5, 2.8, 0, 0, // 44
		7.5, 0.5, 2.8, 1, 0,  // 45
		7.5, 0.5, 3.8, 1, 1,  // 46
		4.5, 0.5, 3.8, 0, 1, // 47

		//PETIKEMAS 15
		// front
		4.5, 0.5, 4.9, 0, 0,  // 48
		7.5, 0.5, 4.9, 1, 0,   // 25
		7.5, 1.5, 4.9, 1, 1,   // 26
		4.5, 1.5, 4.9, 0, 1,  // 27
		// right
		7.5, 1.5, 4.9, 0, 0,  // 28
		7.5, 1.5, 3.9, 1, 0,  // 29
		7.5, 0.5, 3.9, 1, 1,  // 30
		7.5, 0.5, 4.9, 0, 1,  // 31
		// back
		4.5, 0.5, 3.9, 0, 0, // 32
		7.5, 0.5, 3.9, 1, 0, // 33
		7.5, 1.5, 3.9, 1, 1, // 34
		4.5, 1.5, 3.9, 0, 1, // 35
		// left
		4.5, 0.5, 3.9, 0, 0, // 36
		4.5, 0.5, 4.9, 1, 0, // 37
		4.5, 1.5, 4.9, 1, 1, // 38
		4.5, 1.5, 3.9, 0, 1, // 39
		// upper
		7.5, 1.5, 3.9, 0, 0,   // 40
		4.5, 1.5, 3.9, 1, 0,  // 41
		4.5, 1.5, 4.9, 1, 1,  // 42
		7.5, 1.5, 4.9, 0, 1,   // 43
		// bottom
		4.5, 0.5, 3.9, 0, 0, // 44
		7.5, 0.5, 3.9, 1, 0,  // 45
		7.5, 0.5, 4.9, 1, 1,  // 46
		4.5, 0.5, 4.9, 0, 1, // 47

		//PETIKEMAS 16
		// front
		4.5, 0.5, 8.0, 0, 0,  // 48
		7.5, 0.5, 8.0, 1, 0,   // 25
		7.5, 1.5, 8.0, 1, 1,   // 26
		4.5, 1.5, 8.0, 0, 1,  // 27
		// right
		7.5, 1.5, 8.0, 0, 0,  // 28
		7.5, 1.5, 7.0, 1, 0,  // 29
		7.5, 0.5, 7.0, 1, 1,  // 30
		7.5, 0.5, 8.0, 0, 1,  // 31
		// back
		4.5, 0.5, 7.0, 0, 0, // 32
		7.5, 0.5, 7.0, 1, 0, // 33
		7.5, 1.5, 7.0, 1, 1, // 34
		4.5, 1.5, 7.0, 0, 1, // 35
		// left
		4.5, 0.5, 7.0, 0, 0, // 36
		4.5, 0.5, 8.0, 1, 0, // 37
		4.5, 1.5, 8.0, 1, 1, // 38
		4.5, 1.5, 7.0, 0, 1, // 39
		// upper
		7.5, 1.5, 7.0, 0, 0,   // 40
		4.5, 1.5, 7.0, 1, 0,  // 41
		4.5, 1.5, 8.0, 1, 1,  // 42
		7.5, 1.5, 8.0, 0, 1,   // 43
		// bottom
		4.5, 0.5, 7.0, 0, 0, // 44
		7.5, 0.5, 7.0, 1, 0,  // 45
		7.5, 0.5, 8.0, 1, 1,  // 46
		4.5, 0.5, 8.0, 0, 1, // 47

		//PETIKEMAS 17
		// front
		4.5, 0.5, 9.1, 0, 0,  // 48
		7.5, 0.5, 9.1, 1, 0,   // 25
		7.5, 1.5, 9.1, 1, 1,   // 26
		4.5, 1.5, 9.1, 0, 1,  // 27
		// right
		7.5, 1.5, 9.1, 0, 0,  // 28
		7.5, 1.5, 8.1, 1, 0,  // 29
		7.5, 0.5, 8.1, 1, 1,  // 30
		7.5, 0.5, 9.1, 0, 1,  // 31
		// back
		4.5, 0.5, 8.1, 0, 0, // 32
		7.5, 0.5, 8.1, 1, 0, // 33
		7.5, 1.5, 8.1, 1, 1, // 34
		4.5, 1.5, 8.1, 0, 1, // 35
		// left
		4.5, 0.5, 8.1, 0, 0, // 36
		4.5, 0.5, 9.1, 1, 0, // 37
		4.5, 1.5, 9.1, 1, 1, // 38
		4.5, 1.5, 8.1, 0, 1, // 39
		// upper
		7.5, 1.5, 8.1, 0, 0,   // 40
		4.5, 1.5, 8.1, 1, 0,  // 41
		4.5, 1.5, 9.1, 1, 1,  // 42
		7.5, 1.5, 9.1, 0, 1,   // 43
		// bottom
		4.5, 0.5, 8.1, 0, 0, // 44
		7.5, 0.5, 8.1, 1, 0,  // 45
		7.5, 0.5, 9.1, 1, 1,  // 46
		4.5, 0.5, 9.1, 0, 1, // 47

		//PETIKEMAS 18
		// front
		4.5, 0.5, 10.2, 0, 0,  // 48
		7.5, 0.5, 10.2, 1, 0,   // 25
		7.5, 1.5, 10.2, 1, 1,   // 26
		4.5, 1.5, 10.2, 0, 1,  // 27
		// right
		7.5, 1.5, 10.2, 0, 0,  // 28
		7.5, 1.5, 9.2, 1, 0,  // 29
		7.5, 0.5, 9.2, 1, 1,  // 30
		7.5, 0.5, 10.2, 0, 1,  // 31
		// back
		4.5, 0.5, 9.2, 0, 0, // 32
		7.5, 0.5, 9.2, 1, 0, // 33
		7.5, 1.5, 9.2, 1, 1, // 34
		4.5, 1.5, 9.2, 0, 1, // 35
		// left
		4.5, 0.5, 9.2, 0, 0, // 36
		4.5, 0.5, 10.2, 1, 0, // 37
		4.5, 1.5, 10.2, 1, 1, // 38
		4.5, 1.5, 9.2, 0, 1, // 39
		// upper
		7.5, 1.5, 9.2, 0, 0,   // 40
		4.5, 1.5, 9.2, 1, 0,  // 41
		4.5, 1.5, 10.2, 1, 1,  // 42
		7.5, 1.5, 10.2, 0, 1,   // 43
		// bottom
		4.5, 0.5, 9.2, 0, 0, // 44
		7.5, 0.5, 9.2, 1, 0,  // 45
		7.5, 0.5, 10.2, 1, 1,  // 46
		4.5, 0.5, 10.2, 0, 1, // 47

		//PETIKEMAS 19
		// front
		4.5, 0.5, 11.3, 0, 0,  // 48
		7.5, 0.5, 11.3, 1, 0,   // 25
		7.5, 1.5, 11.3, 1, 1,   // 26
		4.5, 1.5, 11.3, 0, 1,  // 27
		// right
		7.5, 1.5, 11.3, 0, 0,  // 28
		7.5, 1.5, 10.3, 1, 0,  // 29
		7.5, 0.5, 10.3, 1, 1,  // 30
		7.5, 0.5, 11.3, 0, 1,  // 31
		// back
		4.5, 0.5, 10.3, 0, 0, // 32
		7.5, 0.5, 10.3, 1, 0, // 33
		7.5, 1.5, 10.3, 1, 1, // 34
		4.5, 1.5, 10.3, 0, 1, // 35
		// left
		4.5, 0.5, 10.3, 0, 0, // 36
		4.5, 0.5, 11.3, 1, 0, // 37
		4.5, 1.5, 11.3, 1, 1, // 38
		4.5, 1.5, 10.3, 0, 1, // 39
		// upper
		7.5, 1.5, 10.3, 0, 0,   // 40
		4.5, 1.5, 10.3, 1, 0,  // 41
		4.5, 1.5, 11.3, 1, 1,  // 42
		7.5, 1.5, 11.3, 0, 1,   // 43
		// bottom
		4.5, 0.5, 10.3, 0, 0, // 44
		7.5, 0.5, 10.3, 1, 0,  // 45
		7.5, 0.5, 11.3, 1, 1,  // 46
		4.5, 0.5, 11.3, 0, 1, // 47

		//PETIKEMAS 20
		// front
		4.5, 0.5, 12.4, 0, 0,  // 48
		7.5, 0.5, 12.4, 1, 0,   // 25
		7.5, 1.5, 12.4, 1, 1,   // 26
		4.5, 1.5, 12.4, 0, 1,  // 27
		// right
		7.5, 1.5, 12.4, 0, 0,  // 28
		7.5, 1.5, 11.4, 1, 0,  // 29
		7.5, 0.5, 11.4, 1, 1,  // 30
		7.5, 0.5, 12.4, 0, 1,  // 31
		// back
		4.5, 0.5, 11.4, 0, 0, // 32
		7.5, 0.5, 11.4, 1, 0, // 33
		7.5, 1.5, 11.4, 1, 1, // 34
		4.5, 1.5, 11.4, 0, 1, // 35
		// left
		4.5, 0.5, 11.4, 0, 0, // 36
		4.5, 0.5, 12.4, 1, 0, // 37
		4.5, 1.5, 12.4, 1, 1, // 38
		4.5, 1.5, 11.4, 0, 1, // 39
		// upper
		7.5, 1.5, 11.4, 0, 0,   // 40
		4.5, 1.5, 11.4, 1, 0,  // 41
		4.5, 1.5, 12.4, 1, 1,  // 42
		7.5, 1.5, 12.4, 0, 1,   // 43
		// bottom
		4.5, 0.5, 11.4, 0, 0, // 44
		7.5, 0.5, 11.4, 1, 0,  // 45
		7.5, 0.5, 12.4, 1, 1,  // 46
		4.5, 0.5, 12.4, 0, 1, // 47
	};

	unsigned int indices[] = {
		//PETIKEMAS 1
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,   // bottom

		//PETIKEMAS 2
		24,  25,  26,  24,  26,  27,   // front
		28,  29,  30,  28,  30,  31,   // right
		32,  33,  34,  32,  34,  35,  // back
		36,  38,  37,  36,  39,  38,  // left
		40,  42,  41,  40,  43,  42,  // upper
		44,  46,  45,  44,  47,  46,   // bottom

		//PETIKEMAS 3
		48,  49,  50,  48,  50,  51,   // front
		52,  53,  54,  52,  54,  55,   // right
		56,  57,  58,  56,  58,  59,  // back
		60,  62,  61,  60,  63,  62,  // left
		64,  66,  65,  64,  67,  66,  // upper
		68,  70,  69,  68,  71,  70,   // bottom

		//PETIKEMAS 4
		72,  73,  74,  72,  74,  75,   // front
		76,  77,  78,  76,  78,  79,   // right
		80,  81,  82,  80,  82,  83,  // back
		84,  86,  85,  84,  87,  86,  // left
		88,  90,  89,  88,  91,  90,  // upper
		92,  94,  93,  92,  95,  94,   // bottom

		//PETIKEMAS 5
		96,    97,   98,   96,   98,   99,   // front
		100,  101,  102,  100,  102,  103,   // right
		104,  105,  106,  104,  106,  107,  // back
		108,  110,  109,  108,  111,  110,  // left
		112,  114,  113,  112,  115,  114,  // upper
		116,  118,  117,  116,  119,  118,   // bottom

		//PETIKEMAS 6
		120,  121,  122,  120,  122,  123,   // front
		124,  125,  126,  124,  126,  127,   // right
		128,  129,  130,  128,  130,  131,  // back
		132,  134,  133,  132,  135,  134,  // left
		136,  138,  137,  136,  139,  138,  // upper
		140,  142,  141,  140,  143,  142,   // bottom

		//PETIKEMAS 7
		144,  145,  146,  144,  146,  147,   // front
		148,  149,  150,  148,  150,  151,   // right
		152,  153,  154,  152,  154,  155,  // back
		156,  158,  157,  156,  159,  158,  // left
		160,  162,  161,  160,  163,  162,  // upper
		164,  166,  165,  164,  167,  166,   // bottom

		//PETIKEMAS 8
		168,  169,  170,  168,  170,  171,   // front
		172,  173,  174,  172,  174,  175,   // right
		176,  177,  178,  176,  178,  179,  // back
		180,  182,  181,  180,  183,  182,  // left
		184,  186,  185,  184,  187,  186,  // upper
		188,  190,  189,  188,  191,  190,   // bottom

		//PETIKEMAS 9
		192,  193,  194,  192,  194,  195,   // front
		196,  197,  198,  196,  198,  199,   // right
		200,  201,  202,  200,  202,  203,  // back
		204,  206,  205,  204,  207,  206,  // left
		208,  210,  209,  208,  211,  210,  // upper
		212,  214,  213,  212,  215,  214,   // bottom

		//PETIKEMAS 10
		216,  217,  218,  216,  218,  219,   // front
		220,  221,  222,  220,  222,  223,   // right
		224,  225,  226,  224,  226,  227,  // back
		228,  230,  229,  228,  231,  230,  // left
		232,  234,  233,  232,  235,  234,  // upper
		236,  238,  237,  236,  239,  238,   // bottom

		//DIATAS MERAH
				//PETIKEMAS 11
		240,  241,  242,  240,  242,  243,   // front
		244,  245,  246,  244,  246,  247,   // right
		248,  249,  250,  248,  250,  251,  // back
		252,  254,  253,  252,  255,  254,  // left
		256,  258,  257,  256,  259,  258,  // upper
		260,  262,  261,  260,  263,  262,   // bottom

		//PETIKEMAS 12
		264,  265,  266,  264,  266,  267,   // front
		268,  269,  270,  268,  270,  271,   // right
		272,  273,  274,  272,  274,  275,  // back
		276,  278,  277,  276,  279,  278,  // left
		280,  282,  281,  280,  283,  282,  // upper
		284,  286,  285,  284,  287,  286,   // bottom

		//PETIKEMAS 13
		288,  289,  290,  288,  290,  291,   // front
		292,  293,  294,  292,  294,  295,   // right
		296,  297,  298,  296,  298,  299,  // back
		300,  302,  301,  300,  303,  302,  // left
		304,  306,  305,  304,  307,  306,  // upper
		308,  310,  309,  308,  311,  310,   // bottom

		//PETIKEMAS 14
		312,  313,  314,  312,  314,  315,   // front
		316,  317,  318,  316,  318,  319,   // right
		320,  321,  322,  320,  322,  323,  // back
		324,  326,  325,  324,  327,  326,  // left
		328,  330,  329,  328,  331,  330,  // upper
		332,  334,  333,  332,  335,  334,   // bottom

		//PETIKEMAS 15
		336,  337,  338,  336,  338,  339,   // front
		340,  341,  342,  340,  342,  343,   // right
		344,  345,  346,  344,  346,  347,  // back
		348,  350,  349,  348,  351,  350,  // left
		352,  354,  353,  352,  355,  354,  // upper
		356,  358,  357,  356,  359,  356,   // bottom

		//PETIKEMAS 16
		360,  361,  362,  360,  362,  363,   // front
		364,  365,  366,  364,  366,  367,   // right
		368,  369,  370,  368,  370,  371,  // back
		372,  374,  373,  372,  375,  374,  // left
		376,  378,  377,  376,  379,  378,  // upper
		380,  382,  381,  380,  383,  382,   // bottom

		//PETIKEMAS 17
		384,  385,  386,  384,  386,  387,   // front
		388,  389,  390,  388,  390,  391,   // right
		392,  393,  394,  392,  394,  395,  // back
		396,  398,  397,  396,  399,  398,  // left
		400,  402,  401,  400,  403,  402,  // upper
		404,  406,  405,  404,  407,  406,   // bottom

		//PETIKEMAS 18
		408,  409,  410,  408,  410,  411,   // front
		412,  413,  414,  412,  414,  415,   // right
		416,  417,  418,  416,  418,  419,  // back
		420,  422,  421,  420,  423,  422,  // left
		424,  426,  425,  424,  427,  426,  // upper
		428,  430,  429,  428,  431,  430,   // bottom

		//PETIKEMAS 19
		432,  433,  434,  432,  434,  435,   // front
		436,  437,  438,  436,  438,  439,   // right
		440,  441,  442,  440,  442,  443,  // back
		444,  446,  445,  444,  447,  446,  // left
		448,  450,  449,  448,  451,  450,  // upper
		452,  454,  453,  452,  455,  454,   // bottom

		//PETIKEMAS 20
		456,  457,  458,  456,  458,  459,   // front
		460,  461,  462,  460,  462,  463,   // right
		464,  465,  466,  464,  466,  467,  // back
		468,  470,  469,  468,  471,  470,  // left
		472,  474,  473,  472,  475,  474,  // upper
		476,  478,  477,  476,  479,  478,   // bottom
	};

	glGenVertexArrays(1, &VAO3);
	glGenBuffers(1, &VBO3);
	glGenBuffers(1, &EBO3);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::DrawColoredCubeYellow(GLuint shader)
{
	//glUseProgram(shaderProgram);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture3);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "model"), 0);

	UseShader(shader);
	glBindVertexArray(VAO3);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO3); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 1500, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::ObjekPetikemasGreen() {
	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("petikemas2.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	float vertices[] = {
		//PETIKEMAS 1
		// front
		4.5, -0.5, 0.5, 0, 0,  // 0
		7.5, -0.5, 0.5, 1, 0,   // 1
		7.5, 0.5, 0.5, 1, 1,   // 2
		4.5, 0.5, 0.5, 0, 1,  // 3
		// right
		7.5, 0.5,  0.5, 0, 0,  // 4
		7.5, 0.5, -0.5, 1, 0,  // 5
		7.5, -0.5, -0.5, 1, 1,  // 6
		7.5, -0.5,  0.5, 0, 1,  // 7
		// back
		4.5, -0.5, -0.5, 0, 0, // 8 
		7.5, -0.5, -0.5, 1, 0, // 9
		7.5, 0.5, -0.5, 1, 1, // 10
		4.5, 0.5, -0.5, 0, 1, // 11
		// left
		4.5, -0.5, -0.5, 0, 0, // 12
		4.5, -0.5, 0.5, 1, 0, // 13
		4.5, 0.5, 0.5, 1, 1, // 14
		4.5, 0.5, -0.5, 0, 1, // 15
		// upper
		7.5, 0.5, 0.5, 0, 0,   // 16
		4.5, 0.5, 0.5, 1, 0,  // 17
		4.5, 0.5, -0.5, 1, 1,  // 18
		7.5, 0.5, -0.5, 0, 1,   // 19
		// bottom
		4.5, -0.5, -0.5, 0, 0, // 20
		7.5, -0.5, -0.5, 1, 0,  // 21
		7.5, -0.5, 0.5, 1, 1,  // 22
		4.5, -0.5, 0.5, 0, 1, // 23

		//PETIKEMAS 2
		// front
		4.5, -0.5, 1.6, 0, 0,  // 24
		7.5, -0.5, 1.6, 1, 0,   // 25
		7.5, 0.5, 1.6, 1, 1,   // 26
		4.5, 0.5, 1.6, 0, 1,  // 27
		// right
		7.5, 0.5,  1.6, 0, 0,  // 28
		7.5, 0.5, 0.6, 1, 0,  // 29
		7.5, -0.5, 0.6, 1, 1,  // 30
		7.5, -0.5,  1.6, 0, 1,  // 31
		// back
		4.5, -0.5, 0.6, 0, 0, // 32
		7.5, -0.5, 0.6, 1, 0, // 33
		7.5, 0.5, 0.6, 1, 1, // 34
		4.5, 0.5, 0.6, 0, 1, // 35
		// left
		4.5, -0.5, 0.6, 0, 0, // 36
		4.5, -0.5, 1.6, 1, 0, // 37
		4.5, 0.5, 1.6, 1, 1, // 38
		4.5, 0.5, 0.6, 0, 1, // 39
		// upper
		7.5, 0.5, 1.6, 0, 0,   // 40
		4.5, 0.5, 1.6, 1, 0,  // 41
		4.5, 0.5, 0.6, 1, 1,  // 42
		7.5, 0.5, 0.6, 0, 1,   // 43
		// bottom
		4.5, -0.5, 0.6, 0, 0, // 44
		7.5, -0.5, 0.6, 1, 0,  // 45
		7.5, -0.5, 1.6, 1, 1,  // 46
		4.5, -0.5, 1.6, 0, 1, // 47

		//PETIKEMAS 3
		// front
		4.5, -0.5, 2.7, 0, 0,  // 48
		7.5, -0.5, 2.7, 1, 0,   // 25
		7.5, 0.5, 2.7, 1, 1,   // 26
		4.5, 0.5, 2.7, 0, 1,  // 27
		// right
		7.5, 0.5,  2.7, 0, 0,  // 28
		7.5, 0.5, 1.7, 1, 0,  // 29
		7.5, -0.5, 1.7, 1, 1,  // 30
		7.5, -0.5,  2.7, 0, 1,  // 31
		// back
		4.5, -0.5, 1.7, 0, 0, // 32
		7.5, -0.5, 1.7, 1, 0, // 33
		7.5, 0.5, 1.7, 1, 1, // 34
		4.5, 0.5, 1.7, 0, 1, // 35
		// left
		4.5, -0.5, 1.7, 0, 0, // 36
		4.5, -0.5, 2.7, 1, 0, // 37
		4.5, 0.5, 2.7, 1, 1, // 38
		4.5, 0.5, 1.7, 0, 1, // 39
		// upper
		7.5, 0.5, 2.7, 0, 0,   // 40
		4.5, 0.5, 2.7, 1, 0,  // 41
		4.5, 0.5, 1.7, 1, 1,  // 42
		7.5, 0.5, 1.7, 0, 1,   // 43
		// bottom
		4.5, -0.5, 1.7, 0, 0, // 44
		7.5, -0.5, 1.7, 1, 0,  // 45
		7.5, -0.5, 2.7, 1, 1,  // 46
		4.5, -0.5, 2.7, 0, 1, // 47

		//PETIKEMAS 4
		// front
		4.5, -0.5, 3.8, 0, 0,  // 48
		7.5, -0.5, 3.8, 1, 0,   // 25
		7.5, 0.5, 3.8, 1, 1,   // 26
		4.5, 0.5, 3.8, 0, 1,  // 27
		// right
		7.5, 0.5, 3.8, 0, 0,  // 28
		7.5, 0.5, 2.8, 1, 0,  // 29
		7.5, -0.5, 2.8, 1, 1,  // 30
		7.5, -0.5, 3.8, 0, 1,  // 31
		// back
		4.5, -0.5, 2.8, 0, 0, // 32
		7.5, -0.5, 2.8, 1, 0, // 33
		7.5, 0.5, 2.8, 1, 1, // 34
		4.5, 0.5, 2.8, 0, 1, // 35
		// left
		4.5, -0.5, 2.8, 0, 0, // 36
		4.5, -0.5, 3.8, 1, 0, // 37
		4.5, 0.5, 3.8, 1, 1, // 38
		4.5, 0.5, 2.8, 0, 1, // 39
		// upper
		7.5, 0.5, 3.8, 0, 0,   // 40
		4.5, 0.5, 3.8, 1, 0,  // 41
		4.5, 0.5, 2.8, 1, 1,  // 42
		7.5, 0.5, 2.8, 0, 1,   // 43
		// bottom
		4.5, -0.5, 2.8, 0, 0, // 44
		7.5, -0.5, 2.8, 1, 0,  // 45
		7.5, -0.5, 3.8, 1, 1,  // 46
		4.5, -0.5, 3.8, 0, 1, // 47

		//PETIKEMAS 5
		// front
		4.5, -0.5, 4.9, 0, 0,  // 48
		7.5, -0.5, 4.9, 1, 0,   // 25
		7.5, 0.5, 4.9, 1, 1,   // 26
		4.5, 0.5, 4.9, 0, 1,  // 27
		// right
		7.5, 0.5, 4.9, 0, 0,  // 28
		7.5, 0.5, 3.9, 1, 0,  // 29
		7.5, -0.5, 3.9, 1, 1,  // 30
		7.5, -0.5, 4.9, 0, 1,  // 31
		// back
		4.5, -0.5, 3.9, 0, 0, // 32
		7.5, -0.5, 3.9, 1, 0, // 33
		7.5, 0.5, 3.9, 1, 1, // 34
		4.5, 0.5, 3.9, 0, 1, // 35
		// left
		4.5, -0.5, 3.9, 0, 0, // 36
		4.5, -0.5, 4.9, 1, 0, // 37
		4.5, 0.5, 4.9, 1, 1, // 38
		4.5, 0.5, 3.9, 0, 1, // 39
		// upper
		7.5, 0.5, 3.9, 0, 0,   // 40
		4.5, 0.5, 3.9, 1, 0,  // 41
		4.5, 0.5, 4.9, 1, 1,  // 42
		7.5, 0.5, 4.9, 0, 1,   // 43
		// bottom
		4.5, -0.5, 3.9, 0, 0, // 44
		7.5, -0.5, 3.9, 1, 0,  // 45
		7.5, -0.5, 4.9, 1, 1,  // 46
		4.5, -0.5, 4.9, 0, 1, // 47

		//PETIKEMAS 6
		// front
		4.5, -0.5, 8.0, 0, 0,  // 48
		7.5, -0.5, 8.0, 1, 0,   // 25
		7.5, 0.5, 8.0, 1, 1,   // 26
		4.5, 0.5, 8.0, 0, 1,  // 27
		// right
		7.5, 0.5, 8.0, 0, 0,  // 28
		7.5, 0.5, 7.0, 1, 0,  // 29
		7.5, -0.5, 7.0, 1, 1,  // 30
		7.5, -0.5, 8.0, 0, 1,  // 31
		// back
		4.5, -0.5, 7.0, 0, 0, // 32
		7.5, -0.5, 7.0, 1, 0, // 33
		7.5, 0.5, 7.0, 1, 1, // 34
		4.5, 0.5, 7.0, 0, 1, // 35
		// left
		4.5, -0.5, 7.0, 0, 0, // 36
		4.5, -0.5, 8.0, 1, 0, // 37
		4.5, 0.5, 8.0, 1, 1, // 38
		4.5, 0.5, 7.0, 0, 1, // 39
		// upper
		7.5, 0.5, 7.0, 0, 0,   // 40
		4.5, 0.5, 7.0, 1, 0,  // 41
		4.5, 0.5, 8.0, 1, 1,  // 42
		7.5, 0.5, 8.0, 0, 1,   // 43
		// bottom
		4.5, -0.5, 7.0, 0, 0, // 44
		7.5, -0.5, 7.0, 1, 0,  // 45
		7.5, -0.5, 8.0, 1, 1,  // 46
		4.5, -0.5, 8.0, 0, 1, // 47

		//PETIKEMAS 7
		// front
		4.5, -0.5, 9.1, 0, 0,  // 48
		7.5, -0.5, 9.1, 1, 0,   // 25
		7.5, 0.5, 9.1, 1, 1,   // 26
		4.5, 0.5, 9.1, 0, 1,  // 27
		// right
		7.5, 0.5, 9.1, 0, 0,  // 28
		7.5, 0.5, 8.1, 1, 0,  // 29
		7.5, -0.5, 8.1, 1, 1,  // 30
		7.5, -0.5, 9.1, 0, 1,  // 31
		// back
		4.5, -0.5, 8.1, 0, 0, // 32
		7.5, -0.5, 8.1, 1, 0, // 33
		7.5, 0.5, 8.1, 1, 1, // 34
		4.5, 0.5, 8.1, 0, 1, // 35
		// left
		4.5, -0.5, 8.1, 0, 0, // 36
		4.5, -0.5, 9.1, 1, 0, // 37
		4.5, 0.5, 9.1, 1, 1, // 38
		4.5, 0.5, 8.1, 0, 1, // 39
		// upper
		7.5, 0.5, 8.1, 0, 0,   // 40
		4.5, 0.5, 8.1, 1, 0,  // 41
		4.5, 0.5, 9.1, 1, 1,  // 42
		7.5, 0.5, 9.1, 0, 1,   // 43
		// bottom
		4.5, -0.5, 8.1, 0, 0, // 44
		7.5, -0.5, 8.1, 1, 0,  // 45
		7.5, -0.5, 9.1, 1, 1,  // 46
		4.5, -0.5, 9.1, 0, 1, // 47

		//PETIKEMAS 8
		// front
		4.5, -0.5, 10.2, 0, 0,  // 48
		7.5, -0.5, 10.2, 1, 0,   // 25
		7.5, 0.5, 10.2, 1, 1,   // 26
		4.5, 0.5, 10.2, 0, 1,  // 27
		// right
		7.5, 0.5, 10.2, 0, 0,  // 28
		7.5, 0.5, 9.2, 1, 0,  // 29
		7.5, -0.5, 9.2, 1, 1,  // 30
		7.5, -0.5, 10.2, 0, 1,  // 31
		// back
		4.5, -0.5, 9.2, 0, 0, // 32
		7.5, -0.5, 9.2, 1, 0, // 33
		7.5, 0.5, 9.2, 1, 1, // 34
		4.5, 0.5, 9.2, 0, 1, // 35
		// left
		4.5, -0.5, 9.2, 0, 0, // 36
		4.5, -0.5, 10.2, 1, 0, // 37
		4.5, 0.5, 10.2, 1, 1, // 38
		4.5, 0.5, 9.2, 0, 1, // 39
		// upper
		7.5, 0.5, 9.2, 0, 0,   // 40
		4.5, 0.5, 9.2, 1, 0,  // 41
		4.5, 0.5, 10.2, 1, 1,  // 42
		7.5, 0.5, 10.2, 0, 1,   // 43
		// bottom
		4.5, -0.5, 9.2, 0, 0, // 44
		7.5, -0.5, 9.2, 1, 0,  // 45
		7.5, -0.5, 10.2, 1, 1,  // 46
		4.5, -0.5, 10.2, 0, 1, // 47

		//PETIKEMAS 9
		// front
		4.5, -0.5, 11.3, 0, 0,  // 48
		7.5, -0.5, 11.3, 1, 0,   // 25
		7.5, 0.5, 11.3, 1, 1,   // 26
		4.5, 0.5, 11.3, 0, 1,  // 27
		// right
		7.5, 0.5, 11.3, 0, 0,  // 28
		7.5, 0.5, 10.3, 1, 0,  // 29
		7.5, -0.5, 10.3, 1, 1,  // 30
		7.5, -0.5, 11.3, 0, 1,  // 31
		// back
		4.5, -0.5, 10.3, 0, 0, // 32
		7.5, -0.5, 10.3, 1, 0, // 33
		7.5, 0.5, 10.3, 1, 1, // 34
		4.5, 0.5, 10.3, 0, 1, // 35
		// left
		4.5, -0.5, 10.3, 0, 0, // 36
		4.5, -0.5, 11.3, 1, 0, // 37
		4.5, 0.5, 11.3, 1, 1, // 38
		4.5, 0.5, 10.3, 0, 1, // 39
		// upper
		7.5, 0.5, 10.3, 0, 0,   // 40
		4.5, 0.5, 10.3, 1, 0,  // 41
		4.5, 0.5, 11.3, 1, 1,  // 42
		7.5, 0.5, 11.3, 0, 1,   // 43
		// bottom
		4.5, -0.5, 10.3, 0, 0, // 44
		7.5, -0.5, 10.3, 1, 0,  // 45
		7.5, -0.5, 11.3, 1, 1,  // 46
		4.5, -0.5, 11.3, 0, 1, // 47

		//PETIKEMAS 10
		// front
		4.5, -0.5, 12.4, 0, 0,  // 48
		7.5, -0.5, 12.4, 1, 0,   // 25
		7.5, 0.5, 12.4, 1, 1,   // 26
		4.5, 0.5, 12.4, 0, 1,  // 27
		// right
		7.5, 0.5, 12.4, 0, 0,  // 28
		7.5, 0.5, 11.4, 1, 0,  // 29
		7.5, -0.5, 11.4, 1, 1,  // 30
		7.5, -0.5, 12.4, 0, 1,  // 31
		// back
		4.5, -0.5, 11.4, 0, 0, // 32
		7.5, -0.5, 11.4, 1, 0, // 33
		7.5, 0.5, 11.4, 1, 1, // 34
		4.5, 0.5, 11.4, 0, 1, // 35
		// left
		4.5, -0.5, 11.4, 0, 0, // 36
		4.5, -0.5, 12.4, 1, 0, // 37
		4.5, 0.5, 12.4, 1, 1, // 38
		4.5, 0.5, 11.4, 0, 1, // 39
		// upper
		7.5, 0.5, 11.4, 0, 0,   // 40
		4.5, 0.5, 11.4, 1, 0,  // 41
		4.5, 0.5, 12.4, 1, 1,  // 42
		7.5, 0.5, 12.4, 0, 1,   // 43
		// bottom
		4.5, -0.5, 11.4, 0, 0, // 44
		7.5, -0.5, 11.4, 1, 0,  // 45
		7.5, -0.5, 12.4, 1, 1,  // 46
		4.5, -0.5, 12.4, 0, 1, // 47
	};

	unsigned int indices[] = {
		//PETIKEMAS 1
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,   // bottom

		//PETIKEMAS 2
		24,  25,  26,  24,  26,  27,   // front
		28,  29,  30,  28,  30,  31,   // right
		32,  33,  34,  32,  34,  35,  // back
		36,  38,  37,  36,  39,  38,  // left
		40,  42,  41,  40,  43,  42,  // upper
		44,  46,  45,  44,  47,  46,   // bottom

		//PETIKEMAS 3
		48,  49,  50,  48,  50,  51,   // front
		52,  53,  54,  52,  54,  55,   // right
		56,  57,  58,  56,  58,  59,  // back
		60,  62,  61,  60,  63,  62,  // left
		64,  66,  65,  64,  67,  66,  // upper
		68,  70,  69,  68,  71,  70,   // bottom

		//PETIKEMAS 4
		72,  73,  74,  72,  74,  75,   // front
		76,  77,  78,  76,  78,  79,   // right
		80,  81,  82,  80,  82,  83,  // back
		84,  86,  85,  84,  87,  86,  // left
		88,  90,  89,  88,  91,  90,  // upper
		92,  94,  93,  92,  95,  94,   // bottom

		//PETIKEMAS 5
		96,    97,   98,   96,   98,   99,   // front
		100,  101,  102,  100,  102,  103,   // right
		104,  105,  106,  104,  106,  107,  // back
		108,  110,  109,  108,  111,  110,  // left
		112,  114,  113,  112,  115,  114,  // upper
		116,  118,  117,  116,  119,  118,   // bottom

		//PETIKEMAS 6
		120,  121,  122,  120,  122,  123,   // front
		124,  125,  126,  124,  126,  127,   // right
		128,  129,  130,  128,  130,  131,  // back
		132,  134,  133,  132,  135,  134,  // left
		136,  138,  137,  136,  139,  138,  // upper
		140,  142,  141,  140,  143,  142,   // bottom

		//PETIKEMAS 7
		144,  145,  146,  144,  146,  147,   // front
		148,  149,  150,  148,  150,  151,   // right
		152,  153,  154,  152,  154,  155,  // back
		156,  158,  157,  156,  159,  158,  // left
		160,  162,  161,  160,  163,  162,  // upper
		164,  166,  165,  164,  167,  166,   // bottom

		//PETIKEMAS 8
		168,  169,  170,  168,  170,  171,   // front
		172,  173,  174,  172,  174,  175,   // right
		176,  177,  178,  176,  178,  179,  // back
		180,  182,  181,  180,  183,  182,  // left
		184,  186,  185,  184,  187,  186,  // upper
		188,  190,  189,  188,  191,  190,   // bottom

		//PETIKEMAS 9
		192,  193,  194,  192,  194,  195,   // front
		196,  197,  198,  196,  198,  199,   // right
		200,  201,  202,  200,  202,  203,  // back
		204,  206,  205,  204,  207,  206,  // left
		208,  210,  209,  208,  211,  210,  // upper
		212,  214,  213,  212,  215,  214,   // bottom

		//PETIKEMAS 10
		216,  217,  218,  216,  218,  219,   // front
		220,  221,  222,  220,  222,  223,   // right
		224,  225,  226,  224,  226,  227,  // back
		228,  230,  229,  228,  231,  230,  // left
		232,  234,  233,  232,  235,  234,  // upper
		236,  238,  237,  236,  239,  238,   // bottom
	};

	glGenVertexArrays(1, &VAO4);
	glGenBuffers(1, &VBO4);
	glGenBuffers(1, &EBO4);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO4);

	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);



	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::DrawColoredCubeGreen(GLuint shader)
{
	//glUseProgram(shaderProgram);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture4);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "model"), 0);

	UseShader(shader);
	glBindVertexArray(VAO4);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO4); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 500, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::road()
{
	// Load and create a texture 
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("road1.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-10.0, -0.5, -15.0,  0,  0,
		 10.0, -0.5, -15.0, 50,  0,
		 10.0, -0.5,  15.0, 50, 50,
		-10.0, -0.5,  15.0,  0, 50,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawColoredPlane()
{
	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 1);

	glBindVertexArray(VAO2); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}


void Demo::BuildTexturedCube()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("petikemas0.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords, normal
		// front
		-3.0, -1.0, 1.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		3.0, -1.0, 1.0, 1, 0,  0.0f,  0.0f,  1.0f, // 1
		3.0,  1.0, 1.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		-3.0,  1.0, 1.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		 // right
		 3.0,  1.0,  1.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		 3.0,  1.0, -1.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		 3.0, -1.0, -1.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		 3.0, -1.0,  1.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// back
		-3.0, -1.0, -1.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		3.0,  -1.0, -1.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		3.0,   1.0, -1.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		-3.0,  1.0, -1.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // left
		 -3.0, -1.0, -1.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 -3.0, -1.0,  1.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 -3.0,  1.0,  1.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 -3.0,  1.0, -1.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		3.0, 1.0,  1.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		-3.0, 1.0, 1.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		-3.0, 1.0, -1.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		3.0, 1.0, -1.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19

		// bottom
		-3.0, -1.0, -1.0, 0, 0, 0.0f,  -1.0f,  0.0f, // 20
		3.0, -1.0, -1.0, 1, 0,  0.0f,  -1.0f,  0.0f, // 21
		3.0, -1.0,  1.0, 1, 1,  0.0f,  -1.0f,  0.0f, // 22
		-3.0, -1.0,  1.0, 0, 1, 0.0f,  -1.0f,  0.0f, // 23
	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22   // bottom
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::BuildTexturedPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("road1.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-25.0f,	-0.5f, -25.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f, -25.0f, 25,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f,  25.0f, 25, 25, 0.0f,  1.0f,  0.0f,
		-25.0f,	-0.5f,  25.0f,  0, 25, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawTexturedCube(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}

//Camera


void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}

void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 1.0f;
	posCamZ = 8.0f;
	viewCamX = 0.0f;
	viewCamY = 1.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.001f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Shadow Mapping Demo", 800, 600, false, false);
}