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

#include "BVH.h"
#include "basic/Image.h"
#include "basic/Scene.h"
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
        Raytracer(unsigned width, unsigned height, int samples);

        ~Raytracer();

        Color Tracing(Ray& ray, int depth, int E);
        
        void RenderIndirectProgressive(Color* colorArray, bool& abort, bool& restart, int& samples);

        void RenderIndirect(float& time, Image& image);

        void RenderDirect(float& time, Image& directImage, Image& normalImage, Image& boundingBoxImage);

        void TestPixel(int x, int y);

        void TestRaytracing(Ray& ray, int depth);

        void SetResolution(const int& width, const int& height);

        void RotateCamera(float, float, float);

        void MoveCamera(float, float);

        void ScaleCamera(float);

        void UnloadScene();

        Color GetEnvColor(const Vector3& dir) const;

        // Vector3 ToneMapping(const Vector3 &radiance) const;
    public:
        Scene m_Scene;

        int m_CurrentSamples;

        int m_Samples;

        double m_Progress;

        bool is_rendering;

        std::string m_ScenePath;

        unsigned short m_Width;

        unsigned short m_Height;
    };
}
