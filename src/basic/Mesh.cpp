#include "Mesh.h"

namespace new_type_renderer
{
    MeshFace::MeshFace(shared_ptr<Mesh>& mesh, unsigned int vid1, unsigned int vid2, unsigned int vid3) : m_Mesh(mesh), m_Vid1(vid1), m_Vid2(vid2), m_Vid3(vid3)
    {
    }

    // Triangle::Triangle(Vector3 p1, Vector3 p2, Vector3 p3)
    // {
    //     m_P1 = p1;
    //     m_P2 = p2;
    //     m_P3 = p3;
    //     auto u = m_P2 - m_P1;
    //     auto v = m_P3 - m_P1;
    //     m_Normal = u.Cross(v).Normalize();
    //     isMesh = false;
    // }

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

    float MeshFace::Intersect(Ray& r)
    {
        Vector3 p1 = GetVertexPosition1();
        Vector3 p2 = GetVertexPosition2();
        Vector3 p3 = GetVertexPosition3();

        Vector3 e1 = p2 - p1;
        Vector3 e2 = p3 - p2;

        Vector3 n = e1.Cross(e2);
        // Vector3 n = GetVertexNormal1() + GetVertexNormal2() + GetVertexNormal3();
        // n.Normalize();

        float dn = r.dir.Dot(n);
        // Back-face culled
        if (dn < 0.0f)
        {
            // ray is  parallel to triangle plane
            return 0;
        }

        Vector3 center = (p1 + p2 + p3) / 3;
        double dist = -r.origin.Dot(n) + center.Dot(n);
        double tt = dist / dn;
        Vector3 hit = r.origin + r.dir * tt;

        auto u = p2 - p1;
        auto v = p3 - p1;

        // is I inside T?
        float uu, uv, vv, wu, wv, D;
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
            return 0;
        }

        m_T = (uv * wu - uu * wv) / D;
        if (m_T < 0.0 || (m_S + m_T) > 1.0)
        {
            // I is outside T
            return 0;
        }

        r.uv = m_UV1 * (1 - m_S - m_T) + m_UV2 * m_S + m_UV3 * m_T;

        r.normal = GetHitNormal(hit);
        return tt;
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
            bounds.m_DistNear[i] = fmin(d1, fmin(d2, d3));
            bounds.m_DistFar[i] = fmax(d1, fmax(d2, d3));
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

        // Vector3 p1 = GetVertexPosition1();
        // Vector3 p2 = GetVertexPosition2();
        // Vector3 p3 = GetVertexPosition3();
        //
        // Vector4 vp1(p1, 1.0);
        // Vector4 vp2(p2, 1.0);
        // Vector4 vp3(p3, 1.0);
        //
        // vp1 = m * vp1;
        // vp2 = m * vp2;
        // vp3 = m * vp3;
        //
        // p1 = Vector3(vp1.x, vp1.y, vp1.z);
        // p2 = Vector3(vp2.x, vp2.y, vp2.z);
        // p3 = Vector3(vp3.x, vp3.y, vp3.z);
        //
        // m_VertexNormal1 = m * m_VertexNormal1;
        // m_VertexNormal2 = m * m_VertexNormal2;
        // m_VertexNormal3 = m * m_VertexNormal3;
        //
        // m_Normal = m_Normal * m_VertexNormal3;
    }

    Mesh::Mesh()
    {
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::UpdateTransformMatrix(const Matrix4x4& m)
    {
        // for (uint32_t i = 0; i < m_Faces.size(); ++i)
        // {
            // Vector4 p1(m_Faces[i]->m_P1, 1.0);
            // Vector4 p2(m_Faces[i]->m_P2, 1.0);
            // Vector4 p3(m_Faces[i]->m_P3, 1.0);
            //
            // p1 = m * p1;
            // p2 = m * p2;
            // p3 = m * p3;
            //
            // m_Faces[i]->m_P1 = Vector3(p1.x, p1.y, p1.z);
            // m_Faces[i]->m_P2 = Vector3(p2.x, p2.y, p2.z);
            // m_Faces[i]->m_P3 = Vector3(p3.x, p3.y, p3.z);
            
            // m_Faces[i]->m_VertexNormal1 = m * m_Faces[i]->m_VertexNormal1;
            // m_Faces[i]->m_VertexNormal2 = m * m_Faces[i]->m_VertexNormal2;
            // m_Faces[i]->m_VertexNormal3 = m * m_Faces[i]->m_VertexNormal3;
            
            // m_Faces[i]->m_Normal = m * m_Faces[i]->m_Normal;
        // }
        for (int i = 0; i < m_Positions.size(); i++)
        {
            m_Positions[i] = m * m_Positions[i];
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
