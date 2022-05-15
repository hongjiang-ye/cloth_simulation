#ifndef PENDULUMSYSTEM_H
#define PENDULUMSYSTEM_H

#include <vecmath.h>
#include <vector>
#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

#include "particleSystem.h"
#include "spring.h"

class PendulumSystem: public ParticleSystem
{
public:
	PendulumSystem() {}
	PendulumSystem(int numParticles);
	
	vector<Vector3f> evalF(vector<Vector3f> state);
	vector<Vector3f> evalForces(vector<Vector3f> state);
	virtual void addExternelForce(vector<Vector3f>& particles_force);
	virtual void addExtraVel(vector<Vector3f>& state);
	void clearFixedPointsForce(vector<Vector3f>& particles_force);

	void draw();
	virtual void setState(const vector<Vector3f>  & newState) { m_vVecState = newState; };

	int getParticlePosIndex(int particle_id);
	int getParticleVelIndex(int particle_id);
	void addSpring(int particle1_id, int particle2_id);

	vector<Spring> springs;
	vector<int> fixed_points_idx;

	float Ks;
	float Kd;
	float MOTION_Kd;
	float MASS;
	float GRAVITY;
};

#endif
