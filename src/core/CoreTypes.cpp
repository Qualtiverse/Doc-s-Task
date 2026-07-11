#include "CoreTypes.h"
#include "raylib-cpp.hpp"

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

PhysicsObject::PhysicsObject() : m_id(-1), m_active(true), m_dynamic(true), m_mass(1.0f) {
    m_color = WHITE;
}

PhysicsObject::PhysicsObject(raylib::Vector3 position, raylib::Vector3 size, float mass, bool dynamic)
    : m_position(position), m_size(size), m_mass(mass), m_dynamic(dynamic), m_active(true) {
    m_rotation = {0, 0, 0, 1};
    m_velocity = {0, 0, 0};
    m_angularVelocity = {0, 0, 0};
    m_color = raylib::Color{(unsigned char)(50 + rand() % 100), 
                           (unsigned char)(100 + rand() % 155), 
                           (unsigned char)(50 + rand() % 100), 255};
}

void PhysicsObject::Update(float dt) {
    if (!m_active || !m_dynamic) return;
    
    if (m_mass > 0) {
        raylib::Vector3 acceleration = {m_forces.x / m_mass, m_forces.y / m_mass, m_forces.z / m_mass};
        m_velocity.x += acceleration.x * dt;
        m_velocity.y += acceleration.y * dt;
        m_velocity.z += acceleration.z * dt;
    }
    
    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;
    m_position.z += m_velocity.z * dt;
    
    if (m_position.y < 0.5f) {
        m_position.y = 0.5f;
        if (m_velocity.y < 0) m_velocity.y *= -0.5f;
    }
    
    m_velocity.x *= 0.99f;
    m_velocity.y *= 0.99f;
    m_velocity.z *= 0.99f;
    
    m_forces = {0, 0, 0};
}

void PhysicsObject::Draw(const raylib::Camera3D& camera) const {
    if (!m_active) return;
    
    raylib::DrawCube(m_position, m_size.x, m_size.y, m_size.z, m_color);
    raylib::DrawCubeWires(m_position, m_size.x, m_size.y, m_size.z, BLACK);
}

void PhysicsObject::SetTransform(const TransformData& data) {
    m_position = data.position;
    m_rotation = data.rotation;
    m_velocity = data.velocity;
    m_angularVelocity = data.angularVelocity;
    m_active = data.valid;
}

TransformData PhysicsObject::GetTransformData() const {
    return TransformData(m_position, m_rotation, m_velocity, m_angularVelocity, 0);
}

void PhysicsObject::ApplyForce(const raylib::Vector3& force) {
    if (m_dynamic && m_active) {
        m_forces.x += force.x;
        m_forces.y += force.y;
        m_forces.z += force.z;
    }
}

void PhysicsObject::ApplyImpulse(const raylib::Vector3& impulse) {
    if (m_dynamic && m_active && m_mass > 0) {
        m_velocity.x += impulse.x / m_mass;
        m_velocity.y += impulse.y / m_mass;
        m_velocity.z += impulse.z / m_mass;
    }
}