#include "Bang/CapsuleCollider.h"

#include "Bang/Physics.h"
#include "Bang/MetaNode.h"
#include "Bang/Transform.h"
#include "Bang/GameObject.h"

USING_NAMESPACE_BANG

CapsuleCollider::CapsuleCollider()
{
    SetPhysicsObjectType( PhysicsObject::Type::CAPSULE_COLLIDER );
}

CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::SetRadius(float radius)
{
    if (radius != GetRadius())
    {
        m_radius = radius;
        UpdatePxShape();
    }
}

void CapsuleCollider::SetHeight(float height)
{
    if (height != GetHeight())
    {
        m_height = height;
        UpdatePxShape();
    }
}

void CapsuleCollider::SetAxis(Axis3D axis)
{
    if (axis != GetAxis())
    {
        m_axis = axis;
        UpdatePxShape();
    }
}

Axis3D CapsuleCollider::GetAxis() const
{
    return m_axis;
}

float CapsuleCollider::GetRadius() const
{
    return m_radius;
}

float CapsuleCollider::GetHeight() const
{
    return m_height;
}

float CapsuleCollider::GetScaledHeight() const
{
    Vector3 scale = GetGameObject()->GetTransform()->GetScale();
    return scale.y * GetHeight();
}

float CapsuleCollider::GetScaledRadius() const
{
    Vector3 scale = GetGameObject()->GetTransform()->GetScale();
    float maxScaleXZ = Math::Max(scale.x, scale.z);
    return maxScaleXZ * GetRadius();
}

void CapsuleCollider::CloneInto(ICloneable *clone) const
{
    Collider::CloneInto(clone);

    CapsuleCollider *ccClone = SCAST<CapsuleCollider*>(clone);
    ccClone->SetRadius( GetRadius() );
    ccClone->SetHeight( GetHeight() );
    ccClone->SetAxis( GetAxis() );
}

void CapsuleCollider::ImportMeta(const MetaNode &metaNode)
{
    Collider::ImportMeta(metaNode);

    if (metaNode.Contains("Radius"))
    {
        SetRadius( metaNode.Get<float>("Radius") );
    }

    if (metaNode.Contains("Height"))
    {
        SetHeight( metaNode.Get<float>("Height") );
    }

    if (metaNode.Contains("Axis"))
    {
        SetAxis( SCAST<Axis3D>( metaNode.Get<int>("Axis") ) );
    }

}

void CapsuleCollider::ExportMeta(MetaNode *metaNode) const
{
    Collider::ExportMeta(metaNode);

    metaNode->Set("Radius", GetRadius());
    metaNode->Set("Height", GetHeight());
    metaNode->Set("Axis3D", SCAST<int>(GetAxis()));
}

Quaternion CapsuleCollider::GetInternalRotation() const
{
    switch (GetAxis())
    {
        case Axis3D::X:
            return Quaternion::Identity;
        break;

        case Axis3D::Y:
            return Quaternion::AngleAxis(Math::Pi * 0.5f, Vector3::Forward);
        break;

        case Axis3D::Z:
            return Quaternion::AngleAxis(Math::Pi * 0.5f, Vector3::Up);
        break;
    }
    return Quaternion::Identity;
}

void CapsuleCollider::UpdatePxShape()
{
    Collider::UpdatePxShape();

    if (GetPxShape())
    {
        ASSERT(GetPxRigidBody());
        ASSERT(GetPxShape());

        float scaledRadius = GetScaledRadius();
        float scaledHeight = GetScaledHeight();
        physx::PxCapsuleGeometry capsuleGeometry;
        capsuleGeometry.radius = scaledRadius;
        capsuleGeometry.halfHeight = scaledHeight * 0.5f;
        GetPxShape()->setGeometry(capsuleGeometry);

        physx::PxRigidBodyExt::updateMassAndInertia(*GetPxRigidBody(), 1.0f);
    }
}

