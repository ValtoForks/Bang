#include "Bang/UIInputNumber.h"

#include "Bang/Debug.h"
#include "Bang/Input.h"
#include "Bang/UILabel.h"
#include "Bang/UICanvas.h"
#include "Bang/GameObject.h"
#include "Bang/UIInputText.h"
#include "Bang/RectTransform.h"
#include "Bang/UITextRenderer.h"
#include "Bang/GameObjectFactory.h"

USING_NAMESPACE_BANG

UIInputNumber::UIInputNumber()
{
    CONSTRUCT_CLASS_ID(UIInputNumber)
}

UIInputNumber::~UIInputNumber()
{
}

void UIInputNumber::SetValue(float v)
{
    const float clampedValue = Math::Clamp(v, GetMinValue(), GetMaxValue());
    const uint placesMult = Math::Pow(10u, GetDecimalPlaces());
    const float finalValue = SCAST<float>(
                      Math::Round(clampedValue * placesMult) ) / placesMult;

    if (finalValue != GetValue())
    {
        m_value = finalValue;
        EventEmitter<IEventsValueChanged>::
           PropagateToListeners(&IEventsValueChanged::OnValueChanged, this);
    }

    if (!HasFocus())
    {
        UpdateTextFromValue();
    }
    ChangeTextColorBasedOnMinMax();
}

void UIInputNumber::SetMinMaxValues(float min, float max)
{
    if (max < min)
    {
        Debug_Warn("Max and min are swapped! Correcting them...");
    }

    m_minMaxValues = Vector2(Math::Min(min, max), Math::Max(min, max));
    SetValue( GetValue() );
}

void UIInputNumber::SetDecimalPlaces(uint decimalPlaces)
{
    m_decimalPlaces = decimalPlaces;

    String allowedChars = "0123456789+-";
    if (GetDecimalPlaces() > 0)
    {
        allowedChars += ",.";
    }
    GetInputText()->SetAllowedCharacters(allowedChars);

    SetValue( GetValue() );
}

float UIInputNumber::GetValue() const
{
    return m_value;
}

uint UIInputNumber::GetDecimalPlaces() const
{
    return m_decimalPlaces;
}
void UIInputNumber::UpdateValueFromText()
{
    float value = 0.0f;
        const String &content = GetInputText()->GetText()->GetContent();
    if (!content.IsEmpty())
    {
        std::istringstream iss(content);
        iss >> value;
    }
    SetValue(value);
}

void UIInputNumber::UpdateTextFromValue()
{
    String vStr = String::ToString(GetValue(), GetDecimalPlaces());
    if (GetInputText()->GetText())
    {
        GetInputText()->GetText()->SetContent(vStr);
    }
}

void UIInputNumber::ChangeTextColorBasedOnMinMax()
{
    // Colorize text based on the textValue
    // (which can be different from GetValue())
    float textValue = String::ToFloat(GetInputText()->GetText()->GetContent());
    bool isOutOfRange = (textValue < GetMinValue() || textValue > GetMaxValue());
    Color textColor = isOutOfRange ? Color::Red : Color::Black;
    if (GetInputText()->GetText())
    {
        GetInputText()->GetText()->SetTextColor(textColor);
    }
}

UIInputText *UIInputNumber::GetInputText() const
{
    return p_inputText;
}

float UIInputNumber::GetMinValue() const
{
    return GetMinMaxValues()[0];
}

float UIInputNumber::GetMaxValue() const
{
    return GetMinMaxValues()[1];
}

const Vector2 &UIInputNumber::GetMinMaxValues() const
{
    return m_minMaxValues;
}
bool UIInputNumber::HasFocus() const
{
    return GetInputText()->GetLabel()->GetFocusable() ?
                GetInputText()->GetLabel()->GetFocusable()->HasFocus() :
                false;
}

UIEventResult UIInputNumber::OnUIEvent(UIFocusable*, const UIEvent &event)
{
    switch (event.type)
    {
        case UIEvent::Type::FOCUS_TAKEN:
            return UIEventResult::INTERCEPT;
        break;

        case UIEvent::Type::FOCUS_LOST:
            UpdateTextFromValue();
            return UIEventResult::INTERCEPT;
        break;

        case UIEvent::Type::KEY_DOWN:
            switch (event.key.key)
            {
                case Key::ENTER:
                    UICanvas::GetActive(this)->SetFocus(nullptr);
                    return UIEventResult::INTERCEPT;
                break;

                case Key::UP:
                case Key::DOWN:
                {
                    float increment = (event.key.key == Key::DOWN ? -1 : 1);
                    SetValue( GetValue() + increment );
                    UpdateTextFromValue();
                    GetInputText()->GetLabel()->SelectAll();
                    return UIEventResult::INTERCEPT;
                }
                break;

                default:
                break;
            }
        break;

        default:
        break;
    }
    return UIEventResult::IGNORE;
}

void UIInputNumber::OnValueChanged(EventEmitter<IEventsValueChanged> *object)
{
    ASSERT(object == GetInputText());
    UpdateValueFromText();
}

UIInputNumber *UIInputNumber::CreateInto(GameObject *go)
{
    REQUIRE_COMPONENT(go, RectTransform);

    GameObjectFactory::CreateUIInputTextInto(go);

    UIInputNumber *inputNumber = go->AddComponent<UIInputNumber>();

    UIInputText *inputText = go->GetComponent<UIInputText>();
    inputText->SetAllowedCharacters("0123456789.,-+");
    inputText->GetText()->SetHorizontalAlign(HorizontalAlignment::LEFT);
    inputText->GetText()->SetTextSize(12);

    inputText->EventEmitter<IEventsValueChanged>::RegisterListener(inputNumber);
    inputText->EventEmitter<IEventsFocus>::RegisterListener(inputNumber);

    inputNumber->p_inputText = inputText;

    inputNumber->SetValue(0.0f);

    return inputNumber;
}

