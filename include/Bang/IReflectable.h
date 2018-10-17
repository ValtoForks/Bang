#ifndef IREFLECTABLE_H
#define IREFLECTABLE_H

#include "Bang/BPReflectedStruct.h"
#include "Bang/BangDefines.h"

namespace Bang
{
#define BANG_REFLECT_VARIABLE(...)
#define BANG_REFLECT_CLASS(...)
#define BANG_REFLECT_STRUCT(...)
#define BANG_REFLECT_DEFINITIONS(...)

class BPReflectedStruct;

class IReflectable
{
public:
    const BPReflectedStruct &GetReflectionInfo() const;

protected:
    IReflectable() = default;
    virtual ~IReflectable() = default;

    virtual void Reflect() const;
    BPReflectedStruct *GetReflectionInfoPtr() const;

private:
    mutable bool m_alreadyReflected = false;
    mutable BPReflectedStruct m_reflectionInfo;
};
}

#endif  // IREFLECTABLE_H
