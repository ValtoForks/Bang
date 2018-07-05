#include "Bang/Texture2D.h"

#include "Bang/GL.h"
#include "Bang/ImageIO.h"
#include "Bang/Resources.h"

USING_NAMESPACE_BANG

Texture2D::Texture2D() : Texture(GL::TextureTarget::TEXTURE_2D)
{
    SetFormat(GL::ColorFormat::RGBA8);
    CreateEmpty(1, 1);

    SetFilterMode(GL::FilterMode::BILINEAR);
    SetWrapMode(GL::WrapMode::CLAMP_TO_EDGE);
}

Texture2D::~Texture2D()
{
}

void Texture2D::OnFormatChanged()
{
    Texture::OnFormatChanged();

    if (GetWidth() >= 1 && GetHeight() >= 1 && GetResourceFilepath().IsFile())
    {
        Imageb img;
        ImageIO::Import(GetResourceFilepath(), &img);
        Import(img);
    }
}

void Texture2D::CreateEmpty(const Vector2i &size)
{
    Fill(nullptr, size.x, size.y, GetColorComp(), GetDataType());
}

void Texture2D::Resize(const Vector2i &size)
{
    if (size != GetSize())
    {
        CreateEmpty(size.x, size.y);
    }
}

void Texture2D::Fill(const Color &fillColor, int width, int height)
{
    Array<Color> inputData = Array<Color>(width * height, fillColor);
    Fill(RCAST<const Byte*>(inputData.Data()), width, height,
         GL::ColorComp::RGBA, GL::DataType::FLOAT);
}

void Texture2D::Fill(const Byte *newData,
                     int width, int height,
                     GL::ColorComp inputDataColorComp,
                     GL::DataType inputDataType)
{
    SetWidth(width);
    SetHeight(height);

    GL::Push( GetGLBindTarget() );

    Bind();
    GL::TexImage2D(GetTextureTarget(),
                   GetWidth(),
                   GetHeight(),
                   GetFormat(),
                   inputDataColorComp,
                   inputDataType,
                   newData);

    if (newData && GetWidth() > 0 && GetHeight() > 0) { GenerateMipMaps(); }

    GL::Pop( GetGLBindTarget() );

    PropagateResourceChanged();
}

void Texture2D::SetAlphaCutoff(float alphaCutoff)
{
    if (alphaCutoff != GetAlphaCutoff())
    {
        m_alphaCutoff = alphaCutoff;
        PropagateResourceChanged();
    }
}

float Texture2D::GetAlphaCutoff() const
{
    return m_alphaCutoff;
}

const Imageb &Texture2D::GetImage() const
{
    return m_image;
}

void Texture2D::ImportXML(const XMLNode &xmlInfo)
{
    Asset::ImportXML(xmlInfo);

    if (xmlInfo.Contains("Format"))
    { SetFormat( xmlInfo.Get<GL::ColorFormat>("Format") ); }

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

    xmlInfo->Set("Format", GetFormat());
    xmlInfo->Set("FilterMode", GetFilterMode());
    xmlInfo->Set("WrapMode", GetWrapMode());
    xmlInfo->Set("AlphaCutoff", GetAlphaCutoff());
}

void Texture2D::Import(const Path &imageFilepath)
{
    ImageIO::Import(imageFilepath, &m_image, this, nullptr);

    Path importFilepath = ImportFilesManager::GetImportFilepath(imageFilepath);
    ImportXMLFromFile(importFilepath);
}

void Texture2D::Import(const Image<Byte> &image)
{
    if (image.GetData())
    {
        m_image = image;

        SetWidth(image.GetWidth());
        SetHeight(image.GetHeight());

        Fill(image.GetData(),
             GetWidth(), GetHeight(),
             GL::ColorComp::RGBA,
             GL::DataType::UNSIGNED_BYTE);
    }
}

GL::BindTarget Texture2D::GetGLBindTarget() const
{
    return GL::BindTarget::TEXTURE_2D;
}

