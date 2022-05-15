#include "spring.h"

Spring::Spring(int particle1_id, int particle2_id, float rest_length, float Ks, float Kd) {
    this->particle1_id = particle1_id;
    this->particle2_id = particle2_id;
    this->rest_length = rest_length;
    this->Ks = Ks;
    this->Kd = Kd;
}

Vector3f Spring::computeForce(Vector3f pos1, Vector3f pos2, Vector3f vel1, Vector3f vel2) {
    // Compute the force applied to particle 1.
    Vector3f d = pos2 - pos1;
    float d_abs = d.abs();
    Vector3f f_s = this->Ks * (d / d_abs) * (d_abs - this->rest_length);

    // internal damping
    Vector3f f_d = -this->Kd * Vector3f::dot(d / d_abs, vel2 - vel1) * (d_abs - this->rest_length);
    // Vector3f f_d = 0.0f;

    return f_s + f_d;
}
