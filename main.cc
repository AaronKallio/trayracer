#include <stdio.h>
#include "window.h"
#include "vec3.h"
#include "raytracer.h"
#include "sphere.h"
#include <stdio.h>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <random>
#include <chrono>


#define degtorad(angle) angle * MPI / 180

int main()
{ 
    
    unsigned w;
    unsigned h;
    int raysPerPixel = 1;
    int sphereNum;
    std::cout << "Enter Width: ";
    std::cin >> w;
    std::cout << "Enter Height: ";
    std::cin >> h;
    std::cout << "Enter Rays Per Pixel: ";
    std::cin >> raysPerPixel;
    std::cout << "Enter Sphere Amount: ";
    std::cin >> sphereNum;

    std::mt19937 rng;
    rng.seed(1);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    //std::mt19937 rngMinus;
    //rngMinus.seed(1);
    std::uniform_real_distribution<float> distMinus(-1.0f, 1.0f);
    std::uniform_int_distribution<int> ballType(0, 2);


    Display::Window wnd;
    
    wnd.SetTitle("TrayRacer");
    
    if (!wnd.Open())
        return 1;

    std::vector<Color> framebuffer;

    //const unsigned w = 200;
    //const unsigned h = 100;
    framebuffer.resize(w * h);
    
    
    int maxBounces = 5;

    Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);

    // Create some objects
    Material* mat = new Material();
    mat->type = 1;
    mat->color = { 0.5,0.5,0.5 };
    mat->roughness = 0.3;
    Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
    rt.AddObject(ground);
    //will make sphereNum amount of spheres instead of sphereNum * 3 
    for (int it = 0; it < sphereNum; it++)
    {
        int ball = ballType(rng); //random number to randomly choose which type of sphere

        if(ball ==1)
        {
            Material* mat = new Material();
                mat->type = 1;
                float r = dist(rng);
                float g = dist(rng);
                float b = dist(rng);
                mat->color = { r,g,b };
                mat->roughness = dist(rng);
                const float span = 10.0f;
                Sphere* ground = new Sphere(
                    dist(rng) * 0.7f + 0.2f,
                    {
                        distMinus(rng) * span,
                        dist(rng)* span + 0.2f,
                        distMinus(rng)* span
                    },
                    mat);
            rt.AddObject(ground);
        }
        else if(ball==3)
        {
            Material* mat = new Material();
            mat->type = 3;
            float r = dist(rng);
            float g = dist(rng);
            float b = dist(rng);
            mat->color = { r,g,b };
            mat->roughness = dist(rng);
            const float span = 30.0f;
            Sphere* ground = new Sphere(
                dist(rng) * 0.7f + 0.2f,
                {
                    distMinus(rng)* span,
                    dist(rng)* span + 0.2f,
                    distMinus(rng)* span
                },
                mat);
            rt.AddObject(ground);
        }
        else{
            Material* mat = new Material();
            mat->type = 2;
            float r = dist(rng);
            float g = dist(rng);
            float b = dist(rng);
            mat->color = { r,g,b };
            mat->roughness = dist(rng);
            mat->refractionIndex = 1.65;
            const float span = 25.0f;
            Sphere* ground = new Sphere(
                dist(rng) * 0.7f + 0.2f,
                {
                    distMinus(rng)* span,
                    dist(rng)* span + 0.2f,
                    distMinus(rng)* span
                },
                mat);
            rt.AddObject(ground);
        }
    }
    
    bool exit = false;

    // camera
    bool resetFramebuffer = false;
    vec3 camPos = { 0,1.0f,10.0f };
    vec3 moveDir = { 0,0,0 };

    
    float pitch = 0;
    float yaw = 0;
    float oldx = 0;
    float oldy = 0;


    float rotx = 0;
    float roty = 0;

    // number of accumulated frames
    int frameIndex = 0;

    std::vector<Color> framebufferCopy;
    framebufferCopy.resize(w * h);

    // rendering loop
    while (wnd.IsOpen() && !exit)
    {
        resetFramebuffer = false;
        moveDir = {0,0,0};
        pitch = 0;
        yaw = 0;

        // poll input
        wnd.Update();
        int channels = 3; // RGB

        rotx -= pitch;
        roty -= yaw;

        moveDir = normalize(moveDir);

        mat4 xMat = (rotationx(rotx));
        mat4 yMat = (rotationy(roty));
        mat4 cameraTransform = multiply(yMat, xMat);

        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
        
        cameraTransform.m30 = camPos.x;
        cameraTransform.m31 = camPos.y;
        cameraTransform.m32 = camPos.z;

        rt.SetViewMatrix(cameraTransform);
        
        if (resetFramebuffer)
        {
            rt.Clear();
            frameIndex = 0;
        }
        auto start = std::chrono::high_resolution_clock::now();
        rt.SetObjectArr();
        rt.Raytrace();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Elapsed time in seconds is " << elapsed.count()<<"\n";
        int rayCount = rt.RayCounter();
        std::cout << "Ray amount is " << rayCount << "\n";
        std::cout << "Rays per second is " << rayCount/ elapsed.count() << "\n";



        frameIndex++;

        // Get the average distribution of all samples
        {
            size_t p = 0;
            for (Color const& pixel : framebuffer)
            {
                framebufferCopy[p] = pixel;
                framebufferCopy[p].r /= frameIndex;
                framebufferCopy[p].g /= frameIndex;
                framebufferCopy[p].b /= frameIndex;
                p++;
            }
        }

        glClearColor(0, 0, 0, 1.0);
        glClear( GL_COLOR_BUFFER_BIT );

        wnd.Blit((float*)&framebufferCopy[0], w, h);
        wnd.SwapBuffers();

        std::vector<unsigned char> pixels(w* h * 3);

        for (size_t i = 0; i < w * h; i++) {
            pixels[i * 3 + 0] = static_cast<unsigned char>(std::min(1.0f, framebuffer[i].r) * 255.0f);  //resizes the RGB for stbi image output
            pixels[i * 3 + 1] = static_cast<unsigned char>(std::min(1.0f, framebuffer[i].g) * 255.0f);
            pixels[i * 3 + 2] = static_cast<unsigned char>(std::min(1.0f, framebuffer[i].b) * 255.0f);
        }
        stbi_write_jpg("output.jpg", w, h, 3, pixels.data(), 90);
        wnd.Close();
        return 0;
    }

    if (wnd.IsOpen())
        wnd.Close();

    return 0;
} 