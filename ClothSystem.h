#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vecmath.h>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

#include "particleSystem.h"
#include "pendulumSystem.h"
#include "shader.h"


class Cuboid {
public:
	Vector3f center, size;
	float x_min, x_max, y_min, y_max, z_min, z_max;
	float EPISLON = 0.1f;

	Cuboid() {}
	Cuboid(Vector3f center, Vector3f size) {
		this->center = center;
		this->size = size;

		this->x_min = center.x() - size.x() / 2;
		this->x_max = center.x() + size.x() / 2;
		this->y_min = center.y() - size.y() / 2;
		this->y_max = center.y() + size.y() / 2;
		this->z_min = center.z() - size.z() / 2;
		this->z_max = center.z() + size.z() / 2;
	}

	bool isPointInside(Vector3f point) {
		return (point.x() < this->x_max + EPISLON && point.x() > this->x_min - EPISLON) && 
			   (point.y() < this->y_max + EPISLON && point.y() > this->y_min - EPISLON) &&
			   (point.z() < this->z_max + EPISLON && point.z() > this->z_min - EPISLON);
	}

	Vector3f pushPointToClosestSurface(Vector3f point);
	int closestAxis(Vector3f point);
};


class ClothSystem: public PendulumSystem  {
public:
	ClothSystem(int width, int height);

	void toggleWind();
	void toggleWireframe();
	void toggleSwing();
	void toggleShading();
	void toggleCloth();

	virtual void addExternelForce(vector<Vector3f>& particles_force);
	virtual void addExtraVel(vector<Vector3f>& state);
	virtual void setState(const vector<Vector3f>  & newState);

	int getParticleId(int x, int y);
	Vector3f getPos(int x, int y);
	void draw();

private:
	int width;
	int height;

	bool has_wind;
	bool show_wireframe;
	bool does_swing;
	int shading_mode;
	int cloth_id;

	float swing_degree;

	const float EPISLON = 0.1f;
	vector<Vector4f> balls;  // Vector3f pos + radius
	vector<Cuboid> cuboids;

	void computeVertexNormals();
	
	vector<Vector3f> faces;  // each entry stores the three indices of each vertex
	vector<Vector3f> normals;

	Shader phongShader, clothShader;
	unsigned int VBO, VAO;
	GLint attribVertexPosition, attribVertexNormal;

	glm::mat4 model, view, projection;
	glm::vec3 object_color, camera_pos;
	glm::vec3 light1_pos, light2_pos;

	void initRendering();
	void getLightingParameters();
	void setPhongShader();
	void setClothShader();
	void drawLightCubes();
};


#endif
