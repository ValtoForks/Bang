#ifndef UIMASK_H
#define UIMASK_H

#include <array>

#include "Bang/Array.h"
#include "Bang/BangDefines.h"
#include "Bang/Component.h"
#include "Bang/ComponentClassIds.h"
#include "Bang/ComponentMacros.h"
#include "Bang/GL.h"
#include "Bang/MetaNode.h"
#include "Bang/RenderPass.h"
#include "Bang/String.h"

NAMESPACE_BANG_BEGIN

class UIMask : public Component
{
    COMPONENT_WITH_FAST_DYNAMIC_CAST(UIMask)

public:
    UIMask();
    virtual ~UIMask() override;

    virtual void OnRender(RenderPass renderPass) override;
    virtual void OnBeforeChildrenRender(RenderPass renderPass) override;
    virtual void OnAfterChildrenRender(RenderPass renderPass) override;

    void SetMasking(bool maskEnabled);
    void SetDrawMask(bool drawMask);

    bool IsMasking() const;
    bool IsDrawMask() const;

    // Serializable
    virtual void ImportMeta(const MetaNode &metaNode) override;
    virtual void ExportMeta(MetaNode *metaNode) const override;

private:
    bool m_masking = true;
    bool m_drawMask = false;
    bool m_restoringStencil = false;

    std::array<bool, 4> m_colorMaskBefore;
    GL::Function  m_stencilFuncBefore;
    GL::StencilOperation m_stencilOpBefore;

    void PrepareStencilToDrawMask();
    void PrepareStencilToDrawChildren();
    void RestoreStencilBuffer(RenderPass renderPass);
};

NAMESPACE_BANG_END

#endif // UIMASK_H
