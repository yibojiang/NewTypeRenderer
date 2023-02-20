#pragma once
#include <cassert>

#include "Shape.h"
#include "math/Matrix.h"
#include "math/Vector.h"

namespace new_type_renderer
{
    class Mesh;

    class MeshFace : public Shape, public enable_shared_from_this<MeshFace>
    {
    public:
        MeshFace(shared_ptr<Mesh>& mesh, unsigned int vid1, unsigned int vid2, unsigned int vid3);

        ~MeshFace() override;

        Vector3 GetVertexPosition1() const;

        Vector3 GetVertexPosition2() const;

        Vector3 GetVertexPosition3() const;

        Vector3 GetVertexNormal1() const;

        Vector3 GetVertexNormal2() const;

        Vector3 GetVertexNormal3() const;

        void SetVertexNormals(Vector3 n1, Vector3 n2, Vector3 n3);

        void SetupUVs(Vector2 uv1, Vector2 uv2, Vector2 uv3);

        void SetNormal(const Vector3& normal);

        Vector3 GetHitNormal(const Vector3& hitLocation) const;

        Intersection Intersect(Ray& r) override;

        void ComputeBounds() override;

        Vector3 GetCentriod() const override;

        void UpdateTransformMatrix(const Matrix4x4& m) override;

    public:
        // UV coordinates for each vertex
        Vector2 m_UV1{};
        Vector2 m_UV2{};
        Vector2 m_UV3{};

        float m_S{ 0.0f };
        float m_T{ 0.0f };

        // Vertex inddex
        unsigned int m_Vid1{ 0 };
        unsigned int m_Vid2{ 0 };
        unsigned int m_Vid3{ 0 };

        weak_ptr<Mesh> m_Mesh{};
    };

    class Mesh : public Shape, public enable_shared_from_this<Mesh>
    {
    public:
        Mesh();

        ~Mesh() override;

        void UpdateTransformMatrix(const Matrix4x4& m) override;

        void ComputeBounds() override;

        Vector3 GetCentriod() const override;

        void AddFace(shared_ptr<MeshFace>& face);

        void SmoothVertexNormal();

    public:
        std::vector<shared_ptr<MeshFace>> m_Faces{};

        // The vertex normal is calculated from the vertex position in SmoothVertexNormal()
        std::vector<Vector3> m_VertexNormals{};

        std::vector<Vector3> m_Positions{};

        std::vector<int> m_Indices{};
    };
}
