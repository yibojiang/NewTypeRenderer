#include "Mesh.h"

namespace new_type_renderer
{
    MeshFace::MeshFace(shared_ptr<Mesh>& mesh, unsigned int vid1, unsigned int vid2, unsigned int vid3) : m_Mesh(mesh), m_Vid1(vid1), m_Vid2(vid2), m_Vid3(vid3)
    {
    }

    MeshFace::~MeshFace()
    {
    }

    Vector3 MeshFace::GetVertexPosition1() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_Positions[m_Vid1];
    }

    Vector3 MeshFace::GetVertexPosition2() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_Positions[m_Vid2];
    }

    Vector3 MeshFace::GetVertexPosition3() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_Positions[m_Vid3];
    }

    Vector3 MeshFace::GetVertexNormal1() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_VertexNormals[m_Vid1];
    }

    Vector3 MeshFace::GetVertexNormal2() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_VertexNormals[m_Vid2];
    }

    Vector3 MeshFace::GetVertexNormal3() const
    {
        assert(m_Mesh.expired() == false);
        return m_Mesh.lock()->m_VertexNormals[m_Vid3];
    }

    void MeshFace::SetupUVs(Vector2 uv1, Vector2 uv2, Vector2 uv3)
    {
        m_UV1 = uv1;
        m_UV2 = uv2;
        m_UV3 = uv3;
    }

    Vector3 MeshFace::GetHitNormal(const Vector3& hitLocation) const
    {
        Vector3 normal = GetVertexNormal1() + GetVertexNormal2() + GetVertexNormal3();
        return normal.Normalized();
    }

    Intersection MeshFace::Intersect(Ray& r)
    {
        Intersection intersection;
        Vector3 p1 = GetVertexPosition1();
        Vector3 p2 = GetVertexPosition2();
        Vector3 p3 = GetVertexPosition3();

        Vector3 e1 = p2 - p1;
        Vector3 e2 = p3 - p2;

        Vector3 n = e1.Cross(e2);
        // Vector3 n = GetVertexNormal1() + GetVertexNormal2() + GetVertexNormal3();
        n.Normalize();

        double dn = r.dir.Dot(n);
        // Back-face culled
        // if (dn >= 0.0f)
        // {
        //     // ray is parallel to triangle plane
        //     return 0;
        // }

        Vector3 center = (p1 + p2 + p3) / 3;
        double dist = -r.origin.Dot(n) + center.Dot(n);
        double tt = dist / dn;
        Vector3 hit = r.origin + r.dir * tt;

        Vector3 u = p2 - p1;
        Vector3 v = p3 - p1;

        // is I inside T?
        double uu, uv, vv, wu, wv, D;
        uu = u.Dot(u);
        uv = u.Dot(v);
        vv = v.Dot(v);

        Vector3 w = hit - p1;
        wu = w.Dot(u);
        wv = w.Dot(v);
        D = uv * uv - uu * vv;

        // get and test parametric coords
        m_S = (uv * wv - vv * wu) / D;
        if (m_S < 0.0 || m_S > 1.0)
        {
            // I is outside T
            return intersection;
        }

        m_T = (uv * wu - uu * wv) / D;
        if (m_T < 0.0 || (m_S + m_T) > 1.0)
        {
            // I is outside T
            return intersection;
        }

        // r.uv = m_UV1 * (1 - m_S - m_T) + m_UV2 * m_S + m_UV3 * m_T;
        // r.normal = GetHitNormal(hit);
        intersection.m_UV = m_UV1 * (1 - m_S - m_T) + m_UV2 * m_S + m_UV3 * m_T;
        intersection.m_Normal = GetHitNormal(hit);
        intersection.m_HitObject = weak_from_this();
        intersection.m_Distance = tt;
        return intersection;
    }

    void MeshFace::ComputeBounds()
    {
        Vector3 p1 = GetVertexPosition1();
        Vector3 p2 = GetVertexPosition2();
        Vector3 p3 = GetVertexPosition3();

        for (uint8_t i = 0; i < SLABCOUNT; ++i)
        {
            Vector3 slabN = Extents::m_Normals[i];
            double d1 = -p1.Dot(slabN);
            double d2 = -p2.Dot(slabN);
            double d3 = -p3.Dot(slabN);
            m_Bounds.m_DistNear[i] = fmin(d1, fmin(d2, d3));
            m_Bounds.m_DistFar[i] = fmax(d1, fmax(d2, d3));
        }
    }

    Vector3 MeshFace::GetCentriod() const
    {
        Vector3 p1 = GetVertexPosition1();
        Vector3 p2 = GetVertexPosition2();
        Vector3 p3 = GetVertexPosition3();
        return (p1 + p2 + p3) / 3.0;
    }

    void MeshFace::UpdateTransformMatrix(const Matrix4x4& m)
    {
        // Transform is already applied on Mesh, so need to apply it on the face
    }

    Mesh::Mesh()
    {
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::UpdateTransformMatrix(const Matrix4x4& m)
    {
        for (int i = 0; i < m_Positions.size(); i++)
        {
            m_Positions[i] = m.TransformLocation(m_Positions[i]);
        }

        for (int i = 0; i < m_Positions.size(); i++)
        {
            m_VertexNormals[i] = m * m_VertexNormals[i];
        }
    }

    void Mesh::ComputeBounds()
    {
    }

    Vector3 Mesh::GetCentriod() const
    {
        return Vector3{};
    }

    void Mesh::AddFace(shared_ptr<MeshFace>& face)
    {
        m_Faces.push_back(face);
    }

    void Mesh::SmoothVertexNormal()
    {
        // Average the vertex normal on the mesh, https://iquilezles.org/articles/normals/
        for (int i = 0; i < m_Faces.size(); i++)
        {
            auto face = m_Faces[i];
            Vector3 p1 = face->GetVertexPosition1();
            Vector3 p2 = face->GetVertexPosition2();
            Vector3 p3 = face->GetVertexPosition3();

            Vector3 e1 = p2 - p1;
            Vector3 e2 = p3 - p2;

            Vector3 n = e1.Cross(e2);

            m_VertexNormals[face->m_Vid1] = m_VertexNormals[face->m_Vid1] + n;
            m_VertexNormals[face->m_Vid2] = m_VertexNormals[face->m_Vid1] + n;
            m_VertexNormals[face->m_Vid3] = m_VertexNormals[face->m_Vid1] + n;
        }

        for (int i = 0; i < m_VertexNormals.size(); i++)
        {
            m_VertexNormals[i].Normalize();
        }
    }
}
