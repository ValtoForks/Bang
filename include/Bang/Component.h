#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>

#include "Bang/Array.tcc"
#include "Bang/BangDefines.h"
#include "Bang/ComponentClassIds.h"
#include "Bang/ComponentFactory.h"
#include "Bang/ComponentMacros.h"
#include "Bang/EventEmitter.h"
#include "Bang/EventListener.tcc"
#include "Bang/FastDynamicCast.h"
#include "Bang/IEventsComponentChangeGameObject.h"
#include "Bang/IEventsDestroy.h"
#include "Bang/IToString.h"
#include "Bang/MetaNode.h"
#include "Bang/Object.h"
#include "Bang/RenderPass.h"
#include "Bang/Serializable.h"
#include "Bang/String.h"

NAMESPACE_BANG_BEGIN

FORWARD class Collider;
FORWARD class Collision;
FORWARD class GameObject;
FORWARD class ICloneable;
FORWARD class IEventsComponentChangeGameObject;

class Component : public Object,
                  public IToString,
                  public EventEmitter<IEventsComponentChangeGameObject>
{
    SERIALIZABLE(Component)
    CLONEABLE_COMPONENT(Component)
    SET_CLASS_ID_AS_ROOT()

public:
    template <class T, class... Args>
    static T* Create(const Args&... args) { return new T(args...); }
    static void Destroy(Component *component);
    static void DestroyImmediate(Component *component);

    void SetGameObject(GameObject *gameObject);

    GameObject *GetGameObject() const;

    // ICloneable
    virtual void CloneInto(ICloneable *clone) const override;

    // IToString
    virtual String ToString() const override;

    // Serializable
    virtual void ImportMeta(const MetaNode &metaNode) override;
    virtual void ExportMeta(MetaNode *metaNode) const override;

protected:
    Component();
    virtual ~Component() override;

    virtual void OnPreStart() override;
    virtual void OnStart() override;
    virtual void OnUpdate();
    virtual void OnAfterChildrenUpdate();
    virtual void OnPostUpdate();
    virtual void OnBeforeRender();
    virtual void OnBeforeChildrenRender(RenderPass renderPass);
    virtual void OnRender(RenderPass renderPass);
    virtual void OnAfterChildrenRender(RenderPass renderPass);
    virtual void OnDestroy() override;

    void Update();
    void AfterChildrenUpdate();
    void PostUpdate();
    void BeforeRender();
    void BeforeChildrenRender(RenderPass renderPass);
    void Render(RenderPass renderPass);
    void AfterChildrenRender(RenderPass renderPass);

    virtual void OnGameObjectChanged(GameObject *previousGameObject,
                                     GameObject *newGameObject);
    virtual bool CanBeRepeatedInGameObject() const;

    // Object
    bool CalculateEnabledRecursively() const override;

private:
    GameObject *p_gameObject = nullptr;

    void SetGameObjectForced(GameObject *gameObject);

    friend class GameObject;
    friend class PxSceneContainer;
};

NAMESPACE_BANG_END

#endif // COMPONENT_H
