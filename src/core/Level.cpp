#include "Level.h"
#include "raylib.h"

void LevelBase::Load() {
    m_completed = false;
}

void LevelBase::Unload() {
    m_objects.clear();
}

void LevelBase::Update(float dt, TimeRecorder& recorder) {
    for (auto& obj : m_objects) {
        if (obj->IsActive()) {
            obj->Update(dt);
        }
    }
}

void LevelBase::Draw(const Camera3D& camera) {
    for (auto& obj : m_objects) {
        if (obj->IsActive()) {
            obj->Draw();
        }
    }
    
    DrawCylinder(m_goalPosition, m_goalRadius, m_goalRadius, 0.2f, 16, Fade(m_goalColor, 0.3f));
    DrawCylinderWires(m_goalPosition, m_goalRadius, m_goalRadius, 2.0f, 8, m_goalColor);
}

void LevelBase::DrawUI() {
}

bool LevelBase::IsComplete() const {
    return m_completed;
}

void LevelBase::Reset() {
    m_completed = false;
    for (auto& obj : m_objects) {
        obj->SetPosition(obj->GetStartPosition());
        obj->SetVelocity({0, 0, 0});
        obj->SetActive(true);
    }
}

void LevelBase::AddObject(std::unique_ptr<PhysicsObject> obj) {
    m_objects.push_back(std::move(obj));
}

void LevelBase::AddBox(Vector3 pos, Vector3 size, float mass, bool dynamic, Color color) {
    auto obj = std::make_unique<PhysicsObject>(pos, size, mass, !dynamic);
    obj->SetColor(color);
    obj->SetStartPosition(pos);
    m_objects.push_back(std::move(obj));
}

void LevelBase::AddStaticBox(Vector3 pos, Vector3 size, Color color) {
    AddBox(pos, size, 0.0f, false, color);
}

void LevelBase::CreateGround(float size) {
    AddStaticBox({0, -0.5f, 0}, {size, 1, size}, DARKGRAY);
}

void LevelBase::CreateGoal(Vector3 pos, float radius) {
    m_goalPosition = pos;
    m_goalRadius = radius;
}

bool LevelBase::CheckGoal(Vector3 playerPos) const {
    float dx = playerPos.x - m_goalPosition.x;
    float dz = playerPos.z - m_goalPosition.z;
    return (dx*dx + dz*dz) <= (m_goalRadius * m_goalRadius);
}

void LevelBase::GetObjectStates(std::vector<TransformData>& states) const {
    states.clear();
    for (auto& obj : m_objects) {
        if (obj->IsActive() && !obj->IsStatic()) {
            states.push_back(obj->GetTransformData());
        }
    }
}

void LevelBase::SetObjectStates(const std::vector<TransformData>& states) {
    int idx = 0;
    for (auto& obj : m_objects) {
        if (obj->IsActive() && !obj->IsStatic() && idx < (int)states.size()) {
            obj->SetFromTransformData(states[idx]);
            idx++;
        }
    }
}