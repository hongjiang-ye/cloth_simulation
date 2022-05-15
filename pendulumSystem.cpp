#include <iostream>
#include <cstdlib>
#include "pendulumSystem.h"

using namespace std;

PendulumSystem::PendulumSystem(int numParticles) : ParticleSystem(numParticles)
{
	this->Ks = 50.0f;
	this->Kd = 2.0f;
	this->MOTION_Kd = 0.5f;
	this->MASS = 1.0f;
	this->GRAVITY = -9.8f;

	m_numParticles = numParticles;  // should be >= 2
	
	// fill in code for initializing the state based on the number of particles
	m_vVecState = vector<Vector3f>(m_numParticles * 2);

	for (int i = 0; i < m_numParticles; i++) {
		// for this system, we care about the position and the velocity
		m_vVecState[getParticlePosIndex(i)] = Vector3f(i * 0.2, 0, 0);  // arrange in x axis
		m_vVecState[getParticleVelIndex(i)] = Vector3f(0, 0, 0);  // zero initial velocity

		// set springs
		if (i > 0) {
			this->addSpring(i - 1, i);
		}
	}

	// Set fixed points
	this->fixed_points_idx.push_back(0);
}

void PendulumSystem::addSpring(int particle1_id, int particle2_id) {
	float rest_length = (this->m_vVecState[getParticlePosIndex(particle1_id)] - 
						 this->m_vVecState[getParticlePosIndex(particle2_id)]).abs();
	this->springs.push_back(Spring(particle1_id, particle2_id, rest_length, this->Ks, this->Kd));
}

void printPoint(Vector3f point) {
	cout << "Point: (" << point.x() << ", " << point.y() << ", " << point.z() << ")" << endl;
}

vector<Vector3f> PendulumSystem::evalForces(vector<Vector3f> state) {
	// Compute F(x, v)

	// Store the current sum of the applied forces
	vector<Vector3f> particles_force(m_numParticles, Vector3f(0, 0, 0));

	for (int i = 0; i < m_numParticles; i++) {
		// Apply gravity, and y axis is the vertical axis
		particles_force[i] += Vector3f(0, this->GRAVITY, 0) * this->MASS;

		// Apply motion damping
		particles_force[i] += -1.0f * this->MOTION_Kd * state[getParticleVelIndex(i)];
	}

	// Update springs' forces
	for (size_t i = 0; i < this->springs.size(); i++) {
		Spring spring = springs[i];
		Vector3f force = spring.computeForce(
			state[getParticlePosIndex(spring.particle1_id)], 
			state[getParticlePosIndex(spring.particle2_id)], 
			state[getParticleVelIndex(spring.particle1_id)],
			state[getParticleVelIndex(spring.particle2_id)]
		);
		particles_force[spring.particle1_id] += force;
		particles_force[spring.particle2_id] += -force;
	}

	return particles_force;
}

void PendulumSystem::clearFixedPointsForce(vector<Vector3f>& particles_force) {
	for (size_t i = 0; i < this->fixed_points_idx.size(); i++) {
		particles_force[this->fixed_points_idx[i]] = Vector3f(0);
	}	
}

void PendulumSystem::addExternelForce(vector<Vector3f>& particles_force) {}

void PendulumSystem::addExtraVel(vector<Vector3f>& state) {}

// TODO: implement evalF
// for a given state, evaluate f(X,t)
vector<Vector3f> PendulumSystem::evalF(vector<Vector3f> state)
{
	vector<Vector3f> f(m_numParticles * 2);

	// YOUR CODE HERE
	vector<Vector3f> particles_force = this->evalForces(state);

	this->addExternelForce(particles_force);
	this->clearFixedPointsForce(particles_force);
	this->addExtraVel(state);

	// Compute accleration by the summed force
	vector<Vector3f> acc(m_numParticles);
	for (int i = 0; i < m_numParticles; i++) {
		acc[i] = particles_force[i] / this->MASS;
	}

	vector<Vector3f> vel(state.begin() + m_numParticles, state.end());
	for (int i = 0; i < m_numParticles; i++) {
		f[i] = vel[i];
		f[i + m_numParticles] = acc[i];
	}

	return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw()
{
	for (int i = 0; i < m_numParticles; i++) {
		Vector3f pos = m_vVecState[getParticlePosIndex(i)];//  position of particle i. YOUR CODE HERE
		glPushMatrix();
		glTranslatef(pos[0], pos[1], pos[2] );
		glutSolidSphere(0.075f,10.0f,10.0f);
		glPopMatrix();
	}

	// draw a line for spring
	for (size_t i = 0; i < this->springs.size(); i++) {
		// why set colors didn't work?
		glBegin(GL_LINES);
			glColor3f(1., 1., 1.);
			glVertex3fv(m_vVecState[springs[i].particle1_id]);
			glVertex3fv(m_vVecState[springs[i].particle2_id]);
		glEnd();
	}
}

int PendulumSystem::getParticlePosIndex(int particle_id) {
	return particle_id;
}

int PendulumSystem::getParticleVelIndex(int particle_id) {
	return particle_id + this->m_numParticles;
}
