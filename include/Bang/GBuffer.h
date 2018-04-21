#ifndef GBUFFER_H
#define GBUFFER_H

#include "Bang/GL.h"
#include "Bang/Color.h"
#include "Bang/AARect.h"
#include "Bang/Framebuffer.h"

NAMESPACE_BANG_BEGIN

FORWARD class ShaderProgram;

class GBuffer : public Framebuffer
{
public:

    static const GL::Attachment AttColor        = GL::Attachment::Color0;

    // (diffColor.r, diffColor.g, diffColor.b, diffColor.a)
    static const GL::Attachment AttAlbedo      = GL::Attachment::Color1;

    // (normal.x, normal.y, normal.z, 0)
    static const GL::Attachment AttNormal       = GL::Attachment::Color2;

    // (receivesLighting, roughness, metalness, ---)
    // If receivesLighting >  0 ---> receivesShadows
    // If receivesLighting <= 0 ---> not receivesShadows
    static const GL::Attachment AttMisc         = GL::Attachment::Color3;

    static const GL::Attachment AttColorRead    = GL::Attachment::Color4;

    static const GL::Attachment AttDepthStencil = GL::Attachment::DepthStencil;

    GBuffer(int width, int height);
    virtual ~GBuffer();

    void BindAttachmentsForReading(ShaderProgram *sp);

    void ApplyPass(ShaderProgram *sp,
                   bool willReadFromColor = false,
                   const AARect &mask = AARect::NDCRect);

    void PrepareColorReadBuffer(const AARect &readNDCRect = AARect::NDCRect);

    void SetAllDrawBuffers() const override;
    void SetAllDrawBuffersExceptColor();
    void SetColorDrawBuffer();

    void ClearAllBuffersExceptColor();
    void ClearBuffersAndBackground(const Color &backgroundColor);

    static String GetMiscTexName();
    static String GetColorsTexName();
    static String GetAlbedoTexName();
    static String GetNormalsTexName();
    static String GetDepthStencilTexName();

private:
    void RenderViewportPlane();

    friend class GEngine;
};

NAMESPACE_BANG_END

#endif // GBUFFER_H
