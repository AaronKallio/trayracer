/*
#include "raytracer.h"
#include <thread>
#include <random>
int rayCount = 0;
std::vector<Object*> uniqueObjects;  //store all the objects once so that they dont have to be re counted for every pixel

void
Raytracer::SetObjectArr()
{
    for (size_t i = 0; i < this->objects.size(); ++i)
    {
        Object* obj = this->objects[i];
        std::vector<Object*>::iterator it = std::find_if(uniqueObjects.begin(), uniqueObjects.end(),
            [obj](const auto& val)
            {
                return (obj->GetId() == val->GetId());
            }
        );

        if (it == uniqueObjects.end())
        {
            uniqueObjects.push_back(obj);
        }
    }
    //std::sort(uniqueObjects.begin(), uniqueObjects.end());
    //sort(uniqueObjects);
}




Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h)
{
    // empty
}




    void Raytracer::Raytrace()
    {
        //splits the pixels into rows and uses multithreading - faster for the most part except when
        // there is a low amount of pixels in the first place e.g. 100x100
        
        int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;

        auto render_rows = [this](int start_y, int end_y) {
            static int leet = 1337; // randomness with seed to make image more realistic (no hard edges etc)
            std::mt19937 generator(leet++);
            std::uniform_real_distribution<float> dis(0.0f, 1.0f);

            for (int y = start_y; y < end_y; ++y)
            {
                for (int x = 0; x < this->width; ++x)
                {
                    Color color;
                    for (int i = 0; i < this->rpp; ++i)
                    {

                        float u = ((float(x + dis(generator)) / float(this->width)) * 2.0f) - 1.0f;
                        float v = ((float(y + dis(generator)) / float(this->height)) * 2.0f) - 1.0f;

                        vec3 direction = vec3(u, v, -1.0f);
                        direction = transform(direction, this->frustum);

                        Ray* ray = new Ray(get_position(this->view), direction);
                        color += this->TracePath(*ray, 0);   //
                        rayCount++;
                        delete ray;
                    }

                    color.r /= this->rpp;
                    color.g /= this->rpp;
                    color.b /= this->rpp;

                    this->frameBuffer[y * this->width + x] += color;
                }
            }
            };




    // Split rows across threads
    int rows_per_thread = this->height / num_threads;
    for (int t = 0; t < num_threads; ++t)
    {
        int start = t * rows_per_thread;
        int end = (t == num_threads - 1) ? this->height : start + rows_per_thread;
        threads.emplace_back(render_rows, start, end);
    }

    // Join threads
    for (auto& th : threads) th.join();
}



Color
Raytracer::TracePath(Ray ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;  

    if (Raycast(ray, hitPoint, hitNormal, hitObject, distance, this->objects))
    {
        rayCount++;
        Ray* scatteredRay = new Ray(hitObject->ScatterRay(ray, hitPoint, hitNormal));
        if (n < this->bounces)
        {
            return hitObject->GetColor() * this->TracePath(*scatteredRay, n + 1);
        }
        delete scatteredRay;

        if (n == this->bounces)
        {
            return {0,0,0};
        }
    }

    return this->Skybox(ray.m);
}




bool
Raytracer::Raycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance, std::vector<Object*> world)
{
    bool isHit = false;
    HitResult closestHit;
    HitResult hit;


    for (int i = 0; i < uniqueObjects.size(); i++)
    {
           auto objectIt = &uniqueObjects[i]; //uses pre prepared array of objects and doesnt sort for no reason 
            Object* object = *objectIt;
            auto opt = object->Intersect(ray, closestHit.t);
            if (opt.HasValue())
            {
                hit = opt.Get();
                assert(hit.t < closestHit.t);
                hitPoint = hit.p;
                hitNormal = hit.normal;
                hitObject = hit.object;
                distance = closestHit.t;

                return true;   //used to only return after every object has been checked 
            }

    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    return isHit;
}

void
Raytracer::Clear()
{
    for (auto& color : this->frameBuffer)
    {
        color.r = 0.0f;
        color.g = 0.0f;
        color.b = 0.0f;
    }
}


void
Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view); 
    mat4 basis = transpose(inverseView);
    this->frustum = basis;
}


Color
Raytracer::Skybox(vec3 direction)
{
    float t = 0.5*(direction.y + 1.0);
    vec3 vec = vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
    return {(float)vec.x, (float)vec.y, (float)vec.z};
}

int Raytracer::RayCounter()
{   
    return rayCount;
}


bool Raytracer::Sphereintersect(Object* objArr[],float t) {
    return true;
}
*/



#include "raytracer.h"
#include <thread>
#include <random>
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <iostream>
int rayCount = 0;
std::vector<Object*> uniqueObjects; //store all the objects once so that they dont have to be re counted for every pixel
OptimisedObject* objs;
//std::vector<Object*>& objects;
//std::vector<Object*> sortedObj[64][64];
void Raytracer::SetObjectArr()
{
    for (auto* obj : this->objects)
    {
        auto it = std::find_if(uniqueObjects.begin(), uniqueObjects.end(),
            [obj](const auto& val) { return obj->GetId() == val->GetId(); });
        if (it == uniqueObjects.end())
            uniqueObjects.push_back(obj);
    }
}

void Raytracer::SetOptArr() {
    objs =  this->optObj;
}

void Raytracer::SetObjectSort() {
    for (auto* obj : objects)
    {
        //int x = obj.
    }
}

Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces): frameBuffer(frameBuffer), rpp(rpp), bounces(bounces), width(w), height(h)
{
}


void Raytracer::Raytrace()
{
    //splits the pixels into rows and uses multithreading - faster for the most part except when
    // there is a low amount of pixels in the first place e.g. 100x100
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // Fallback
    std::vector<std::thread> threads;
    
    auto render_rows = [this](int start_y, int end_y) {
        static thread_local std::mt19937 generator(1337); // randomness with seed to make image more realistic (no hard edges etc)
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        const vec3 cameraPos = get_position(view);
        for (int y = start_y; y < end_y; ++y)
        {
            for (int x = 0; x < this->width; ++x)
            {
                Color color{};
                for (int i = 0; i < this->rpp; ++i)
                {
                    float u = ((x + dis(generator)) / float(width)) * 2.0f - 1.0f;
                    float v = ((y + dis(generator)) / float(height)) * 2.0f - 1.0f;

                    vec3 direction = transform(
                        vec3(u, v, -1.0f),
                        frustum
                    );

                    Ray ray(cameraPos, direction);

                    //color += TracePath(ray, 0);
                    color += TracePathOpti(ray, 0);
                }

                //color /= float(this->rpp);
                color.r /= this->rpp; 
                color.g /= this->rpp; 
                color.b /= this->rpp;
                this->frameBuffer[y * this->width + x] += color;
            }
        }
        };

    // Split rows across threads
    int rows_per_thread = this->height / num_threads;
    for (int t = 0; t < num_threads; ++t)
    {
        int start = t * rows_per_thread;
        int end = (t == num_threads - 1) ? this->height : start + rows_per_thread;
        threads.emplace_back(render_rows, start, end);
    }
    // Join threads
    for (auto& th : threads) th.join();
}

float distance = FLT_MAX;
Color Raytracer::TracePath(const Ray& ray, unsigned n)
{
    vec3 hitPoint, hitNormal;
    Object* hitObject = nullptr;

    float radius;
    vec3 centre;
    Material* mat;
    //float distance = FLT_MAX;

    if (Raycast(ray, hitPoint, hitNormal, hitObject))//, distance))
    {
        //++rayCount;
        Ray scattered = hitObject->ScatterRay(ray, hitPoint, hitNormal);

        if (n < this->bounces)
            return hitObject->GetColor() * TracePath(scattered, n + 1);

        //if (n == this->bounces)
            return { 0,0,0 };
    }

    return this->Skybox(ray.m);
}


Color Raytracer::TracePathOpti(const Ray& ray, unsigned n)
{
    vec3 hitPoint, hitNormal;
    int obj = -1;
    //float radius;
    //vec3 centre;
    //Material* mat;
    //float distance = FLT_MAX;

    if (RaycastOpti(ray, hitPoint, hitNormal, obj))//, distance))
    {
        //++rayCount;
        //std::cout << " bruv bud ";
        Ray scattered = BSDF(objs->material[obj], ray, hitPoint, hitNormal);
        Color color = { 1,0,0 };
        if (n < this->bounces)
            return objs->material[obj]->color * TracePathOpti(scattered, n + 1);
            //return objs->material[obj]->color;
        return { 0,0,0 };
    }

    return this->Skybox(ray.m);
}




//BSDF(this->material, ray, point, normal)
bool Raytracer::Raycast(const Ray& ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject){//, float& distance)//, std::vector<Object*>& objects)
//{
    HitResult closestHit;
    HitResult hit;
    float len = sqrt(
        ray.m.x * ray.m.x +
        ray.m.y * ray.m.y +
        ray.m.z * ray.m.z);
    for (auto* object : uniqueObjects) //uses pre prepared array of objects and doesnt sort for no reason 
    {
        auto opt = object->Intersect(ray, closestHit.t, len); 
        if (opt.HasValue())
        {
            hit = opt.Get();
            assert(hit.t < closestHit.t);
            hitPoint = hit.p;
            hitNormal = hit.normal;
            hitObject = hit.object;
            distance = hit.t;
            return true;  //used to only return after every object has been checked 
        }
    }


    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    return false;
}



const float minDist = 0.001f;
inline int IntersectOpti(const Ray& ray, float maxDist, float len, int i, float* tHit, vec3* pHit, vec3* normalHit)
{

    //float len = sqrt(
    //    ray.m.x * ray.m.x +
    //    ray.m.y * ray.m.y +
    //    ray.m.z * ray.m.z);

    vec3 dir = { ray.m.x / len,ray.m.y / len,ray.m.z / len }; //normalise ray.m for reduced calculation
    vec3 oc = ray.b - objs->centre[i];
    float b = dot(oc, dir);
    float c = dot(oc, oc) - objs->radius[i] * objs->radius[i];
    float discriminant = b * b - c;

    // No intersection if discriminant < 0
    if (discriminant < 0.0f)
        return -1;

    float sqrtDisc = sqrt(discriminant);
    float temp = -b - sqrtDisc;
    float temp2 = -b + sqrtDisc;
    float t = (temp > minDist && temp < maxDist) ? temp : temp2;  //if temp is applicible then use temp otherwise use temp2
    if (!(t > minDist && t < maxDist)) return -1;

    // Compute hit
    //HitResult hit;

    *tHit = i;
    *pHit = ray.b + dir * t;
    *normalHit = (*pHit - objs->centre[i]) * (1.0f / objs->radius[i]);
    //hit.object = this;

    return i;
}


bool Raytracer::RaycastOpti(const Ray& ray, vec3& hitPoint, vec3& hitNormal, int hitIndex) {

    float closestHit;
    vec3 pClose;              // hit point
    vec3 normalClose;         // normal
    float tClose = FLT_MAX;

    int hit;
    vec3 p;              // hit point
    vec3 normal;         // normal
    float t = FLT_MAX;

    float len = sqrt(
        ray.m.x * ray.m.x +
        ray.m.y * ray.m.y +
        ray.m.z * ray.m.z);

    for (int i = 0; i < objs->centre.size(); i++) //uses pre prepared array of objects and doesnt sort for no reason 
    {
        auto opt = IntersectOpti(ray, tClose, len, i, &closestHit, &pClose, &normalClose);
        if (opt != -1)
        {
            hit = i;
            assert(t < tClose);
            hitPoint = p;
            hitNormal = normal;
            hitIndex = i;
            distance = t;
            return true;  //used to only return after every object has been checked 
        }
    }



    hitPoint = pClose;
    hitNormal = normalClose;
    hitIndex = closestHit;
    return false;
}

void Raytracer::Clear()
{
    std::fill(frameBuffer.begin(), frameBuffer.end(), Color{ 0.0f, 0.0f, 0.0f });
}


void Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view);
    this->frustum = transpose(inverseView);
}


Color Raytracer::Skybox(const vec3& direction)
{
    float t = 0.5f * (direction.y + 1.0f);
    vec3 color = vec3(1.0f, 1.0f, 1.0f) * (1.0f - t) + vec3(0.5f, 0.7f, 1.0f) * t;
    return { float(color.x), float(color.y), float(color.z) };
}


int Raytracer::RayCounter()
{
    return rayCount;
}

