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
        
        vec3 oc = ray.b - this->center;
        vec3 dir = ray.m;
        float b = dot(oc, dir);
    
        // early out if sphere is "behind" ray
        if (b > 0)
            return Optional<HitResult>();

        float a = dot(dir, dir);
        float c = dot(oc, oc) - this->radius * this->radius;
        float discriminant = b * b - a * c;

        if (discriminant > 0)
        {
            constexpr float minDist = 0.001f;
            float div = 1.0f / a;
            float sqrtDisc = sqrt(discriminant);
            float temp = (-b - sqrtDisc) * div;
            float temp2 = (-b + sqrtDisc) * div;

            if (temp < maxDist && temp > minDist)
            {
                HitResult hit;
                vec3 p = ray.PointAt(temp);
                hit.p = p;
                hit.normal = (p - this->center) * (1.0f / this->radius);
                hit.t = temp;
                hit.object = this;
                return Optional<HitResult>(hit);
            }
            if (temp2 < maxDist && temp2 > minDist)
            {
                HitResult hit;
                vec3 p = ray.PointAt(temp2);
                hit.p = p;
                hit.normal = (p - this->center) * (1.0f / this->radius);
                hit.t = temp2;
                hit.object = this;
                return Optional<HitResult>(hit);
            }
        }

        return Optional<HitResult>();
    }
    

    /*
    Optional<HitResult> Intersect(Ray ray, float maxDist) override
    {
        constexpr float minDist = 0.001f;
        vec3 oc = ray.b - center;
        float b = dot(oc, ray.m);
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
        hit.p = ray.b + ray.m * t;
        hit.normal = (hit.p - center) * (1.0f / this->radius); 
        hit.object = this;

        return Optional<HitResult>(hit);
    }
    */

    Hit4 Intersect4(Ray4 ray, __m128 cx, __m128 cy, __m128 cz, float radius, float maxDist)
    {
        __m128 minDist = _mm_set1_ps(0.001f);
        __m128 maxD = _mm_set1_ps(maxDist);
        __m128 r2 = _mm_set1_ps(radius * radius);

        // oc = ray.origin - sphere.center
        __m128 ox = _mm_sub_ps(ray.ox, cx);
        __m128 oy = _mm_sub_ps(ray.oy, cy);
        __m128 oz = _mm_sub_ps(ray.oz, cz);

        // b = dot(oc, dir)
        __m128 b = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ox, ray.dx), _mm_mul_ps(oy, ray.dy)),
            _mm_mul_ps(oz, ray.dz)
        );

        // c = dot(oc, oc) - r^2
        __m128 c = _mm_sub_ps(
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(ox, ox), _mm_mul_ps(oy, oy)),
                _mm_mul_ps(oz, oz)
            ),
            r2
        );

        // discriminant = b*b - c
        __m128 disc = _mm_sub_ps(_mm_mul_ps(b, b), c);

        // sqrt(discriminant)
        __m128 sqrtDisc = _mm_sqrt_ps(_mm_max_ps(disc, _mm_setzero_ps()));

        // t0 = -b - sqrtDisc, t1 = -b + sqrtDisc
        __m128 t0 = _mm_sub_ps(_mm_setzero_ps(), _mm_add_ps(b, sqrtDisc));
        __m128 t1 = _mm_sub_ps(_mm_setzero_ps(), _mm_sub_ps(b, sqrtDisc));

        // select nearest valid t
        __m128 mask0 = _mm_and_ps(_mm_cmpgt_ps(t0, minDist), _mm_cmplt_ps(t0, maxD));
        __m128 mask1 = _mm_and_ps(_mm_cmpgt_ps(t1, minDist), _mm_cmplt_ps(t1, maxD));

        // t = (mask0) ? t0 : t1
        __m128 t = _mm_or_ps(_mm_and_ps(mask0, t0), _mm_and_ps(_mm_andnot_ps(mask0, mask1), t1));

        // final hit mask
        __m128 hitMask = _mm_or_ps(mask0, mask1);

        Hit4 result;
        result.t = t;
        result.mask = hitMask;

        return result;
    }

    
    Ray ScatterRay(Ray ray, vec3 point, vec3 normal) override
    {
        return BSDF(this->material, ray, point, normal);
    }

};