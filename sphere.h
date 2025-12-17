#pragma once
#include "object.h"
#include <stdlib.h>
#include <time.h>
#include "mat4.h"
#include "pbr.h"
#include "random.h"
#include "ray.h"
#include "material.h"
#include <immintrin.h>

// returns a random point on the surface of a unit sphere
inline vec3 random_point_on_unit_sphere()
{
    float x = RandomFloatNTP();
    float y = RandomFloatNTP();
    float z = RandomFloatNTP();
    vec3 v( x, y, z );
    return normalize(v);
}

// a spherical object
class Sphere : public Object
{
public:
    float radius;
    vec3 center;
    Material const* const material;

    Sphere(float radius, vec3 center, Material const* const material) : 
        radius(radius),
        center(center),
        material(material)
    {

    }

    ~Sphere() override
    {
    
    }

    Color GetColor()
    {
        return material->color;
    }

    
    Optional<HitResult> Intersect(Ray ray, float maxDist) override
    {
        constexpr float minDist = 0.001f;
        float len = sqrt(
            ray.m.x * ray.m.x +
            ray.m.y * ray.m.y +
            ray.m.z * ray.m.z);

        vec3 dir = {ray.m.x / len,ray.m.y / len,ray.m.z / len}; //normalise ray.m for reduced calculation
        vec3 oc = ray.b - center;
        float b = dot(oc, dir);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - c;

        // No intersection if discriminant < 0
        if (discriminant < 0.0f) 
            return Optional<HitResult>();

        float sqrtDisc = sqrt(discriminant);
        float temp = -b - sqrtDisc;
        float temp2 = -b + sqrtDisc;
        float t = (temp > minDist && temp < maxDist) ? temp : temp2;  //if temp is applicible then use temp otherwise use temp2
        if (!(t > minDist && t < maxDist)) return Optional<HitResult>();

        // Compute hit
        HitResult hit;
        hit.t = t;
        hit.p = ray.b + dir * t;
        hit.normal = (hit.p - center) * (1.0f / this->radius); 
        hit.object = this;

        return Optional<HitResult>(hit);
    }
   

    
    Ray ScatterRay(Ray ray, vec3 point, vec3 normal) override
    {
        return BSDF(this->material, ray, point, normal);
    }

};