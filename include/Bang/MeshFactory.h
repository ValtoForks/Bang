#ifndef MESHFACTORY_H
#define MESHFACTORY_H

#include "Bang/Set.h"
#include "Bang/Mesh.h"
#include "Bang/Path.h"
#include "Bang/Model.h"
#include "Bang/ResourceHandle.h"

NAMESPACE_BANG_BEGIN

class MeshFactory
{
public:
    static RH<Mesh> GetPlane();
    static RH<Mesh> GetUIPlane();
    static RH<Mesh> GetUIPlaneInvUVY();
    static RH<Mesh> GetUIPlane3x3();
    static RH<Mesh> GetUIPlane3x3InvUVY();
    static RH<Mesh> GetCube();
    static RH<Mesh> GetCylinder();
    static RH<Mesh> GetCapsule();
    static RH<Mesh> GetSphere();
    static RH<Mesh> GetCone();
    static RH<Mesh> GetCamera();

    static RH<Mesh> GetMesh(const String &enginePath);
    static RH<Mesh> GetMesh(const Path &fullPath);

protected:
    MeshFactory() = default;
    virtual ~MeshFactory() = default;

private:
    Set< RH<Model> > m_modelCache;

    static MeshFactory* GetActive();

    friend class Resources;
};

NAMESPACE_BANG_END

#endif // MESHFACTORY_H
