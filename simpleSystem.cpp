#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "simpleSystem.h"

using namespace std;

SimpleSystem::SimpleSystem() {
	m_numParticles = 1;
	m_vVecState.push_back(Vector3f(1, 0, 0));

	// shader = Shader("./shaders/particle.vs", "./shaders/particle.fs");
}

// TODO: implement evalF
// for a given state, evaluate f(X,t)
vector<Vector3f> SimpleSystem::evalF(vector<Vector3f> state)
{
	vector<Vector3f> f;

	// YOUR CODE HERE
	for (size_t i = 0; i < state.size(); i++) {
		f.push_back(Vector3f(-state[i].y(), state[i].x(), 0));
	}

	return f;
}
	
// render the system (ie draw the particles)
void SimpleSystem::draw()
{
	Vector3f pos = this->getState()[0];//YOUR PARTICLE POSITION
	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2] );
	glutSolidSphere(0.075f,10.0f,10.0f);
	glPopMatrix();

    // // set up vertex data (and buffer(s)) and configure vertex attributes
    // // ------------------------------------------------------------------
	// float vertices[] = {
    //     -0.5f, -0.5f, -0.5f, 
    //      0.5f, -0.5f, -0.5f,  
    //      0.5f,  0.5f, -0.5f,  
    //      0.5f,  0.5f, -0.5f,  
    //     -0.5f,  0.5f, -0.5f, 
    //     -0.5f, -0.5f, -0.5f, 

    //     -0.5f, -0.5f,  0.5f, 
    //      0.5f, -0.5f,  0.5f,  
    //      0.5f,  0.5f,  0.5f,  
    //      0.5f,  0.5f,  0.5f,  
    //     -0.5f,  0.5f,  0.5f, 
    //     -0.5f, -0.5f,  0.5f, 

    //     -0.5f,  0.5f,  0.5f, 
    //     -0.5f,  0.5f, -0.5f, 
    //     -0.5f, -0.5f, -0.5f, 
    //     -0.5f, -0.5f, -0.5f, 
    //     -0.5f, -0.5f,  0.5f, 
    //     -0.5f,  0.5f,  0.5f, 

    //      0.5f,  0.5f,  0.5f,  
    //      0.5f,  0.5f, -0.5f,  
    //      0.5f, -0.5f, -0.5f,  
    //      0.5f, -0.5f, -0.5f,  
    //      0.5f, -0.5f,  0.5f,  
    //      0.5f,  0.5f,  0.5f,  

    //     -0.5f, -0.5f, -0.5f, 
    //      0.5f, -0.5f, -0.5f,  
    //      0.5f, -0.5f,  0.5f,  
    //      0.5f, -0.5f,  0.5f,  
    //     -0.5f, -0.5f,  0.5f, 
    //     -0.5f, -0.5f, -0.5f, 

    //     -0.5f,  0.5f, -0.5f, 
    //      0.5f,  0.5f, -0.5f,  
    //      0.5f,  0.5f,  0.5f,  
    //      0.5f,  0.5f,  0.5f,  
    //     -0.5f,  0.5f,  0.5f, 
    //     -0.5f,  0.5f, -0.5f, 
    // };

    // unsigned int VBO, cubeVAO;
    // glGenVertexArrays(1, &cubeVAO);
    // glGenBuffers(1, &VBO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindVertexArray(cubeVAO);

    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    //  // draw our first triangle
	// this->shader.use();
    
    // GLfloat modelview_gl[16];
	// glGetFloatv(GL_MODELVIEW_MATRIX, modelview_gl);

    // // current point's translation
    // glm::mat4 modelview = glm::translate(glm::make_mat4(modelview_gl), glm::vec3(pos[0], pos[1], pos[2]));

	// GLfloat projection_gl[16];
	// glGetFloatv(GL_PROJECTION_MATRIX, projection_gl);

	// this->shader.setMat4("modelview", modelview);
	// this->shader.setMat4("projection", glm::make_mat4(projection_gl));
	// glBindVertexArray(cubeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
	// glDrawArrays(GL_TRIANGLES, 0, 36);
	// glUseProgram(0);  // important, to unbind the shader
}
