#include "CoreTypes.h"
#include "raylib.h"

uint32_t PhysicsObject::s_nextId = 0;

TimeRecorder::TimeRecorder(int maxFrames) : m_maxFrames(maxFrames) {
    m_frames.reserve(maxFrames);
}

void TimeRecorder::StartRecording(int numObjects) {
    m_recording = true;
    m_replaying = false;
    m_numObjects = numObjects;
    m_frames.clear();
    m_recordingStartTime = 0;
    m_currentFrame = 0;
}

void TimeRecorder::StopRecording() {
    m_recording = false;
}

void TimeRecorder::RecordFrame(const std::vector<TransformData>& states, float time) {
    if (!m_recording || (int)states.size() != m_numObjects) return;
    
    if (m_frames.empty()) {
        m_recordingStartTime = time;
    }
    
    float relTime = time - m_recordingStartTime;
    
    RecordingFrame frame(relTime);
    frame.objectStates = states;
    
    if (m_currentFrame < (int)m_frames.size()) {
        m_frames[m_currentFrame] = std::move(frame);
    } else {
        m_frames.push_back(std::move(frame));
    }
    m_currentFrame++;
    
    if (m_currentFrame >= m_maxFrames) {
        m_currentFrame = 0;
        m_recordingStartTime = time;
    }
}

void TimeRecorder::StartReplay() {
    if (m_frames.empty()) return;
    m_replaying = true;
    m_recording = false;
    m_replayIndex = 0;
    m_replayStartTime = 0;
}

void TimeRecorder::StopReplay() {
    m_replaying = false;
    m_replayIndex = 0;
}

bool TimeRecorder::GetReplayFrame(std::vector<TransformData>& outStates, float currentTime) {
    if (!m_replaying || m_frames.empty()) return false;
    
    float targetTime = currentTime - m_replayStartTime;
    if (targetTime > m_frames.back().timestamp) {
        StopReplay();
        return false;
    }
    
    while (m_replayIndex < (int)m_frames.size() - 1 && 
           m_frames[m_replayIndex + 1].timestamp <= targetTime) {
        m_replayIndex++;
    }
    
    outStates = m_frames[m_replayIndex].objectStates;
    return true;
}

void TimeRecorder::ScrubToTime(float time) {
    if (m_frames.empty()) return;
    m_replayIndex = 0;
    while (m_replayIndex < (int)m_frames.size() - 1 && 
           m_frames[m_replayIndex + 1].timestamp <= time) {
        m_replayIndex++;
    }
}

float TimeRecorder::GetRecordingDuration() const {
    if (m_frames.empty()) return 0;
    return m_frames.back().timestamp;
}

int TimeRecorder::GetFrameCount() const {
    return (int)m_frames.size();
}

void TimeRecorder::Clear() {
    m_frames.clear();
    m_recording = false;
    m_replaying = false;
    m_replayIndex = 0;
    m_currentFrame = 0;
}

PhysicsObject::PhysicsObject() : m_id(s_nextId++), m_active(true), m_isStatic(true) {
    m_color = GRAY;
    m_startPosition = {0, 0, 0};
}

PhysicsObject::PhysicsObject(const Vector3& pos, const Vector3& size, float mass, bool isStatic)
    : m_id(s_nextId++), m_startPosition(pos), m_position(pos), m_size(size), m_mass(mass), m_isStatic(isStatic), m_active(true) {
    m_rotation = {0, 0, 0, 1};
    m_velocity = {0, 0, 0};
    m_angularVelocity = {0, 0, 0};
    m_color = Color{(unsigned char)(50 + rand() % 100), 
                           (unsigned char)(100 + rand() % 155), 
                           (unsigned char)(50 + rand() % 100), 255};
}

void PhysicsObject::Update(float dt) {
    if (!m_active || m_isStatic) return;
    
    if (m_mass > 0) {
        Vector3 acceleration = {m_forces.x / m_mass, m_forces.y / m_mass, m_forces.z / m_mass};
        m_velocity.x += acceleration.x * dt;
        m_velocity.y += acceleration.y * dt;
        m_velocity.z += acceleration.z * dt;
    }
    
    m_velocity.y -= 9.81f * dt;
    
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;
    m_position.z += m_velocity.z * dt;
    
    if (m_position.y < 0.5f) {
        m_position.y = 0.5f;
        if (m_velocity.y < 0) m_velocity.y *= -0.3f;
    }
    
    m_velocity.x *= 0.98f;
    m_velocity.y *= 0.98f;
    m_velocity.z *= 0.98f;
    
    m_forces = {0, 0, 0};
}

void PhysicsObject::Draw() const {
    if (!m_active) return;
    
    DrawCube(m_position, m_size.x, m_size.y, m_size.z, m_color);
    DrawCubeWires(m_position, m_size.x, m_size.y, m_size.z, BLACK);
}

void PhysicsObject::SetPosition(const Vector3& pos) { m_position = pos; }
void PhysicsObject::SetRotation(const Quaternion& rot) { m_rotation = rot; }
void PhysicsObject::SetVelocity(const Vector3& vel) { m_velocity = vel; }
void PhysicsObject::SetAngularVelocity(const Vector3& vel) { m_angularVelocity = vel; }

TransformData PhysicsObject::GetTransformData() const {
    TransformData data;
    data.position = m_position;
    data.rotation = m_rotation;
    data.velocity = m_velocity;
    data.angularVelocity = m_angularVelocity;
    data.valid = m_active;
    data.objectId = m_id;
    return data;
}

void PhysicsObject::SetFromTransformData(const TransformData& data) {
    m_position = data.position;
    m_rotation = data.rotation;
    m_velocity = data.velocity;
    m_angularVelocity = data.angularVelocity;
    m_active = data.valid;
}

void PhysicsObject::ApplyForce(const Vector3& force) {
    if (!m_isStatic && m_active) {
        m_forces.x += force.x;
        m_forces.y += force.y;
        m_forces.z += force.z;
    }
}

void PhysicsObject::ApplyImpulse(const Vector3& impulse) {
    if (!m_isStatic && m_active && m_mass > 0) {
        m_velocity.x += impulse.x / m_mass;
        m_velocity.y += impulse.y / m_mass;
        m_velocity.z += impulse.z / m_mass;
    }
}