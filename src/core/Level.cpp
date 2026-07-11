#include "Level.h"
#include "raylib-cpp.hpp"

void Level::Load() {
    m_completed = false;
}

void Level::Unload() {
    m_objects.clear();
}

void Level::Update(float dt, TimeRecorder& recorder) {
    for (auto& obj : m_objects) {
        if (obj->IsActive()) {
            obj->Update(dt);
        }
    }
}

void Level::Draw(const raylib::Camera3D& camera) {
    for (auto& obj : m_objects) {
        if (obj->IsActive()) {
            obj->Draw(camera);
        }
    }
    
    raylib::DrawCylinderWires(m_goalPosition, m_goalRadius, m_goalRadius, 2.0f, 8, m_goalColor);
    raylib::DrawCylinder(m_goalPosition, m_goalRadius, m_goalRadius, 0.2f, 16, raylib::Fade(m_goalColor, 0.3f));
}

void Level::DrawUI() {
    // Override in derived classes
}

bool Level::IsComplete() {
    return m_completed;
}

void Level::Reset() {
    m_completed = false;
    for (auto& obj : m_objects) {
        obj->SetPosition(obj->GetStartPosition());
        obj->SetVelocity({0, 0, 0});
        obj->SetActive(true);
    }
}

void Level::AddObject(std::unique_ptr<PhysicsObject> obj) {
    m_objects.push_back(std::move(obj));
}

void Level::AddBox(raylib::Vector3 pos, raylib::Vector3 size, float mass, bool dynamic, raylib::Color color) {
    auto obj = std::make_unique<PhysicsObject>(pos, size, mass, dynamic);
    obj->SetColor(color);
    m_objects.push_back(std::move(obj));
}

void Level::AddStaticBox(raylib::Vector3 pos, raylib::Vector3 size, raylib::Color color) {
    AddBox(pos, size, 0.0f, false, color);
}

void Level::CreateGround(float size) {
    AddStaticBox({0, -0.5f, 0}, {size, 1, size}, DARKGRAY);
}

void Level::CreateGoal(raylib::Vector3 pos, float radius) {
    m_goalPosition = pos;
    m_goalRadius = radius;
}

bool Level::CheckGoal(raylib::Vector3 playerPos) const {
    float dx = playerPos.x - m_goalPosition.x;
    float dz = playerPos.z - m_goalPosition.z;
    return (dx*dx + dz*dz) <= (m_goalRadius * m_goalRadius);
}

void Level::GetObjectStates(std::vector<TransformData>& states) const {
    states.clear();
    for (auto& obj : m_objects) {
        if (obj->IsActive() && obj->IsDynamic()) {
            states.push_back(obj->GetTransformData());
        }
    }
}

void Level::SetObjectStates(const std::vector<TransformData>& states) {
    int idx = 0;
    for (auto& obj : m_objects) {
        if (obj->IsActive() && obj->IsDynamic() && idx < (int)states.size()) {
            obj->SetTransform(states[idx]);
            idx++;
        }
    }
}