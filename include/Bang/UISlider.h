#ifndef UISLIDER_H
#define UISLIDER_H

#include <vector>

#include "Bang/Array.tcc"
#include "Bang/BangDefines.h"
#include "Bang/Color.h"
#include "Bang/Component.h"
#include "Bang/ComponentClassIds.h"
#include "Bang/ComponentMacros.h"
#include "Bang/EventEmitter.h"
#include "Bang/EventEmitter.tcc"
#include "Bang/EventListener.h"
#include "Bang/IEvents.h"
#include "Bang/IEventsValueChanged.h"
#include "Bang/String.h"
#include "Bang/UIFocusable.h"
#include "Bang/UITheme.h"

NAMESPACE_BANG_BEGIN

FORWARD class IEventsValueChanged;
FORWARD class GameObject;
FORWARD class RectTransform;
FORWARD class UIFocusable;
FORWARD class UIImageRenderer;
FORWARD class UIInputNumber;

class UISlider : public Component,
                 public EventListener<IEventsValueChanged>,
                 public EventEmitter<IEventsValueChanged>
{
    COMPONENT_WITH_FAST_DYNAMIC_CAST(UISlider)

public:
    void SetValue(float value);
    void SetMinMaxValues(float minValue, float maxValue);
    void SetValuePercent(float percent);

    float GetValue() const;
    float GetValuePercent() const;

    UIInputNumber *GetInputNumber() const;
    UIImageRenderer *GetGuideRenderer() const;
    UIImageRenderer *GetHandleRenderer() const;
    UIFocusable *GetSliderFocusable() const;
    bool HasFocus() const;

    const Color& GetIdleColor() const;
    const Color& GetOverColor() const;
    const Color& GetPressedColor() const;

private:
    Color m_idleColor    = Color::White;
    Color m_overColor    = UITheme::GetOverColor();
    Color m_pressedColor = UITheme::GetSelectedColor();

    UIImageRenderer *p_guideRenderer  = nullptr;
    UIImageRenderer *p_handleRenderer = nullptr;
    UIFocusable *p_sliderFocusable    = nullptr;
    UIInputNumber *p_inputNumber      = nullptr;

	UISlider();
    virtual ~UISlider() override;

    // Component
    void OnUpdate() override;

    // IEventsValueChanged
    void OnValueChanged(EventEmitter<IEventsValueChanged> *object) override;

    float GetMouseRelativePercent() const;
    void UpdateSliderHandlerFromInputNumberValue();
    RectTransform *GetHandleRectTransform() const;

    static UISlider* CreateInto(GameObject *go);

    friend class GameObjectFactory;
};

NAMESPACE_BANG_END

#endif // UISLIDER_H

