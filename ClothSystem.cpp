#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ClothSystem.h"

#include <cmath>
#include <vector>

Vector3f Cuboid::pushPointToClosestSurface(Vector3f point) {
	vector<float> dists = {abs(point.x() - this->x_min + EPISLON), abs(point.x() - this->x_max - EPISLON),
						   abs(point.y() - this->y_min + EPISLON), abs(point.y() - this->y_max - EPISLON), 
						   abs(point.z() - this->z_min + EPISLON), abs(point.z() - this->z_max - EPISLON)};
	auto min_dist_it = min_element(begin(dists), end(dists));
	int min_dist_idx = min_dist_it - dists.begin();
	
	if (min_dist_idx == 0) point[0] = point.x() - dists[min_dist_idx];
	if (min_dist_idx == 1) point[0] = point.x() + dists[min_dist_idx];
	if (min_dist_idx == 2) point[1] = point.y() - dists[min_dist_idx];
	if (min_dist_idx == 3) point[1] = point.y() + dists[min_dist_idx];
	if (min_dist_idx == 4) point[2] = point.z() - dists[min_dist_idx];
	if (min_dist_idx == 5) point[2] = point.z() + dists[min_dist_idx];
	return point;
}

int Cuboid::closestAxis(Vector3f point) {
	vector<float> dists = {abs(point.x() - this->x_min + EPISLON), abs(point.x() - this->x_max - EPISLON),
						   abs(point.y() - this->y_min + EPISLON), abs(point.y() - this->y_max - EPISLON), 
						   abs(point.z() - this->z_min + EPISLON), abs(point.z() - this->z_max - EPISLON)};
	auto min_dist_it = min_element(begin(dists), end(dists));
	int min_dist_idx = min_dist_it - dists.begin();
	
	if (min_dist_idx == 0 || min_dist_idx == 1) return 0;
	if (min_dist_idx == 2 || min_dist_idx == 3) return 1;
	if (min_dist_idx == 4 || min_dist_idx == 5) return 2;
}

//TODO: Initialize here
ClothSystem::ClothSystem(int width, int height) {
	initRendering();

	this->Ks = 500.0f;
	this->Kd = 5.0f;
	this->MOTION_Kd = 1.0f;
	this->MASS = 0.3f;
	this->GRAVITY = -9.8f;

	has_wind = false;
	show_wireframe = false;
	does_swing = false; 
	shading_mode = 2;
	cloth_id = 0;
	swing_degree = 0;

	this->width = width;
	this->height = height;

	m_numParticles = width * height;

	m_vVecState = vector<Vector3f>(m_numParticles * 2);
	normals = vector<Vector3f>(m_numParticles);

	// Define the cloth particles
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			m_vVecState[getParticlePosIndex(getParticleId(i, j))] = Vector3f(i * 0.2, 0, j * 0.2);
			m_vVecState[getParticleVelIndex(getParticleId(i, j))] = Vector3f(0, 0, 0);
		}
	}

	// Define cloth faces
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (i > 0 && j > 0) {
				// counter-clockwise winding for front-facing rendering
				faces.push_back(Vector3f(getParticleId(i - 1, j - 1), getParticleId(i - 1, j), getParticleId(i, j - 1)));
				faces.push_back(Vector3f(getParticleId(i, j), getParticleId(i, j - 1), getParticleId(i - 1, j)));
			}
		}
	}

	// add structural and flex springs
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (j > 0) {
				this->addSpring(getParticleId(i, j), getParticleId(i, j - 1));
			}
			if (i > 0) {
				this->addSpring(getParticleId(i, j), getParticleId(i - 1, j));
			}
			if (j > 1) {
				this->addSpring(getParticleId(i, j), getParticleId(i, j - 2));
			}
			if (i > 1) {
				this->addSpring(getParticleId(i, j), getParticleId(i - 2, j));
			}
		}
	}

	// add shear springs
	for (int j = 0; j < height - 1; j++) {
		for (int i = 0; i < width; i++) {
			if (i > 0) {
				this->addSpring(getParticleId(i, j), getParticleId(i - 1, j + 1));
			}
			if (i < width - 1) {
				this->addSpring(getParticleId(i, j), getParticleId(i + 1, j + 1));
			}
		}
	}

	// cloth falling off
	// this->balls.push_back(Vector4f(2.0f, -3.0f, 2.0f, 1.5f));

	// cloth swing
	// this->balls.push_back(Vector4f(1.5f, -3.0f, 0.0f, 1.0f));
	// this->balls.push_back(Vector4f(0.3f, -1.5f, 0.0f, 0.5f));
	// this->fixed_points_idx.push_back(getParticleId(0, 0));
	// this->fixed_points_idx.push_back(getParticleId(width - 1, 0));

	// cloth with cuboid
	// this->cuboids.push_back(Cuboid(Vector3f(1.5f, -2.0f, 0.5f), Vector3f(1.0f)));
	// this->fixed_points_idx.push_back(getParticleId(0, 0));
	// this->fixed_points_idx.push_back(getParticleId(width - 1, 0));

	// cloth falling off with cuboid 1
	// this->cuboids.push_back(Cuboid(Vector3f(2.0f, -1.2f, 2.0f), Vector3f(1.5f)));

	// cloth falling off with cuboid 2
	this->balls.push_back(Vector4f(2.0f, -5.0f, 2.0f, 1.5f));
	this->cuboids.push_back(Cuboid(Vector3f(0.0f, -1.2f, 2.0f), Vector3f(1.0f, 1.0f, 4.0f)));
	this->cuboids.push_back(Cuboid(Vector3f(4.0f, -1.2f, 2.0f), Vector3f(1.0f, 1.0f, 4.0f)));
	
	this->cuboids.push_back(Cuboid(Vector3f(2.0f, -1.2f, 0.0f), Vector3f(4.0f, 1.0f, 1.0f)));
	this->cuboids.push_back(Cuboid(Vector3f(2.0f, -1.2f, 4.0f), Vector3f(4.0f, 1.0f, 1.0f)));
}

void ClothSystem::toggleWireframe() {
	show_wireframe = !show_wireframe;
}

void ClothSystem::toggleWind() {
	has_wind = !has_wind;
}

void ClothSystem::toggleSwing() {
	does_swing = !does_swing;
}

void ClothSystem::toggleShading() {
	shading_mode = (shading_mode + 1) % 3;
	if (shading_mode == 0)  cout << "Shading mode: smooth" << endl;
	if (shading_mode == 1)  cout << "Shading mode: phong" << endl;
	if (shading_mode == 2)  cout << "Shading mode: cloth" << endl;
}

void ClothSystem::toggleCloth() {
	cloth_id = (cloth_id + 1) % 3;
}

void ClothSystem::addExternelForce(vector<Vector3f>& particles_force) {
	if (!has_wind) return;
	for (Vector3f& force: particles_force) {
		force += Vector3f(0, 0, -(float)(rand() % 3) * 2);  // change to perlin noise should be better
	}
}

void ClothSystem::addExtraVel(vector<Vector3f>& state) {
	// swing fixed points
	if (!this->does_swing) return;
	swing_degree += 1;
	for (size_t i = 0; i < this->fixed_points_idx.size(); i++) {
		state[getParticleVelIndex(this->fixed_points_idx[i])] = 2 * Vector3f(0, 0, cos(0.4 * swing_degree * 3.14159 / 180));
	}
}

void ClothSystem::setState(const vector<Vector3f> & newState) {
	this->m_vVecState = newState;

	// Process collisions
	for (size_t i = 0; i < this->balls.size(); i++) {
		Vector3f ball_center = balls[i].xyz();
		float radius = balls[i].w();
		for (int n = 0; n < m_numParticles; n++) {
			Vector3f point_center_dis = m_vVecState[getParticlePosIndex(n)] - ball_center;
			if (point_center_dis.abs() - radius < this->EPISLON) {
				m_vVecState[getParticlePosIndex(n)] = ball_center + (radius + this->EPISLON) * point_center_dis.normalized();
			}
		}
	}

	for (size_t i = 0; i < this->cuboids.size(); i++) {
		for (int n = 0; n < m_numParticles; n++) {
			int point_idx = getParticlePosIndex(n);
			if (this->cuboids[i].isPointInside(m_vVecState[point_idx])) {
				m_vVecState[point_idx] = this->cuboids[i].pushPointToClosestSurface(m_vVecState[point_idx]);
				m_vVecState[getParticleVelIndex(n)][this->cuboids[i].closestAxis(m_vVecState[point_idx])] = 0.0f;
				// m_vVecState[getParticleVelIndex(n)] = Vector3f(0);
			}
		}
	}
}

void drawLine(Vector3f p1, Vector3f p2) {
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glVertex3fv(p1);
	glVertex3fv(p2);
	glEnd();
}

Vector3f computeTriangleNormal(Vector3f v1, Vector3f v2, Vector3f v3) {
	// v1, v2, v3 in counter-clockwise order, output the normal facing to the camera
	return Vector3f::cross(v2 - v1, v3 - v1).normalized();
}

void ClothSystem::computeVertexNormals() {
	// Compute the normal vector (facing to the camera) of each vertex, by its adjcent triangles
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			Vector3f normal(0, 0, 0);

			if (i > 0 && j > 0) {
				normal += computeTriangleNormal(getPos(i, j - 1), getPos(i - 1, j), getPos(i, j));
			}
			if (i < width - 1 && j > 0) {
				normal += computeTriangleNormal(getPos(i, j - 1), getPos(i, j), getPos(i + 1, j - 1));
				normal += computeTriangleNormal(getPos(i, j), getPos(i + 1, j), getPos(i + 1, j - 1));
			}
			if (i > 0 && j < height - 1) {
				normal += computeTriangleNormal(getPos(i, j), getPos(i - 1, j), getPos(i - 1, j + 1));
				normal += computeTriangleNormal(getPos(i, j), getPos(i - 1, j + 1), getPos(i, j + 1));
			}
			if (i < width - 1 && j < height - 1) {
				normal += computeTriangleNormal(getPos(i + 1, j), getPos(i, j), getPos(i, j + 1));
			}

			normals[getParticleId(i, j)] = normal.normalized();
		}
	}
}


void ClothSystem::draw() {
	if (this->show_wireframe) {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				if (j > 0) drawLine(getPos(i, j), getPos(i, j - 1));
				if (i > 0) drawLine(getPos(i, j), getPos(i - 1, j));
			}
		}
	} else {
		// // draw particles
		// for (int i = 0; i < m_numParticles; i++) {
		// 	Vector3f pos = m_vVecState[i];
		// 	glPushMatrix();
		// 	glTranslatef(pos[0], pos[1], pos[2] );
		// 	glutSolidSphere(0.075f,10.0f,10.0f);
		// 	glPopMatrix();
		// }

		// // draw springs
		// for (size_t i = 0; i < this->springs.size(); i++) {
		// 	glBegin(GL_LINES);
		// 	glVertex3fv(m_vVecState[springs[i].particle1_id]);
		// 	glVertex3fv(m_vVecState[springs[i].particle2_id]);
		// 	glEnd();
		// }

		this->computeVertexNormals();

		if (this->shading_mode == 0) {
			// Opengl's GL_SMOOTH shading
			for (size_t i = 0; i < this->faces.size(); i++) {
				Vector3f face = faces[i];

				// counter-clockwise order
				Vector3f v1 = m_vVecState[getParticlePosIndex((int)face[0])];
				Vector3f v2 = m_vVecState[getParticlePosIndex((int)face[1])];
				Vector3f v3 = m_vVecState[getParticlePosIndex((int)face[2])];
				Vector3f v1_n = this->normals[(int)face[0]];
				Vector3f v2_n = this->normals[(int)face[1]];
				Vector3f v3_n = this->normals[(int)face[2]];

				// front-facing rendering
				glBegin(GL_TRIANGLES);
				glNormal3fv(v1_n);
				glVertex3fv(v1);
				glNormal3fv(v2_n);
				glVertex3fv(v2);
				glNormal3fv(v3_n);
				glVertex3fv(v3);
				glEnd();

				// clockwise winding for back-facing rendering
				glBegin(GL_TRIANGLES);
				glNormal3fv(-v1_n);
				glVertex3fv(v1);
				glNormal3fv(-v3_n);
				glVertex3fv(v3);
				glNormal3fv(-v2_n);
				glVertex3fv(v2);
				glEnd();
			}
		} else if (this->shading_mode >= 1) {
			// Shading with GLSL

			// set up vertex data (and buffer(s)) and configure vertex attributes
			// ------------------------------------------------------------------
			vector<Vector3f> vertices;
			for (size_t i = 0; i < this->faces.size(); i++) {
				int id_1 = (int)faces[i][0];
				int id_2 = (int)faces[i][1];
				int id_3 = (int)faces[i][2];
				vertices.push_back(m_vVecState[getParticlePosIndex(id_1)]);  vertices.push_back(normals[id_1]);
				vertices.push_back(m_vVecState[getParticlePosIndex(id_2)]);  vertices.push_back(normals[id_2]);
				vertices.push_back(m_vVecState[getParticlePosIndex(id_3)]);  vertices.push_back(normals[id_3]);
				vertices.push_back(m_vVecState[getParticlePosIndex(id_1)]);  vertices.push_back(-normals[id_1]);
				vertices.push_back(m_vVecState[getParticlePosIndex(id_3)]);  vertices.push_back(-normals[id_3]);
				vertices.push_back(m_vVecState[getParticlePosIndex(id_2)]);  vertices.push_back(-normals[id_2]);
			}

			// set vertices data
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

			glVertexAttribPointer(this->attribVertexPosition, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(this->attribVertexPosition);
			glVertexAttribPointer(this->attribVertexNormal, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(this->attribVertexNormal);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			// determine parameters
			this->getLightingParameters();

			if (this->shading_mode == 1) {
				setPhongShader();
			} else if (this->shading_mode == 2) {
				setClothShader();
			}

			// draw cloth and unbind
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, vertices.size() * 3);
			glUseProgram(0);
			glBindVertexArray(0);

			// draw the lamp object
			drawLightCubes();
		}
	}

	// Draw collision objects
	// Balls
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(0.4f, 0.4f, 0.4f);
	for (size_t i = 0; i < this->balls.size(); i++) {
		Vector3f ball_center = balls[i].xyz();
		float radius = balls[i].w();

		glPushMatrix();
		glTranslatef(ball_center.x(), ball_center.y(), ball_center.z());
		glutSolidSphere(radius, 100.0f, 100.0f);
		glPopMatrix();
		glDisable(GL_COLOR_MATERIAL);
	}

	// Cuboids
	for (size_t i = 0; i < this->cuboids.size(); i++) {
		glPushMatrix();
		glColor3ub(255, 255, 255);
		glTranslatef(this->cuboids[i].center.x(), this->cuboids[i].center.y(), this->cuboids[i].center.z());
		glScaled(cuboids[i].size.x(), cuboids[i].size.y(), cuboids[i].size.z());
		glutSolidCube(1);
		glPopMatrix();
	}

}

void ClothSystem::getLightingParameters() {
	this->model = glm::mat4(1.0f);

	GLfloat view_gl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, view_gl);  // current on top is only the camera's view matrix
	this->view = glm::make_mat4(view_gl);

	GLfloat projection_gl[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection_gl);
	this->projection = glm::make_mat4(projection_gl);

	GLfloat object_color_gl[4];
	glGetMaterialfv(GL_FRONT, GL_AMBIENT, object_color_gl);
	this->object_color = glm::make_vec3(object_color_gl);

	glm::mat4 inverse_view = glm::inverse(glm::make_mat4(view_gl));  // to get current camera pos
	this->camera_pos = glm::vec3(inverse_view[3][0], inverse_view[3][1], inverse_view[3][2]);
}

void ClothSystem::setPhongShader() {
	// config shader
	this->phongShader.use();

	// vertex shader
	this->phongShader.setMat4("model", model);
	this->phongShader.setMat4("view", view);
	this->phongShader.setMat4("projection", projection);

	// frag shader
	this->phongShader.setVec3("material.diffuse", object_color);
	this->phongShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f);
	this->phongShader.setFloat("material.shininess", 1.0f);

	// point light 1
	this->phongShader.setVec3("pointLights[0].position", this->light1_pos);
	this->phongShader.setVec3("pointLights[0].ambient", glm::vec3(0.1f));
	this->phongShader.setVec3("pointLights[0].diffuse", glm::vec3(1.0f));
	this->phongShader.setVec3("pointLights[0].specular", glm::vec3(0.5f));
	this->phongShader.setFloat("pointLights[0].constant", 1.0f);
	this->phongShader.setFloat("pointLights[0].linear", 0.007f);
	this->phongShader.setFloat("pointLights[0].quadratic", 0.0002f);
	// point light 2
	this->phongShader.setVec3("pointLights[1].position", this->light2_pos);
	this->phongShader.setVec3("pointLights[1].ambient", glm::vec3(0.1f));
	this->phongShader.setVec3("pointLights[1].diffuse", glm::vec3(1.0f));
	this->phongShader.setVec3("pointLights[1].specular", glm::vec3(0.5f));
	this->phongShader.setFloat("pointLights[1].constant", 1.0f);
	this->phongShader.setFloat("pointLights[1].linear", 0.007f);
	this->phongShader.setFloat("pointLights[1].quadratic", 0.0002f);

	this->phongShader.setVec3("viewPos", camera_pos);
}

void ClothSystem::setClothShader() {
	// config shader
	this->clothShader.use();

	// vertex shader
	this->clothShader.setMat4("model", model);
	this->clothShader.setMat4("view", view);
	this->clothShader.setMat4("projection", projection);
	this->clothShader.setVec3("viewPos", camera_pos);

	// point light 1
	this->clothShader.setVec3("pointLights[0].position", this->light1_pos);
	this->clothShader.setVec3("pointLights[0].color", glm::vec3(1.0f));
	this->clothShader.setFloat("pointLights[0].quadratic_attenuation", 0.001f);

	// point light 2
	this->clothShader.setVec3("pointLights[1].position", this->light2_pos);
	this->clothShader.setVec3("pointLights[1].color", glm::vec3(1.0f));
	this->clothShader.setFloat("pointLights[1].quadratic_attenuation", 0.001f);
	
	if (cloth_id == 0) {
		// set cloth parameters (a)
		this->clothShader.setVec3("threads[0].albedo", glm::vec3(0.2, 0.8, 1) * 0.2f);
		this->clothShader.setFloat("threads[0].k_d", 0.3);
		this->clothShader.setFloat("threads[0].gamma_s", 12);
		this->clothShader.setFloat("threads[0].gamma_v", 24);
		this->clothShader.setFloat("threads[0].a", 0.33);
		this->clothShader.setInt("threads[0].tangent_num", 2);
		this->clothShader.setVec3("threads[0].tangents[0]", glm::normalize(glm::vec3(0, 1, tan(glm::radians(25.0f)))));
		this->clothShader.setVec3("threads[0].tangents[1]", glm::normalize(glm::vec3(0, 1, -tan(glm::radians(25.0f)))));
		this->clothShader.setFloat("threads[0].tangent_weights[0]", 0.5);
		this->clothShader.setFloat("threads[0].tangent_weights[1]", 0.5);

		this->clothShader.setVec3("threads[1].albedo", glm::vec3(0.2, 0.8, 1) * 0.2f);
		this->clothShader.setFloat("threads[1].k_d", 0.3);
		this->clothShader.setFloat("threads[1].gamma_s", 12);
		this->clothShader.setFloat("threads[1].gamma_v", 24);
		this->clothShader.setFloat("threads[1].a", 0.33);
		this->clothShader.setInt("threads[1].tangent_num", 2);
		this->clothShader.setVec3("threads[1].tangents[0]", glm::normalize(glm::vec3(1, 0, tan(glm::radians(25.0f)))));
		this->clothShader.setVec3("threads[1].tangents[1]", glm::normalize(glm::vec3(1, 0, -tan(glm::radians(25.0f)))));
		this->clothShader.setFloat("threads[1].tangent_weights[0]", 0.5);
		this->clothShader.setFloat("threads[1].tangent_weights[1]", 0.5);
	} else if (cloth_id == 1) {
		// set cloth parameters (b)
		this->clothShader.setVec3("threads[0].albedo", glm::vec3(1, 0.95, 0.05) * 0.12f);
		this->clothShader.setFloat("threads[0].k_d", 0.2);
		this->clothShader.setFloat("threads[0].gamma_s", 5);
		this->clothShader.setFloat("threads[0].gamma_v", 10);
		this->clothShader.setFloat("threads[0].a", 0.75);
		this->clothShader.setInt("threads[0].tangent_num", 2);
		this->clothShader.setVec3("threads[0].tangents[0]", glm::normalize(glm::vec3(0, 1, -tan(glm::radians(35.0f)))));
		this->clothShader.setVec3("threads[0].tangents[1]", glm::normalize(glm::vec3(0, 1, tan(glm::radians(35.0f)))));
		this->clothShader.setVec3("threads[0].tangents[2]", glm::vec3(0, 1, 0));
		this->clothShader.setFloat("threads[0].tangent_weights[0]", 1);
		this->clothShader.setFloat("threads[0].tangent_weights[1]", 1);
		this->clothShader.setFloat("threads[0].tangent_weights[2]", 1);

		this->clothShader.setVec3("threads[1].albedo", glm::vec3(1, 0.95, 0.05) * 0.16f);
		this->clothShader.setFloat("threads[1].k_d", 0.3);
		this->clothShader.setFloat("threads[1].gamma_s", 18);
		this->clothShader.setFloat("threads[1].gamma_v", 32);
		this->clothShader.setFloat("threads[1].a", 0.25);
		this->clothShader.setInt("threads[1].tangent_num", 1);
		this->clothShader.setVec3("threads[1].tangents[0]", glm::normalize(glm::vec3(1, 0, 0)));
		this->clothShader.setFloat("threads[1].tangent_weights[0]", 1);
	} else if (cloth_id == 2) {
		// set cloth parameters (e)
		this->clothShader.setVec3("threads[0].albedo", glm::vec3(0.1, 1, 0.4) * 0.2f);
		this->clothShader.setFloat("threads[0].k_d", 0.1);
		this->clothShader.setFloat("threads[0].gamma_s", 4);
		this->clothShader.setFloat("threads[0].gamma_v", 8);
		this->clothShader.setFloat("threads[0].a", 0.86);
		this->clothShader.setInt("threads[0].tangent_num", 2);
		this->clothShader.setVec3("threads[0].tangents[0]", glm::normalize(glm::vec3(0, 1, tan(glm::radians(25.0f)))));
		this->clothShader.setVec3("threads[0].tangents[1]", glm::normalize(glm::vec3(0, 1, -tan(glm::radians(25.0f)))));
		this->clothShader.setFloat("threads[0].tangent_weights[0]", 0.5);
		this->clothShader.setFloat("threads[0].tangent_weights[1]", 0.5);

		this->clothShader.setVec3("threads[1].albedo", glm::vec3(1, 0, 0.1) * 0.6f);
		this->clothShader.setFloat("threads[1].k_d", 0.1);
		this->clothShader.setFloat("threads[1].gamma_s", 5);
		this->clothShader.setFloat("threads[1].gamma_v", 10);
		this->clothShader.setFloat("threads[1].a", 0.14);
		this->clothShader.setInt("threads[1].tangent_num", 1);
		this->clothShader.setVec3("threads[1].tangents[0]", glm::normalize(glm::vec3(1, 0, 0)));
		this->clothShader.setFloat("threads[1].tangent_weights[0]", 1);
	}
}

void ClothSystem::drawLightCubes() {
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glColor3ub(255, 255, 255);
    glTranslatef(this->light1_pos[0], this->light1_pos[1], this->light1_pos[2]);
    glScaled(0.2f, 0.2f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();

	glPushMatrix();
	glColor3ub(255, 255, 255);
    glTranslatef(this->light2_pos[0], this->light2_pos[1], this->light2_pos[2]);
    glScaled(0.2f, 0.2f, 0.2f);
    glutSolidCube(1);
    glPopMatrix();
	
	glEnable(GL_LIGHTING);
}

void ClothSystem::initRendering() {
	this->phongShader = Shader("./shaders/phong_shading.vs", "./shaders/phong_shading.fs");
	this->clothShader = Shader("./shaders/cloth.vs", "./shaders/cloth.fs");
	this->attribVertexPosition = glGetAttribLocation(phongShader.ID, "aPos");
	this->attribVertexNormal = glGetAttribLocation(phongShader.ID, "aNormal");

	// this->light1_pos = glm::vec3(1.7f, -1.0f, 5.0f);
	this->light1_pos = glm::vec3(3.0f, 2.0f, 5.0f);
	this->light2_pos = glm::vec3(1.0f, 1.0f, -3.0f);

	glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

	glVertexAttribPointer(this->attribVertexPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(this->attribVertexPosition);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int ClothSystem::getParticleId(int x, int y) {
	// (x, y)
	return y * this->width + x;
}

Vector3f ClothSystem::getPos(int x, int y) {
	// Position vector of particle (x, y)
	return m_vVecState[getParticlePosIndex(getParticleId(x, y))];
}
