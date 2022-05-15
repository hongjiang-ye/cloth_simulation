#ifndef SPRING_H
#define SPRING_H

#include <iostream>
#include <vecmath.h>

using namespace std;

class Spring 
{
public:

    int particle1_id;
    int particle2_id;
    float rest_length;
    float Ks;  // stiffness
    float Kd;  // internal damping

    Spring(int particle1_id, int particle2_id, float rest_length, float Ks, float Kd);
    Vector3f computeForce(Vector3f pos1, Vector3f pos2, Vector3f vel1, Vector3f vel2);
};

#endif
