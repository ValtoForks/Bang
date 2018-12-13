#include "Bang/RigidBody.h"

#include <istream>

#include "Bang/Debug.h"
#include "Bang/FastDynamicCast.h"
#include "Bang/GameObject.h"
#include "Bang/MetaNode.h"
#include "Bang/MetaNode.tcc"
#include "Bang/Physics.h"
#include "Bang/StreamOperators.h"
#include "PxActor.h"
#include "PxRigidBody.h"
#include "extensions/PxRigidBodyExt.h"

namespace Bang
{
class ICloneable;
}

using namespace Bang;
using namespace physx;

RigidBody::RigidBody()
{
    CONSTRUCT_CLASS_ID(RigidBody)
    SetPhysicsObjectType(PhysicsObject::Type::RIGIDBODY);

    // Create pxActor
    if (Physics *ph = Physics::GetInstance())
    {
        SetPxRigidActor(ph->CreateNewPxRigidActor(false));
        SetIsKinematic(false);
    }
}

RigidBody::~RigidBody()
{
}

void RigidBody::SetMass(float mass)
{
    if (mass != GetMass())
    {
        m_mass = mass;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::SetDrag(float drag)
{
    if (drag != GetDrag())
    {
        m_drag = drag;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::SetAngularDrag(float angularDrag)
{
    if (angularDrag != GetAngularDrag())
    {
        m_angularDrag = angularDrag;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::SetUseGravity(bool useGravity)
{
    if (useGravity != GetUseGravity())
    {
        m_useGravity = useGravity;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::SetIsKinematic(bool isKinematic)
{
    if (isKinematic != GetIsKinematic())
    {
        m_isKinematic = isKinematic;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::SetLinearVelocity(const Vector3 &linearVelocity)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->setLinearVelocity(
            Physics::GetPxVec3FromVector3(linearVelocity));
    }
}

void RigidBody::SetAngularVelocity(const Vector3 &angularVelocity)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->setAngularVelocity(
            Physics::GetPxVec3FromVector3(angularVelocity));
    }
}

void RigidBody::SetMaxAngularVelocity(float maxAngularVelocity)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->setMaxAngularVelocity(maxAngularVelocity);
    }
}

void RigidBody::SetConstraints(const RigidBodyConstraints &constraints)
{
    if (constraints != GetConstraints())
    {
        m_constraints = constraints;
        UpdatePxRigidActorValues();
    }
}

void RigidBody::AddForce(const Vector3 &force, ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->addForce(
            Physics::GetPxVec3FromVector3(force),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::AddTorque(const Vector3 &torque, ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->addTorque(
            Physics::GetPxVec3FromVector3(torque),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::AddForceAtPos(const Vector3 &force,
                              const Vector3 &pos,
                              ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        physx::PxRigidBodyExt::addForceAtPos(
            *GetPxRigidDynamic(),
            Physics::GetPxVec3FromVector3(force),
            Physics::GetPxVec3FromVector3(pos),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::AddForceAtLocalPos(const Vector3 &force,
                                   const Vector3 &pos,
                                   ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        physx::PxRigidBodyExt::addForceAtLocalPos(
            *GetPxRigidDynamic(),
            Physics::GetPxVec3FromVector3(force),
            Physics::GetPxVec3FromVector3(pos),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::AddLocalForceAtPos(const Vector3 &force,
                                   const Vector3 &pos,
                                   ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        physx::PxRigidBodyExt::addLocalForceAtPos(
            *GetPxRigidDynamic(),
            Physics::GetPxVec3FromVector3(force),
            Physics::GetPxVec3FromVector3(pos),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::AddLocalForceAtLocalPos(const Vector3 &force,
                                        const Vector3 &pos,
                                        ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        physx::PxRigidBodyExt::addLocalForceAtLocalPos(
            *GetPxRigidDynamic(),
            Physics::GetPxVec3FromVector3(force),
            Physics::GetPxVec3FromVector3(pos),
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::ClearForce(ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->clearForce(
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

void RigidBody::ClearTorque(ForceMode forceMode)
{
    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->clearTorque(
            SCAST<physx::PxForceMode::Enum>(forceMode));
    }
}

float RigidBody::GetMass() const
{
    return m_mass;
}

float RigidBody::GetDrag() const
{
    return m_drag;
}

float RigidBody::GetAngularDrag() const
{
    return m_angularDrag;
}

bool RigidBody::GetUseGravity() const
{
    return m_useGravity;
}

bool RigidBody::GetIsKinematic() const
{
    return m_isKinematic;
}

Vector3 RigidBody::GetLinearVelocity() const
{
    return GetPxRigidDynamic() ? Physics::GetVector3FromPxVec3(
                                     GetPxRigidDynamic()->getLinearVelocity())
                               : Vector3::Zero();
}

Vector3 RigidBody::GetAngularVelocity() const
{
    return GetPxRigidDynamic() ? Physics::GetVector3FromPxVec3(
                                     GetPxRigidDynamic()->getAngularVelocity())
                               : Vector3::Zero();
}

float RigidBody::GetMaxAngularVelocity() const
{
    return GetPxRigidDynamic() ? GetPxRigidDynamic()->getMaxAngularVelocity()
                               : 0.0f;
}

const RigidBodyConstraints &RigidBody::GetConstraints() const
{
    return m_constraints;
}

void RigidBody::CloneInto(ICloneable *clone, bool cloneGUID) const
{
    Component::CloneInto(clone, cloneGUID);

    RigidBody *rbClone = SCAST<RigidBody *>(clone);
    rbClone->SetMass(GetMass());
    rbClone->SetDrag(GetDrag());
    rbClone->SetAngularDrag(GetAngularDrag());
    rbClone->SetUseGravity(GetUseGravity());
    rbClone->SetIsKinematic(GetIsKinematic());
    rbClone->SetConstraints(GetConstraints());
}

void RigidBody::ImportMeta(const MetaNode &metaNode)
{
    Component::ImportMeta(metaNode);

    if (metaNode.Contains("Mass"))
    {
        SetMass(metaNode.Get<float>("Mass"));
    }

    if (metaNode.Contains("Drag"))
    {
        SetDrag(metaNode.Get<float>("Drag"));
    }

    if (metaNode.Contains("AngularDrag"))
    {
        SetAngularDrag(metaNode.Get<float>("AngularDrag"));
    }

    if (metaNode.Contains("UseGravity"))
    {
        SetUseGravity(metaNode.Get<bool>("UseGravity"));
    }

    if (metaNode.Contains("IsKinematic"))
    {
        SetIsKinematic(metaNode.Get<bool>("IsKinematic"));
    }

    if (metaNode.Contains("Constraints"))
    {
        SetConstraints(
            SCAST<RigidBodyConstraints>(metaNode.Get<int>("Constraints")));
    }
}

void RigidBody::ExportMeta(MetaNode *metaNode) const
{
    Component::ExportMeta(metaNode);

    metaNode->Set("Mass", GetMass());
    metaNode->Set("Drag", GetDrag());
    metaNode->Set("AngularDrag", GetAngularDrag());
    metaNode->Set("UseGravity", GetUseGravity());
    metaNode->Set("IsKinematic", GetIsKinematic());
    metaNode->Set("Constraints", GetConstraints().GetValue());
}

void RigidBody::UpdatePxRigidActorValues()
{
    if (GetPxRigidActor())
    {
        GetPxRigidActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY,
                                        !GetUseGravity());
    }

    if (GetPxRigidDynamic())
    {
        GetPxRigidDynamic()->setRigidBodyFlag(
            physx::PxRigidBodyFlag::eKINEMATIC, GetIsKinematic());
        GetPxRigidDynamic()->setMass(GetMass());
        GetPxRigidDynamic()->setRigidDynamicLockFlags(
            SCAST<physx::PxRigidDynamicLockFlags>(GetConstraints().GetValue()));
        GetPxRigidDynamic()->setLinearDamping(GetDrag());
        GetPxRigidDynamic()->setAngularDamping(GetAngularDrag());
    }
}

void RigidBody::OnPxRigidActorChanged(PxRigidActor *prevPxRigidActor,
                                      PxRigidActor *newPxRigidActor)
{
    BANG_UNUSED_2(prevPxRigidActor, newPxRigidActor);
    UpdatePxRigidActorValues();
}
