#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <unordered_map>

#include "Bang/Map.h"
#include "Bang/Matrix3.h"
#include "Bang/Matrix4.h"
#include "Bang/Vector2.h"
#include "Bang/Vector3.h"
#include "Bang/Vector4.h"
#include "Bang/GLObject.h"
#include "Bang/Resources.h"
#include "Bang/IEventsDestroy.h"
#include "Bang/IEventsResource.h"

NAMESPACE_BANG_BEGIN

FORWARD class Shader;
FORWARD class Texture2D;
FORWARD class TextureCubeMap;

class ShaderProgram : public GLObject,
                      public Resource,
                      public EventListener<IEventsResource>,
                      public EventListener<IEventsDestroy>
{
    RESOURCE(ShaderProgram)

public:
    bool Load(const Path &vShaderPath, const Path &fShaderPath);
    bool Load(const Path &vShaderPath,
              const Path &gShaderPath,
              const Path &fShaderPath);
    bool Load(Shader* vShader, Shader* fShader);
    bool Load(Shader* vShader, Shader* gShader, Shader* fShader);

    bool Link();
    bool IsLinked() const;

    void Bind() override;
    void UnBind() override;
    GL::BindTarget GetGLBindTarget() const override;

    bool SetInt(const String &name, int v, bool warn = true);
    bool SetBool(const String &name, bool v, bool warn = true);
    bool SetFloat(const String &name, float v, bool warn = true);
    bool SetDouble(const String &name, double v, bool warn = true);
    bool SetColor(const String &name, const Color& v, bool warn = true);
    bool SetVector2(const String &name, const Vector2& v, bool warn = true);
    bool SetVector3(const String &name, const Vector3& v, bool warn = true);
    bool SetVector4(const String &name, const Vector4& v, bool warn = true);
    bool SetMatrix3(const String &name, const Matrix3& v, bool warn = true);
    bool SetMatrix4(const String &name, const Matrix4& v, bool warn = true);
    bool SetTexture2D(const String &name, Texture2D *texture, bool warn = true);
    bool SetTextureCubeMap(const String &name, TextureCubeMap *textureCubeMap,
                           bool warn = true);
    bool SetIntArray(const String &name, const Array<int>& v, bool warn = true);
    bool SetBoolArray(const String &name, const Array<bool>& v, bool warn = true);
    bool SetFloatArray(const String &name, const Array<float>& v, bool warn = true);
    bool SetDoubleArray(const String &name, const Array<double>& v, bool warn = true);
    bool SetColorArray(const String &name, const Array<Color>& v, bool warn = true);
    bool SetVector2Array(const String &name, const Array<Vector2>& v, bool warn = true);
    bool SetVector3Array(const String &name, const Array<Vector3>& v, bool warn = true);
    bool SetVector4Array(const String &name, const Array<Vector4>& v, bool warn = true);
    bool SetMatrix3Array(const String &name, const Array<Matrix3>& v, bool warn = true);
    bool SetMatrix4Array(const String &name, const Array<Matrix4>& v, bool warn = true);
    bool SetTexture2DArray(const String &name, const Array<Texture2D*>& v, bool warn = true);

    bool SetShader(Shader *shader, GL::ShaderType type);
    bool SetVertexShader(Shader* vertexShader);
    bool SetGeometryShader(Shader* geometryShader);
    bool SetFragmentShader(Shader* fragmentShader);

    Shader* GetShader(GL::ShaderType type) const;
    Shader* GetVertexShader() const;
    Shader* GetGeometryShader() const;
    Shader* GetFragmentShader() const;

    GLint GetUniformLocation(const String &name) const;

    // Resource
    void Import(const Path &resourceFilepath) override;

private:
    RH<Shader> p_vShader;
    RH<Shader> p_gShader;
    RH<Shader> p_fShader;
    bool m_isLinked = false;

    std::unordered_map<String, int> m_uniformCacheInt;
    std::unordered_map<String, bool> m_uniformCacheBool;
    std::unordered_map<String, float> m_uniformCacheFloat;
    std::unordered_map<String, double> m_uniformCacheDouble;
    std::unordered_map<String, Color> m_uniformCacheColor;
    std::unordered_map<String, Vector2> m_uniformCacheVector2;
    std::unordered_map<String, Vector3> m_uniformCacheVector3;
    std::unordered_map<String, Vector4> m_uniformCacheVector4;
    std::unordered_map<String, Matrix3> m_uniformCacheMatrix3;
    std::unordered_map<String, Matrix4> m_uniformCacheMatrix4;

    mutable std::unordered_map<String, int> m_nameToLocationCache;
    mutable std::unordered_map<String, Texture*> m_namesToTexture;

    ShaderProgram();
    ShaderProgram(Shader *vShader, Shader *fShader);
    ShaderProgram(Shader *vShader, Shader *gShader, Shader *fShader);
    ShaderProgram(const Path& vShaderPath, const Path& fShaderPath);
    ShaderProgram(const Path& vShaderPath,
                  const Path& gShaderPath,
                  const Path& fShaderPath);
    virtual ~ShaderProgram();

    void Bind() const override;
    void UnBind() const override;

    bool SetDefaultTexture2D(const String &name, bool warn = true);
    bool SetDefaultTextureCubeMap(const String &name, bool warn = true);
    bool SetTexture(const String &name, Texture *texture, bool warn = true);

    void BindAllTexturesToUnits();
    void CheckTextureBindingsValidity() const;
    void BindTextureToFreeUnit(const String &textureName, Texture *texture);
    void UnBindAllTexturesFromUnits() const;

    // IResourceListener
    void OnImported(Resource *res) override;

    // IEventsDestroy
    void OnDestroyed(EventEmitter<IEventsDestroy> *object) override;
};

NAMESPACE_BANG_END

#endif // SHADERPROGRAM_H
