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
}



//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/


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


//------------------------------------------------------------------------------
/**
 * @parameter n - the current bounce level
*/
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

//------------------------------------------------------------------------------
/**
*/


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

//------------------------------------------------------------------------------
/**
*/
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

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view); 
    mat4 basis = transpose(inverseView);
    this->frustum = basis;
}

//------------------------------------------------------------------------------
/**
*/
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