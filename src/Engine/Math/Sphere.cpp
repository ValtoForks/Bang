#include "Bang/Sphere.h"

#include "Bang/AABox.h"
#include "Bang/Array.h"
#include "Bang/Array.tcc"
#include "Bang/Math.h"
#include "Bang/Vector3.h"

using namespace Bang;

Sphere::Sphere(float radius)
{
    SetRadius(radius);
}

Sphere::Sphere(const Vector3 &center, float radius)
{
    SetCenter(center);
    SetRadius(radius);
}

void Sphere::SetCenter(const Vector3 &center)
{
    m_center = center;
}
void Sphere::SetRadius(float radius)
{
    m_radius = radius;
}

Array<Vector3> Sphere::GetPoints() const
{
    Array<Vector3> points;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            for (int k = -1; k <= 1; ++k)
            {
                Vector3 dir = Vector3(i, j, k).NormalizedSafe();
                points.PushBack(m_center + m_radius * dir);
            }
        }
    }
    return points;
}
float Sphere::GetDiameter() const
{
    return 2 * m_radius;
}
float Sphere::GetArea() const
{
    return 2 * Math::Pi * m_radius;
}
float Sphere::GetVolume() const
{
    return Math::Pi * m_radius * m_radius;
}
const Vector3 &Sphere::GetCenter() const
{
    return m_center;
}
float Sphere::GetRadius() const
{
    return m_radius;
}

bool Sphere::Contains(const Vector3 &point) const
{
    return Vector3::SqDistance(point, GetCenter()) < GetRadius() * GetRadius();
}

bool Sphere::CheckCollision(const Sphere &sphere) const
{
    return Vector3::SqDistance(GetCenter(), sphere.GetCenter()) <
           (GetRadius() * GetRadius() +
            sphere.GetRadius() * sphere.GetRadius());
}

bool Sphere::CheckCollision(const AABox &aabox,
                            Vector3 *point,
                            Vector3 *normal) const
{
    return aabox.CheckCollision(*this, point, normal);
}

void Sphere::FillFromBox(const AABox &box)
{
    m_radius = box.GetDiagonal().Length() / 2.0f;
    m_center = box.GetCenter();
}

Sphere Sphere::FromBox(const AABox &box)
{
    Sphere s;
    s.FillFromBox(box);
    return s;
}
