#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "math/Matrix.h"

#include "Scene.h"
#include "BVH.h"
#include "Image.h"
#include "basic/SceneNode.h"
#include "HDRLoader/HDRloader.h"

// #define WIREFRAME_ON
#define EXPLICIT_LIGHT_SAMPLE
// #define RUSSIAN_ROULETTE_TERMINATION

namespace new_type_renderer
{
    class Raytracer
    {
    public:
        Raytracer(unsigned width, unsigned height, int _samples);

        ~Raytracer();

        Color tracing(Ray& ray, int depth, int E);
        
        void renderIndirectProgressive(Color* colorArray, bool& abort, bool& restart, int& samples);

        void renderIndirect(double& time, Image& image);

        void renderDirect(double& time, Image& directImage, Image& normalImage, Image& boundingBoxImage);

        void testPixel(int x, int y);

        void testRaytracing(Ray& ray, int depth);

        void setResolution(const int& width, const int& height);

        void rotateCamera(float, float, float);

        void moveCamera(float, float);

        void scaleCamera(float);

        void setupScene(const std::string& scenePath);

        void unloadScene();

        Color getEnvColor(const Vector3& dir) const;

        // Vector3 toneMapping(const Vector3 &radiance) const;
    public:
        Scene scene;
        BVH bvh;
        int curSamples;
        int samples;
        double progress;
        bool isRendering;
        std::string scenePath;

        unsigned short width;
        unsigned short height;
    };
}
