#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "Bang/Bang.h"
#include "Bang/Collider.h"
#include "Bang/MeshRenderer.h"
#include "Bang/ComplexRandom.h"
#include "Bang/ObjectGatherer.h"
#include "Bang/ResourceHandle.h"

NAMESPACE_BANG_BEGIN

FORWARD class VAO;
FORWARD class VBO;

enum class ParticlePhysicsStepMode
{
    EULER,
    EULER_SEMI,
    VERLET
};

enum class ParticleGenerationShape
{
    BOX,
    CONE
};

enum class ParticleRenderMode
{
    ADDITIVE,
    MESH
};

enum class ParticleSimulationSpace
{
    LOCAL,
    WORLD
};

class ParticleSystem : public Renderer
{
    COMPONENT_WITH_FAST_DYNAMIC_CAST(ParticleSystem)

private:
    struct ParticleData
    {
        Vector3 position;
        Vector3 velocity;
        Vector3 force;
        Color startColor;
        Color endColor;
        float totalLifeTime;
        float remainingLifeTime;
        float remainingStartTime;
        float size;
    };
    struct ParticleVBOData
    {
        Vector3 position;
        float size;
        Color color;
    };

public:
	ParticleSystem();
	virtual ~ParticleSystem();

    // Component
    virtual void OnStart() override;
    virtual void OnUpdate() override;

    void Reset();

    void SetMesh(Mesh *mesh);
    void SetLifeTime(const ComplexRandom &lifeTime);
    void SetStartTime(const ComplexRandom &startTime);
    void SetStartSize(const ComplexRandom &startSize);
    void SetBillboard(bool billboard);
    void SetNumParticles(uint numParticles);
    void SetGenerationShape(ParticleGenerationShape shape);
    void SetGenerationShapeBoxSize(const Vector3 &boxSize);
    void SetPhysicsStepMode(ParticlePhysicsStepMode stepMode);
    void SetGravityMultiplier(float gravityMultiplier);
    void SetInitialVelocityMultiplier(float initialVelocityMultiplier);
    void SetGenerationShapeConeFOVRads(float coneFOVRads);
    void SetStartColor(const Color &startColor);
    void SetEndColor(const Color &endColor);
    void SetParticleRenderMode(ParticleRenderMode particleRenderMode);
    void SetSimulationSpace(ParticleSimulationSpace simulationSpace);

    Mesh *GetMesh() const;
    bool GetBillboard() const;
    uint GetNumParticles() const;
    const Vector3 &GetGenerationShapeBoxSize() const;
    ParticleGenerationShape GetGenerationShape() const;
    const ComplexRandom& GetLifeTime() const;
    const ComplexRandom& GetStartTime() const;
    const ComplexRandom& GetStartSize() const;
    const Color &GetStartColor() const;
    const Color &GetEndColor() const;
    float GetGravityMultiplier() const;
    float GetInitialVelocityMultiplier() const;
    float GetGenerationShapeConeFOVRads() const;
    ParticleRenderMode GetParticleRenderMode() const;
    ParticlePhysicsStepMode GetPhysicsStepMode() const;
    ParticleSimulationSpace GetSimulationSpace() const;

    // Renderer
    virtual void Bind() override;
    virtual void OnRender() override;
    virtual void SetUniformsOnBind(ShaderProgram *sp);
    AABox GetAABBox() const override;

    // Component
    void OnGameObjectChanged(GameObject *prevGameObject,
                             GameObject *newGameObject);

    // ICloneable
    virtual void CloneInto(ICloneable *clone) const override;

    // Serializable
    virtual void ImportMeta(const MetaNode &metaNode) override;
    virtual void ExportMeta(MetaNode *metaNode) const override;

private:
    ObjectGatherer<Collider, true> m_sceneCollidersGatherer;

    VAO *p_particlesVAO = nullptr;
    VBO *p_particleDataVBO = nullptr;
    Array<ParticleVBOData> m_particlesVBOData;
    Array<ParticleData> m_particlesData;

    RH<Mesh> m_particleMesh;
    uint m_numParticles = 0;

    ParticleSimulationSpace m_simulationSpace =
                            ParticleSimulationSpace::WORLD;
    ParticlePhysicsStepMode m_physicsStepMode = ParticlePhysicsStepMode::EULER;

    ParticleGenerationShape m_generationShape = ParticleGenerationShape::BOX;
    Vector3 m_generationShapeBoxSize = Vector3::One;
    float m_generationShapeConeFOVRads = Math::Pi/4;

    ParticleRenderMode m_particleRenderMode = Undef<ParticleRenderMode>();

    bool m_billboard = true;
    Color m_startColor = Color::White;
    Color m_endColor   = Color::White;
    ComplexRandom m_lifeTime  = ComplexRandom(0.1f, 3.0f);
    ComplexRandom m_startTime = ComplexRandom(0.1f, 5.0f);
    ComplexRandom m_startSize = ComplexRandom(0.3f, 1.0f);
    float m_gravityMultiplier = 0.0f;
    float m_initialVelocityMultiplier = 1.0f;

    void InitParticle(uint i, const Vector3 &gravity);
    void UpdateParticleData(uint i,
                            float deltaTimeSecs,
                            const Vector3 &gravity,
                            const Array<Collider*> &sceneColliders);
    void CollideParticle(uint i,
                         Collider *collider,
                         const Vector3 &prevPositionNoInt,
                         const Vector3 &newPositionNoInt,
                         const Vector3 &newVelocityNoInt,
                         Vector3 *newPositionAfterInt,
                         Vector3 *newVelocityAfterInt);
    void RecreateVAOForMesh();
    void UpdateDataVBO();

    Vector3 GetParticleInitialPosition() const;
    Vector3 GetParticleInitialVelocity() const;
};

NAMESPACE_BANG_END

#endif // PARTICLESYSTEM_H

