#pragma once

#include "raylib-cpp.hpp"
#include "raylib.h"
#include <vector>
#include <array>
#include <cstdint>

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int MAX_RECORD_FRAMES = 3600;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr int MAX_PHYSICS_OBJECTS = 64;

enum class GameState {
    MENU,
    PLAYING,
    RECORDING,
    REPLAYING,
    PAUSED,
    LEVEL_COMPLETE,
    GAME_COMPLETE
};

enum class TimeMode {
    LIVE,
    RECORDING,
    REPLAYING
};

struct TransformData {
    raylib::Vector3 position{0, 0, 0};
    raylib::Quaternion rotation{0, 0, 0, 1};
    raylib::Vector3 velocity{0, 0, 0};
    raylib::Vector3 angularVelocity{0, 0, 0};
    bool valid = false;
    uint32_t objectId = 0;
};

struct RecordFrame {
    float timestamp = 0;
    std::array<TransformData, MAX_PHYSICS_OBJECTS> transforms;
    int activeCount = 0;
};

struct RecordedObject {
    uint32_t id = 0;
    std::vector<TransformData> keyframes;
    std::string name;
};

class PhysicsObject {
public:
    PhysicsObject() = default;
    PhysicsObject(uint32_t id, const raylib::Vector3& pos, const raylib::Vector3& size, float mass, bool isStatic = false);
    
    void Update(float dt);
    void Draw() const;
    
    void ApplyForce(const raylib::Vector3& force);
    void ApplyImpulse(const raylib::Vector3& impulse);
    void SetPosition(const raylib::Vector3& pos);
    void SetRotation(const raylib::Quaternion& rot);
    void SetVelocity(const raylib::Vector3& vel);
    void SetAngularVelocity(const raylib::Vector3& vel);
    
    raylib::Vector3 GetPosition() const { return m_position; }
    raylib::Quaternion GetRotation() const { return m_rotation; }
    raylib::Vector3 GetVelocity() const { return m_velocity; }
    raylib::Vector3 GetAngularVelocity() const { return m_angularVelocity; }
    uint32_t GetId() const { return m_id; }
    bool IsStatic() const { return m_isStatic; }
    bool IsActive() const { return m_active; }
    
    void SetActive(bool active) { m_active = active; }
    void SetColor(const raylib::Color& color) { m_color = color; }
    
    TransformData GetTransformData() const;
    void SetFromTransformData(const TransformData& data);
    
private:
    uint32_t m_id = 0;
    raylib::Vector3 m_position{0, 0, 0};
    raylib::Quaternion m_rotation{0, 0, 0, 1};
    raylib::Vector3 m_velocity{0, 0, 0};
    raylib::Vector3 m_angularVelocity{0, 0, 0};
    raylib::Vector3 m_force{0, 0, 0};
    raylib::Vector3 m_torque{0, 0, 0};
    raylib::Vector3 m_size{1, 1, 1};
    float m_mass = 1.0f;
    float m_inverseMass = 1.0f;
    raylib::Matrix m_inertiaTensor = raylib::MatrixIdentity();
    bool m_isStatic = false;
    bool m_active = true;
    bool m_useGravity = true;
    raylib::Color m_color = WHITE;
};

class Level {
public:
    virtual ~Level() = default;
    virtual void Load() = 0;
    virtual void Unload() = 0;
    virtual void Update(float dt, class TimeRecorder& recorder) = 0;
    virtual void Draw(const raylib::Camera3D& camera) = 0;
    virtual void DrawUI() = 0;
    virtual void ProcessInput(float dt, TimeMode mode) = 0;
    virtual bool IsComplete() const = 0;
    virtual void Reset() = 0;
    virtual const char* GetName() const = 0;
    virtual const char* GetHint() const = 0;
    
    virtual int GetObjectCount() const = 0;
    virtual void GetObjectStates(std::vector<TransformData>& states) const = 0;
    virtual void SetObjectStates(const std::vector<TransformData>& states) = 0;
    virtual raylib::Vector3 GetPlayerStart() const = 0;
};