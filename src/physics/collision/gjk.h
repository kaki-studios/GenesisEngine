#pragma once
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include "collider.h"
#include <array>


struct SupportPoint{
    glm::vec3 point; //A-B
    glm::vec3 suppA; //Support point from shape A
    glm::vec3 suppB; //same
};

struct Simplex {
    std::array<SupportPoint, 4> points;
    int size = 0;

    void push_front(const SupportPoint& point) {
        if(size < 4) {
            for (int i =size; i>0; i--){
                points[i] = points[i-1];
            }
            points[0] = point;
            ++size;
        }

    }

    SupportPoint operator[](int i) {return points[i];}
    const SupportPoint operator[](int i) const{return points[i];}

};

bool NearestSimplex(Simplex& s, glm::vec3& direction);

struct GJKResult {
    bool colliding;
    Simplex simplex;
};


SupportPoint Support(const Collider* a, const Collider* b, const glm::vec3& dir);

GJKResult GJKIntersect(const Collider* a, const Collider* b);
