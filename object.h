#pragma once
#include "ray.h"
#include "color.h"
#include <float.h>
#include <string>
#include <memory>

class Object;

//------------------------------------------------------------------------------
/**
*/
struct HitResult
{
    // hit point
    vec3 p;
    // normal
    vec3 normal;
    // hit object, or nullptr
    Object* object = nullptr;
    // intersection distance
    float t = FLT_MAX;
};

struct Ray4 {
    __m128 ox, oy, oz; // ray origins
    __m128 dx, dy, dz; // ray directions
};

struct Hit4 {
    __m128 t;          // hit distances
    __m128 mask;       // 1.0 if hit, 0.0 if no hit
};






template<class TYPE>
class Optional
{
public:
    Optional() {}
    Optional(HitResult hit) : hasValue(true)
    {
        this->value = std::make_shared<HitResult>(hit);
    }

    bool HasValue()
    {
        // check if this object has a value.
        if (this->hasValue)
        {
            // doublecheck the value
            if (value == nullptr)
            {
                return false;
            }
            if (value != nullptr)
            {
                // doublecheck the value content.
                if (value->object == nullptr)
                {
                    return false;
                }
                if (value->normal.IsZero())
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        return true;
    }
    HitResult Get()
    {
        assert(this->HasValue());
        return *value;
    }

private:
    bool hasValue = false;
    std::shared_ptr<HitResult> value = nullptr;
};

//------------------------------------------------------------------------------
/**
*/
class Object
{
public:
    Object() 
    {
        static unsigned long long idCounter = 0;
        id = idCounter++;
        // Reserve characters for naming this object something!
        name = new char[256];
        name[0] = 'U';
        name[1] = 'n';
        name[2] = 'n';
        name[3] = 'a';
        name[4] = 'm';
        name[5] = 'e';
        name[6] = 'd';
        name[7] = '\0';

        purpose = std::string("I don't have a purpose at the moment, but hopefully the programmer that overrides me will give me one. :)");
    }

    virtual ~Object()
    {
        // clean up name!
        delete name;
    }

    virtual Optional<HitResult> Intersect(Ray ray, float maxDist) { return {}; };
    virtual Hit4 Intersect4(Ray4 ray, __m128 cx, __m128 cy, __m128 cz, float radius, float maxDist) { { return {}; } };
    virtual Color GetColor() = 0;
    virtual Ray ScatterRay(Ray ray, vec3 point, vec3 normal) { return Ray({ 0,0,0 }, {1,1,1}); };
    std::string GetName() { return std::string((const char*)name); }
    unsigned long long GetId() { return this->id; }

private:
    volatile bool isBigObject = false;
    volatile char* name;
    unsigned long long id;
    std::string purpose;
};