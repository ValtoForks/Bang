#include "Bang/UIInputNumber.h"

#include "Bang/Debug.h"
#include "Bang/Input.h"
#include "Bang/UICanvas.h"
#include "Bang/GameObject.h"
#include "Bang/UIInputText.h"
#include "Bang/RectTransform.h"
#include "Bang/UITextRenderer.h"
#include "Bang/GameObjectFactory.h"

USING_NAMESPACE_BANG

UIInputNumber::UIInputNumber()
{
}

UIInputNumber::~UIInputNumber()
{
}

void UIInputNumber::OnStart()
{
    UIInputText *inputText = GetGameObject()->GetComponent<UIInputText>();
    inputText->EventEmitter<IFocusListener>::RegisterListener(this);
    inputText->EventEmitter<IValueChangedListener>::RegisterListener(this);
}

void UIInputNumber::OnUpdate()
{
    Component::OnUpdate();

    if (HasFocus())
    {
        if (Input::GetKeyDown(Key::Enter))
        {
            UICanvas::GetActive(this)->SetFocus(nullptr);
        }


        float increment = 0.0f;
        if (Input::GetKeyDownRepeat(Key::Up))   { increment =  1.0f; }
        if (Input::GetKeyDownRepeat(Key::Down)) { increment = -1.0f; }
        if (increment != 0.0f)
        {
            SetValue( GetValue() + increment );
            UpdateTextFromValue();
        }
    }
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
        EventEmitter<IValueChangedListener>::
           PropagateToListeners(&IValueChangedListener::OnValueChanged, this);
    }

    if (!HasFocus()) { UpdateTextFromValue(); }
    ChangeTextColorBasedOnMinMax();
}

void UIInputNumber::SetMinMaxValues(float min, float max)
{
    if (max < min) { Debug_Warn("Max and min are swapped! Correcting them..."); }

    m_minMaxValues = Vector2(Math::Min(min, max), Math::Max(min, max));
    SetValue( GetValue() );
}

void UIInputNumber::SetDecimalPlaces(uint decimalPlaces)
{
    m_decimalPlaces = decimalPlaces;

    String allowedChars = "0123456789+-";
    if (GetDecimalPlaces() > 0) { allowedChars += ",."; }
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

void UIInputNumber::OnFocusTaken(IFocusable *focusable)
{
    IFocusListener::OnFocusTaken(focusable);
    m_hasFocus = true;
}

void UIInputNumber::OnFocusLost(IFocusable *focusable)
{
    IFocusListener::OnFocusLost(focusable);
    m_hasFocus = false;
    SetValue( String::ToFloat( GetInputText()->GetText()->GetContent() ) );
}

void UIInputNumber::UpdateValueFromText()
{
    const String &content = GetInputText()->GetText()->GetContent();
    float value = 0.0f;
    if (!content.IsEmpty()) { std::istringstream iss(content); iss >> value; }
    SetValue(value);
}

void UIInputNumber::UpdateTextFromValue()
{
    String vStr = String::ToString(GetValue(), GetDecimalPlaces());
    GetInputText()->GetText()->SetContent(vStr);
}

void UIInputNumber::ChangeTextColorBasedOnMinMax()
{
    // Colorize text based on the textValue
    // (which can be different from GetValue())
    float textValue = String::ToFloat(GetInputText()->GetText()->GetContent());
    bool isOutOfRange = (textValue < GetMinValue() || textValue > GetMaxValue());
    Color textColor = isOutOfRange ? Color::Red : Color::Black;
    GetInputText()->GetText()->SetTextColor(textColor);
}

UIInputText *UIInputNumber::GetInputText() const { return p_inputText; }

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
bool UIInputNumber::HasFocus() const { return m_hasFocus; }

void UIInputNumber::OnValueChanged(Object *object)
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
    inputText->GetText()->SetHorizontalAlign(HorizontalAlignment::Left);
    inputText->GetText()->SetTextSize(12);

    inputNumber->p_inputText = inputText;

    inputNumber->SetValue(0.0f);

    return inputNumber;
}

