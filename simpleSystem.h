#ifndef SIMPLESYSTEM_H
#define SIMPLESYSTEM_H

#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

#include <vecmath.h>
#include <vector>

#include "particleSystem.h"
#include "shader.h"

using namespace std;

class SimpleSystem: public ParticleSystem
{
public:
	SimpleSystem();
	
	vector<Vector3f> evalF(vector<Vector3f> state);
	virtual void setState(const vector<Vector3f>  & newState) { m_vVecState = newState; };
	
	void draw();

// private:
// 	Shader shader;

};

#endif
