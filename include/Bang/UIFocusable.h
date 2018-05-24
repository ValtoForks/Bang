#ifndef UIFOCUSABLE_H
#define UIFOCUSABLE_H

#include "Bang/Component.h"
#include "Bang/IFocusable.h"

NAMESPACE_BANG_BEGIN

class UIFocusable : public Component,
                    public IFocusable
{
    COMPONENT(UIFocusable);

protected:
    // Component
    virtual bool CanBeRepeatedInGameObject() const override;

private:
    UIFocusable();
    virtual ~UIFocusable();
};

NAMESPACE_BANG_END

#endif // UIFOCUSABLE_H

