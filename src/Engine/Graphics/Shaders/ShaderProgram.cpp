#include "Bang/ShaderProgram.h"

#include <ostream>
#include <unordered_map>
#include <utility>

#include "Bang/Assert.h"
#include "Bang/Assets.h"
#include "Bang/Assets.tcc"
#include "Bang/Color.h"
#include "Bang/Debug.h"
#include "Bang/EventEmitter.h"
#include "Bang/EventListener.tcc"
#include "Bang/File.h"
#include "Bang/GL.h"
#include "Bang/GLUniforms.h"
#include "Bang/GLUniforms.tcc"
#include "Bang/IEventsAsset.h"
#include "Bang/IEventsDestroy.h"
#include "Bang/Matrix3.h"
#include "Bang/Matrix3.tcc"
#include "Bang/Matrix4.h"
#include "Bang/Matrix4.tcc"
#include "Bang/Path.h"
#include "Bang/Shader.h"
#include "Bang/ShaderPreprocessor.h"
#include "Bang/StreamOperators.h"
#include "Bang/Texture.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureCubeMap.h"
#include "Bang/TextureFactory.h"
#include "Bang/TextureUnitManager.h"
#include "Bang/UMap.tcc"
#include "Bang/USet.h"
#include "Bang/USet.tcc"

using namespace Bang;

ShaderProgram::ShaderProgram()
{
    m_idGL = GL::CreateProgram();
}

ShaderProgram::ShaderProgram(Shader *vShader, Shader *fShader) : ShaderProgram()
{
    Load(vShader, fShader);
}

ShaderProgram::ShaderProgram(Shader *vShader, Shader *gShader, Shader *fShader)
{
    Load(vShader, gShader, fShader);
}

ShaderProgram::ShaderProgram(const Path &vShaderPath, const Path &fShaderPath)
    : ShaderProgram()
{
    Load(vShaderPath, fShaderPath);
}

ShaderProgram::ShaderProgram(const Path &vShaderPath,
                             const Path &gShaderPath,
                             const Path &fShaderPath)
{
    Load(vShaderPath, gShaderPath, fShaderPath);
}

ShaderProgram::~ShaderProgram()
{
    GL::DeleteProgram(m_idGL);
}

bool ShaderProgram::Load(const Path &shaderPath)
{
    if (shaderPath.IsFile())
    {
        m_shaderPath = shaderPath;
        String shaderSourceCode = File::GetContents(shaderPath);

        AH<Shader> vShader = Assets::Create<Shader>();
        String vShaderSourceCode = ShaderPreprocessor::GetSourceCodeSection(
            shaderSourceCode, GL::ShaderType::VERTEX);
        vShader.Get()->SetSourceCode(vShaderSourceCode);
        vShader.Get()->SetType(GL::ShaderType::VERTEX);
        vShader.Get()->Compile();
        SetShader(vShader.Get(), GL::ShaderType::VERTEX);

        AH<Shader> fShader = Assets::Create<Shader>();
        String fShaderSourceCode = ShaderPreprocessor::GetSourceCodeSection(
            shaderSourceCode, GL::ShaderType::FRAGMENT);
        fShader.Get()->SetSourceCode(fShaderSourceCode);
        fShader.Get()->SetType(GL::ShaderType::FRAGMENT);
        fShader.Get()->Compile();
        SetShader(fShader.Get(), GL::ShaderType::FRAGMENT);

        return Link();
    }
    return false;
}

bool ShaderProgram::Load(const Path &vShaderPath, const Path &fShaderPath)
{
    AH<Shader> vShader = Assets::Load<Shader>(vShaderPath);
    AH<Shader> fShader = Assets::Load<Shader>(fShaderPath);
    return Load(vShader.Get(), fShader.Get());
}

bool ShaderProgram::Load(const Path &vShaderPath,
                         const Path &gShaderPath,
                         const Path &fShaderPath)
{
    AH<Shader> vShader = Assets::Load<Shader>(vShaderPath);
    AH<Shader> gShader = Assets::Load<Shader>(gShaderPath);
    AH<Shader> fShader = Assets::Load<Shader>(fShaderPath);
    return Load(vShader.Get(), gShader.Get(), fShader.Get());
}

bool ShaderProgram::Load(Shader *vShader, Shader *fShader)
{
    if (!vShader || !fShader)
    {
        return false;
    }
    if (vShader == GetVertexShader() && fShader == GetFragmentShader())
    {
        return true;
    }

    SetVertexShader(vShader);
    SetFragmentShader(fShader);
    return Link();
}

bool ShaderProgram::Load(Shader *vShader, Shader *gShader, Shader *fShader)
{
    if (!vShader || !gShader || !fShader)
    {
        return false;
    }
    if (vShader == GetVertexShader() && gShader == GetGeometryShader() &&
        fShader == GetFragmentShader())
    {
        return true;
    }

    SetVertexShader(vShader);
    SetGeometryShader(gShader);
    SetFragmentShader(fShader);
    return Link();
}

bool ShaderProgram::Link()
{
    if (!GetVertexShader())
    {
        Debug_Error("Vertex shader not set. Can't link shader program.");
        return false;
    }

    if (!GetFragmentShader())
    {
        Debug_Error("Fragment shader not set. Can't link shader program.");
        return false;
    }

    if (m_idGL > 0)
    {
        GL::DeleteProgram(m_idGL);
    }
    m_isLinked = false;

    m_idGL = GL::CreateProgram();

    GL::AttachShader(m_idGL, GetVertexShader()->GetGLId());
    if (GetGeometryShader())
    {
        GL::AttachShader(m_idGL, GetGeometryShader()->GetGLId());
    }
    GL::AttachShader(m_idGL, GetFragmentShader()->GetGLId());

    if (!GL::LinkProgram(m_idGL))
    {
        Path vsPath = (GetVertexShader() ? GetVertexShader()->GetAssetFilepath()
                                         : Path::Empty());
        Path gsPath =
            (GetGeometryShader() ? GetGeometryShader()->GetAssetFilepath()
                                 : Path::Empty());
        Path fsPath =
            (GetFragmentShader() ? GetFragmentShader()->GetAssetFilepath()
                                 : Path::Empty());
        Debug_Error("The shader program " << this << "( " << vsPath << ", "
                                          << gsPath
                                          << ", "
                                          << fsPath
                                          << ") did not link: "
                                          << GL::GetProgramErrorMsg(m_idGL));
        GL::DeleteProgram(m_idGL);
        m_idGL = 0;
        return false;
    }

    m_isLinked = true;

    // Invalidate caches
    m_nameToLocationCache.Clear();
    m_uniformCacheBool.Clear();
    m_uniformCacheInt.Clear();
    m_uniformCacheFloat.Clear();
    m_uniformCacheColor.Clear();
    m_uniformCacheVector2.Clear();
    m_uniformCacheVector3.Clear();
    m_uniformCacheVector4.Clear();
    m_uniformCacheMatrix3.Clear();
    m_uniformCacheMatrix4.Clear();
    m_namesToTexture.Clear();

    GLUniforms::GetActive()->BindUniformBuffers(this);

    return true;
}

bool ShaderProgram::IsLinked() const
{
    return m_isLinked;
}

GL::BindTarget ShaderProgram::GetGLBindTarget() const
{
    return GL::BindTarget::SHADER_PROGRAM;
}

template <template <class T> class Container, class T>
bool SetShaderUniformArray(ShaderProgram *sp,
                           const String &name,
                           const Container<T> &v,
                           bool warn)
{
    ASSERT(GL::IsBound(sp));

    bool update = true;
    if (update)
    {
        int location = sp->GetUniformLocation(name);
        if (location >= 0)
        {
            GL::Uniform(location, v);
        }
        else if (warn)
        {
            Debug_Warn("Uniform '" << name << "' not found");
        }
        return (location >= 0);
    }
    return true;
}

template <class T,
          class = typename std::enable_if<!std::is_pointer<T>::value, T>::type>
bool SetShaderUniform(ShaderProgram *sp,
                      UMap<String, T> *cache,
                      const String &name,
                      const T &v,
                      bool warn)
{
    ASSERT(GL::IsBound(sp));

    bool update = true;
    if (cache)
    {
        auto it = cache->Find(name);
        if (it != cache->End() && it->second == v)
        {
            update = false;
        }
    }

    if (update)
    {
        if (cache)
        {
            (*cache)[name] = v;
        }

        int location = sp->GetUniformLocation(name);
        if (location >= 0)
        {
            GL::Uniform(location, v);
        }
        else if (warn)
        {
            Debug_Warn("Uniform '" << name << "' not found");
        }
        return (location >= 0);
    }
    return true;
}

bool ShaderProgram::SetInt(const String &name, int v, bool warn)
{
    return SetShaderUniform<int>(this, &m_uniformCacheInt, name, v, warn);
}
bool ShaderProgram::SetBool(const String &name, bool v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheBool, name, v, warn);
}
bool ShaderProgram::SetFloat(const String &name, float v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheFloat, name, v, warn);
}

bool ShaderProgram::SetDouble(const String &name, double v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheDouble, name, v, warn);
}
bool ShaderProgram::SetColor(const String &name, const Color &v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheColor, name, v, warn);
}
bool ShaderProgram::SetVector2(const String &name, const Vector2 &v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheVector2, name, v, warn);
}
bool ShaderProgram::SetVector3(const String &name, const Vector3 &v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheVector3, name, v, warn);
}
bool ShaderProgram::SetVector4(const String &name, const Vector4 &v, bool warn)
{
    return SetShaderUniform(this, &m_uniformCacheVector4, name, v, warn);
}
bool ShaderProgram::SetMatrix3(const String &name, const Matrix3 &v, bool warn)
{
    // return SetShaderUniform(this, &m_uniformCacheMatrix3, name, v, warn);
    return SetShaderUniform<Matrix3>(this, nullptr, name, v, warn);
}
bool ShaderProgram::SetMatrix4(const String &name, const Matrix4 &v, bool warn)
{
    // return SetShaderUniform(this, &m_uniformCacheMatrix4, name, v, warn);
    return SetShaderUniform<Matrix4>(this, nullptr, name, v, warn);
}
bool ShaderProgram::SetIntArray(const String &name,
                                const Array<int> &v,
                                bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetBoolArray(const String &name,
                                 const Array<bool> &v,
                                 bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetFloatArray(const String &name,
                                  const Array<float> &v,
                                  bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}

bool ShaderProgram::SetDoubleArray(const String &name,
                                   const Array<double> &v,
                                   bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetColorArray(const String &name,
                                  const Array<Color> &v,
                                  bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetVector2Array(const String &name,
                                    const Array<Vector2> &v,
                                    bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetVector3Array(const String &name,
                                    const Array<Vector3> &v,
                                    bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetVector4Array(const String &name,
                                    const Array<Vector4> &v,
                                    bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetMatrix3Array(const String &name,
                                    const Array<Matrix3> &v,
                                    bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetMatrix4Array(const String &name,
                                    const Array<Matrix4> &v,
                                    bool warn)
{
    return SetShaderUniformArray(this, name, v, warn);
}
bool ShaderProgram::SetTexture(const String &name, Texture *texture, bool warn)
{
    if (!texture)
    {
        return false;
    }

    int uniformLocation = GetUniformLocation(name);
    if (uniformLocation < 0)
    {
        if (warn)
        {
            Debug_Warn("Texture uniform '" << name << "' not found.");
        }
        return false;
    }

    bool needToRefreshTexture;
    auto it = m_namesToTexture.Find(name);
    if (it != m_namesToTexture.End())
    {
        // Texture name was already being used...
        Texture *oldTexture = it->second;
        if (texture != oldTexture)
        {
            // We are changing an existing texture. Unregister listener
            if (oldTexture)
            {
                oldTexture->EventEmitter<IEventsDestroy>::UnRegisterListener(
                    this);
            }
            needToRefreshTexture = true;
        }
        else
        {
            // Nothing to do. We already had this tex assigned to this name
            needToRefreshTexture = false;
        }
    }
    else
    {
        needToRefreshTexture = true;
    }

    if (needToRefreshTexture)
    {
        // Texture name was not being used. Register and all stuff
        m_namesToTexture[name] = texture;

        // Register listener to keep track when it is destroyed
        texture->EventEmitter<IEventsDestroy>::RegisterListener(this);
    }

    if (GL::IsBound(this))
    {
        BindTextureToFreeUnit(name, texture);
    }

    if (m_namesToTexture.Size() >=
        TextureUnitManager::GetNumUsableTextureUnits())
    {
        Debug_Error(
            "You are using too many textures at once. Maximum usable is: "
            << TextureUnitManager::GetNumUsableTextureUnits());
    }

    return true;
}
bool ShaderProgram::SetTexture2D(const String &name,
                                 Texture2D *texture2D,
                                 bool warn)
{
    if (texture2D)
    {
        return SetTexture(name, SCAST<Texture *>(texture2D), warn);
    }
    else
    {
        return SetDefaultTexture2D(name, warn);
    }
}

bool ShaderProgram::SetTexture3D(const String &name,
                                 Texture3D *texture3D,
                                 bool warn)
{
    if (texture3D)
    {
        return SetTexture(name, SCAST<Texture *>(texture3D), warn);
    }
    else
    {
        return SetDefaultTexture3D(name, warn);
    }
}

bool ShaderProgram::SetDefaultTexture2D(const String &name, bool warn)
{
    return SetTexture(
        name, SCAST<Texture *>(TextureFactory::GetWhiteTexture()), warn);
}

bool ShaderProgram::SetDefaultTexture3D(const String &name, bool warn)
{
    return SetTexture(
        name, SCAST<Texture3D *>(TextureFactory::GetWhiteTexture3D()), warn);
}

bool ShaderProgram::SetTextureCubeMap(const String &name,
                                      TextureCubeMap *textureCM,
                                      bool warn)
{
    if (textureCM)
    {
        return SetTexture(name, SCAST<Texture *>(textureCM), warn);
    }
    else
    {
        return SetDefaultTextureCubeMap(name, warn);
    }
}
bool ShaderProgram::SetDefaultTextureCubeMap(const String &name, bool warn)
{
    return SetTexture(
        name, SCAST<Texture *>(TextureFactory::GetWhiteTextureCubeMap()), warn);
}

bool ShaderProgram::SetShader(Shader *shader, GL::ShaderType type)
{
    if (shader && shader->GetType() != type)
    {
        String typeName =
            (type == GL::ShaderType::VERTEX
                 ? "Vertex"
                 : (type == GL::ShaderType::GEOMETRY ? "Geometry"
                                                     : "Fragment"));
        Debug_Error("You are trying to set as " << typeName << " shader a "
                                                               "non-"
                                                << typeName
                                                << " shader.");
        return false;
    }

    if (GetShader(type))
    {
        GetShader(type)->EventEmitter<IEventsAsset>::UnRegisterListener(this);
    }

    switch (type)
    {
        case GL::ShaderType::VERTEX: p_vShader.Set(shader); break;
        case GL::ShaderType::GEOMETRY: p_gShader.Set(shader); break;
        case GL::ShaderType::FRAGMENT: p_fShader.Set(shader); break;
    }

    if (GetShader(type))
    {
        GetShader(type)->EventEmitter<IEventsAsset>::RegisterListener(this);
    }

    return true;
}

bool ShaderProgram::SetVertexShader(Shader *vertexShader)
{
    return SetShader(vertexShader, GL::ShaderType::VERTEX);
}

bool ShaderProgram::SetGeometryShader(Shader *geometryShader)
{
    return SetShader(geometryShader, GL::ShaderType::GEOMETRY);
}

bool ShaderProgram::SetFragmentShader(Shader *fragmentShader)
{
    return SetShader(fragmentShader, GL::ShaderType::FRAGMENT);
}

Shader *ShaderProgram::GetShader(GL::ShaderType type) const
{
    switch (type)
    {
        case GL::ShaderType::VERTEX: return p_vShader.Get();
        case GL::ShaderType::GEOMETRY: return p_gShader.Get();
        case GL::ShaderType::FRAGMENT: return p_fShader.Get();
    }

    ASSERT(false);
    return nullptr;
}
Shader *ShaderProgram::GetVertexShader() const
{
    return p_vShader.Get();
}
Shader *ShaderProgram::GetGeometryShader() const
{
    return p_gShader.Get();
}
Shader *ShaderProgram::GetFragmentShader() const
{
    return p_fShader.Get();
}

const Path &ShaderProgram::GetShaderPath() const
{
    return m_shaderPath;
}

GLint ShaderProgram::GetUniformLocation(const String &name) const
{
    auto it = m_nameToLocationCache.Find(name);
    if (it != m_nameToLocationCache.End())
    {
        return it->second;
    }

    const int location = GL::GetUniformLocation(GetGLId(), name);
    m_nameToLocationCache[name] = location;
    return location;
}

void ShaderProgram::Import(const Path &shaderPath)
{
    if (shaderPath.GetExtension() == "bshader")
    {
        Load(shaderPath);
    }
}

void ShaderProgram::Bind() const
{
    ASSERT(false);
}

void ShaderProgram::UnBind() const
{
    ASSERT(false);
}

void ShaderProgram::Bind()
{
    if (IsLinked())
    {
        if (!GL::IsBound(this))
        {
            GL::Bind(this);
        }

        GLUniforms::SetAllUniformsToShaderProgram(this);
        BindAllTexturesToUnits();
    }
}

void ShaderProgram::UnBind()
{
    // GL::UnBind(this);
}

void ShaderProgram::BindAllTexturesToUnits()
{
    ASSERT(GL::IsBound(this));

    for (const auto &pair : m_namesToTexture)
    {
        const String &texName = pair.first;
        Texture *texture = pair.second;
        BindTextureToFreeUnit(texName, texture);
    }
}

void ShaderProgram::CheckTextureBindingsValidity() const
{
    const int uniformsListSize = GL::GetUniformsListSize(GetGLId());
    USet<int> samplers1D, samplers2D, samplers3D, samplersCubeMap;
    for (int i = 0; i < uniformsListSize; ++i)
    {
        GL::UniformType uniformType = GL::GetUniformTypeAt(GetGLId(), i);
        if (uniformType != GL::UniformType::SAMPLER_1D &&
            uniformType != GL::UniformType::SAMPLER_1D_SHADOW &&
            uniformType != GL::UniformType::SAMPLER_1D_ARRAY_SHADOW &&
            uniformType != GL::UniformType::SAMPLER_2D &&
            uniformType != GL::UniformType::SAMPLER_2D_SHADOW &&
            uniformType != GL::UniformType::SAMPLER_2D_ARRAY_SHADOW &&
            uniformType != GL::UniformType::SAMPLER_3D &&
            uniformType != GL::UniformType::SAMPLER_CUBE &&
            uniformType != GL::UniformType::SAMPLER_CUBE_SHADOW)
        {
            continue;
        }

        GLUniforms::GLSLVar<int> uniformVar =
            GLUniforms::GetUniformAt<int>(GetGLId(), i);

        // Debug_Log(uniformVar.name << ": " << uniformVar.value);
        const int texUnit = uniformVar.value;
        const GL::UniformType samplerType = uniformType;
        switch (samplerType)
        {
            case GL::UniformType::SAMPLER_1D:
            case GL::UniformType::SAMPLER_1D_SHADOW:
            case GL::UniformType::SAMPLER_1D_ARRAY_SHADOW:
                samplers1D.Add(texUnit);
                break;

            case GL::UniformType::SAMPLER_2D:
            case GL::UniformType::SAMPLER_2D_SHADOW:
            case GL::UniformType::SAMPLER_2D_ARRAY_SHADOW:
                samplers2D.Add(texUnit);
                break;

            case GL::UniformType::SAMPLER_3D: samplers3D.Add(texUnit); break;

            case GL::UniformType::SAMPLER_CUBE:
            case GL::UniformType::SAMPLER_CUBE_SHADOW:
                samplersCubeMap.Add(texUnit);
                break;

            default: ASSERT(false);
        }

        int numDifferentSamplerTypesWhoPointToThisTexture = 0;

        if (samplers1D.Contains(texUnit))
        {
            ++numDifferentSamplerTypesWhoPointToThisTexture;
        }
        if (samplers2D.Contains(texUnit))
        {
            ++numDifferentSamplerTypesWhoPointToThisTexture;
        }
        if (samplers3D.Contains(texUnit))
        {
            ++numDifferentSamplerTypesWhoPointToThisTexture;
        }
        if (samplersCubeMap.Contains(texUnit))
        {
            ++numDifferentSamplerTypesWhoPointToThisTexture;
        }

        if (texUnit != 0)
        {
            ASSERT(numDifferentSamplerTypesWhoPointToThisTexture == 1);
        }
    }
}

void ShaderProgram::BindTextureToFreeUnit(const String &texUniformName,
                                          Texture *texture)
{
    ASSERT(texture);
    uint unit = TextureUnitManager::BindTextureToUnit(texture);
    SetInt(texUniformName, unit, false);  // Assign unit to sampler
}

void ShaderProgram::UnBindAllTexturesFromUnits() const
{
    TextureUnitManager::UnBindAllTexturesFromAllUnits();
}

void ShaderProgram::OnDestroyed(EventEmitter<IEventsDestroy> *object)
{
    Array<std::pair<String, Texture *>> entriesToRestore;
    Texture *destroyedTex = DCAST<Texture *>(object);
    for (auto it = m_namesToTexture.begin(); it != m_namesToTexture.end();)
    {
        Texture *tex = it->second;
        if (tex == destroyedTex)
        {
            const String &name = it->first;
            entriesToRestore.PushBack(std::make_pair(name, tex));

            it = m_namesToTexture.Remove(it);
            // Dont break, in case it has obj texture several times
        }
        else
        {
            ++it;
        }
    }

    // Set default textures to those removed entries.
    for (const auto &pair : entriesToRestore)
    {
        const String &name = pair.first;
        if (DCAST<TextureCubeMap *>(destroyedTex))
        {
            SetDefaultTextureCubeMap(name, false);
        }
        else if (DCAST<Texture2D *>(destroyedTex))
        {
            SetDefaultTexture2D(name, false);
        }
        else
        {
            SetDefaultTexture3D(name, false);
        }
    }
}

void ShaderProgram::OnImported(Asset *res)
{
    // When used shader is imported, link shaderProgram
    ASSERT(res == GetVertexShader() || res == GetGeometryShader() ||
           res == GetFragmentShader());
    Link();
}
