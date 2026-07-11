#pragma once

#include "raylib.h"
#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <memory>

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
    Vector3 position{0, 0, 0};
    Quaternion rotation{0, 0, 0, 1};
    Vector3 velocity{0, 0, 0};
    Vector3 angularVelocity{0, 0, 0};
    bool valid = false;
    uint32_t objectId = 0;
};

struct RecordingFrame {
    float timestamp = 0;
    std::vector<TransformData> objectStates;
    
    RecordingFrame() : timestamp(0) {}
    RecordingFrame(float time) : timestamp(time) {}
};

class TimeRecorder {
public:
    TimeRecorder(int maxFrames = MAX_RECORD_FRAMES);
    
    void StartRecording(int numObjects);
    void StopRecording();
    void RecordFrame(const std::vector<TransformData>& states, float time);
    
    void StartReplay();
    void StopReplay();
    bool GetReplayFrame(std::vector<TransformData>& outStates, float currentTime);
    
    void ScrubToTime(float time);
    float GetRecordingDuration() const;
    int GetFrameCount() const;
    bool IsRecording() const { return m_recording; }
    bool IsReplaying() const { return m_replaying; }
    
    void Clear();
    
private:
    std::vector<RecordingFrame> m_frames;
    int m_maxFrames;
    int m_numObjects = 0;
    bool m_recording = false;
    bool m_replaying = false;
    int m_replayIndex = 0;
    int m_currentFrame = 0;
    float m_recordingStartTime = 0;
    float m_replayStartTime = 0;
};

class PhysicsObject {
public:
    PhysicsObject();
    PhysicsObject(const Vector3& pos, const Vector3& size, float mass, bool isStatic = false);
    
    void Update(float dt);
    void Draw() const;
    
    void ApplyForce(const Vector3& force);
    void ApplyImpulse(const Vector3& impulse);
    void SetPosition(const Vector3& pos);
    void SetRotation(const Quaternion& rot);
    void SetVelocity(const Vector3& vel);
    void SetAngularVelocity(const Vector3& vel);
    void SetColor(const Color& color) { m_color = color; }
    void SetActive(bool active) { m_active = active; }
    
    Vector3 GetPosition() const { return m_position; }
    Quaternion GetRotation() const { return m_rotation; }
    Vector3 GetVelocity() const { return m_velocity; }
    Vector3 GetAngularVelocity() const { return m_angularVelocity; }
    Vector3 GetSize() const { return m_size; }
    Vector3 GetStartPosition() const { return m_startPosition; }
    uint32_t GetId() const { return m_id; }
    float GetMass() const { return m_mass; }
    bool IsStatic() const { return m_isStatic; }
    bool IsActive() const { return m_active; }
    bool IsDynamic() const { return !m_isStatic; }
    
    TransformData GetTransformData() const;
    void SetFromTransformData(const TransformData& data);
    
    void SetStartPosition(const Vector3& pos) { m_startPosition = pos; }
    
private:
    uint32_t m_id = 0;
    Vector3 m_startPosition{0, 0, 0};
    Vector3 m_position{0, 0, 0};
    Quaternion m_rotation{0, 0, 0, 1};
    Vector3 m_velocity{0, 0, 0};
    Vector3 m_angularVelocity{0, 0, 0};
    Vector3 m_forces{0, 0, 0};
    Vector3 m_size{1, 1, 1};
    float m_mass = 1.0f;
    bool m_isStatic = false;
    bool m_active = true;
    bool m_useGravity = true;
    Color m_color = WHITE;
    static uint32_t s_nextId;
};

class ILevel {
public:
    virtual ~ILevel() = default;
    virtual void Load() = 0;
    virtual void Unload() = 0;
    virtual void Update(float dt, TimeRecorder& recorder) = 0;
    virtual void Draw(const Camera3D& camera) = 0;
    virtual void DrawUI() = 0;
    virtual void ProcessInput(float dt, TimeMode mode) = 0;
    virtual bool IsComplete() const = 0;
    virtual void Reset() = 0;
    virtual const char* GetName() const = 0;
    virtual const char* GetHint() const = 0;
    virtual int GetObjectCount() const = 0;
    virtual void GetObjectStates(std::vector<TransformData>& states) const = 0;
    virtual void SetObjectStates(const std::vector<TransformData>& states) = 0;
    virtual Vector3 GetPlayerStart() const = 0;
};