#include "Bang/Texture2D.h"

#include "Bang/GL.h"
#include "Bang/ImageIO.h"
#include "Bang/Resources.h"

USING_NAMESPACE_BANG

Texture2D::Texture2D() : Texture(GL::TextureTarget::Texture2D)
{
    SetFormat(GL::ColorFormat::RGBA_UByte8);
    CreateEmpty(1, 1);

    SetFilterMode(GL::FilterMode::Bilinear);
    SetWrapMode(GL::WrapMode::ClampToEdge);
}

Texture2D::~Texture2D()
{
}

void Texture2D::CreateEmpty(int width, int height)
{
    Fill(nullptr, width, height, GetColorComp(), GetDataType());
}

void Texture2D::Resize(int width, int height)
{
    if (width != GetWidth() || height != GetHeight())
    {
        CreateEmpty(width, height);
    }
}

void Texture2D::Fill(const Color &fillColor, int width, int height)
{
    Array<Color> inputData = Array<Color>(width * height, fillColor);
    Fill( RCAST<const Byte*>(inputData.Data()), width, height,
          GL::ColorComp::RGBA, GL::DataType::Float);
}

void Texture2D::Fill(const Byte *newData,
                     int width, int height,
                     GL::ColorComp inputDataColorComp,
                     GL::DataType inputDataType)
{
    SetWidth(width);
    SetHeight(height);

    GLId prevBoundId = GL::GetBoundId( GetGLBindTarget() ); // Save state

    Bind();
    GL::TexImage2D(GetTextureTarget(),
                   GetWidth(),
                   GetHeight(),
                   GetFormat(),
                   inputDataColorComp,
                   inputDataType,
                   newData);

    if (newData && GetWidth() > 0 && GetHeight() > 0) { GenerateMipMaps(); }

    GL::Bind(GetGLBindTarget(), prevBoundId); // Restore

    PropagateTextureChanged();
}

void Texture2D::SetAlphaCutoff(float alphaCutoff)
{
    if (alphaCutoff != GetAlphaCutoff())
    {
        m_alphaCutoff = alphaCutoff;
        PropagateTextureChanged();
    }
}

float Texture2D::GetAlphaCutoff() const
{
    return m_alphaCutoff;
}

void Texture2D::ImportXML(const XMLNode &xmlInfo)
{
    Asset::ImportXML(xmlInfo);

    if (xmlInfo.Contains("FilterMode"))
    { SetFilterMode( xmlInfo.Get<GL::FilterMode>("FilterMode") ); }

    if (xmlInfo.Contains("WrapMode"))
    { SetWrapMode( xmlInfo.Get<GL::WrapMode>("WrapMode") ); }

    if (xmlInfo.Contains("AlphaCutoff"))
    { SetAlphaCutoff( xmlInfo.Get<float>("AlphaCutoff") ); }
}

void Texture2D::ExportXML(XMLNode *xmlInfo) const
{
    Asset::ExportXML(xmlInfo);

    xmlInfo->Set("FilterMode", GetFilterMode());
    xmlInfo->Set("WrapMode", GetWrapMode());
    xmlInfo->Set("AlphaCutoff", GetAlphaCutoff());
}

void Texture2D::Import(const Path &imageFilepath)
{
    ImageIO::Import(imageFilepath, this, nullptr);

    Path importFilepath = ImportFilesManager::GetImportFilepath(imageFilepath);
    ImportXMLFromFile(importFilepath);
}

void Texture2D::Import(const Image<Byte> &image)
{
    if (image.GetData())
    {
        SetWidth(image.GetWidth());
        SetHeight(image.GetHeight());

        SetFormat(GL::ColorFormat::RGBA_UByte8);
        Fill(image.GetData(),
             GetWidth(), GetHeight(),
             GL::ColorComp::RGBA,
             GL::DataType::UnsignedByte);
    }
}

GL::BindTarget Texture2D::GetGLBindTarget() const
{
    return GL::BindTarget::Texture2D;
}

