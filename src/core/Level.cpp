#include "Level.h"
#include "raylib.h"
#include <cstdio>
#include <cstring>

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

int LevelBase::EditorAddObject(Vector3 pos, Vector3 size, float mass, bool isStatic, Color color) {
    auto obj = std::make_unique<PhysicsObject>(pos, size, mass, isStatic);
    obj->SetColor(color);
    obj->SetStartPosition(pos);
    m_objects.push_back(std::move(obj));
    return (int)m_objects.size() - 1;
}

void LevelBase::EditorRemoveObject(int index) {
    if (index >= 0 && index < (int)m_objects.size())
        m_objects.erase(m_objects.begin() + index);
}

void LevelBase::EditorClearObjects() {
    m_objects.clear();
    CreateGround(20);
    CreateGoal({0, 1, 10}, 1.5f);
}

PhysicsObject* LevelBase::EditorGetObject(int index) {
    if (index >= 0 && index < (int)m_objects.size())
        return m_objects[index].get();
    return nullptr;
}

void LevelBase::EditorSetGoal(Vector3 pos, float radius) {
    m_goalPosition = pos;
    m_goalRadius = radius;
}

void LevelBase::EditorSetPlayerStart(Vector3 pos) {
    m_playerStart = pos;
}

bool LevelBase::EditorSaveToFile(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) return false;
    fprintf(f, "name=%s\n", m_levelName.c_str());
    fprintf(f, "hint=%s\n", m_levelHint.c_str());
    fprintf(f, "player=%.2f %.2f %.2f\n", m_playerStart.x, m_playerStart.y, m_playerStart.z);
    fprintf(f, "goal=%.2f %.2f %.2f %.2f\n", m_goalPosition.x, m_goalPosition.y, m_goalPosition.z, m_goalRadius);
    for (auto& obj : m_objects) {
        Color c = obj->GetColor();
        fprintf(f, "%s=%.2f %.2f %.2f %.2f %.2f %.2f %d %d %d %d %.2f\n",
            obj->IsStatic() ? "STATIC" : "DYNAMIC",
            obj->GetPosition().x, obj->GetPosition().y, obj->GetPosition().z,
            obj->GetSize().x, obj->GetSize().y, obj->GetSize().z,
            c.r, c.g, c.b, c.a,
            obj->GetMass());
    }
    fclose(f);
    return true;
}

bool LevelBase::EditorLoadFromFile(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    m_objects.clear();
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char* nl = strchr(line, '\n'); if (nl) *nl = 0;
        if (line[0] == 0 || line[0] == '#') continue;
        char* val = strchr(line, '=');
        if (!val) continue;
        *val++ = 0;
        if (strcmp(line, "name") == 0) { m_levelName = val; }
        else if (strcmp(line, "hint") == 0) { m_levelHint = val; }
        else if (strcmp(line, "player") == 0) {
            sscanf(val, "%f %f %f", &m_playerStart.x, &m_playerStart.y, &m_playerStart.z);
        } else if (strcmp(line, "goal") == 0) {
            sscanf(val, "%f %f %f %f", &m_goalPosition.x, &m_goalPosition.y, &m_goalPosition.z, &m_goalRadius);
        } else if (strcmp(line, "STATIC") == 0 || strcmp(line, "DYNAMIC") == 0) {
            Vector3 pos, size; int r,g,b,a; float mass;
            sscanf(val, "%f %f %f %f %f %f %d %d %d %d %f",
                &pos.x, &pos.y, &pos.z, &size.x, &size.y, &size.z, &r, &g, &b, &a, &mass);
            auto obj = std::make_unique<PhysicsObject>(pos, size, mass, strcmp(line, "STATIC") == 0);
            obj->SetColor({(unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a});
            obj->SetStartPosition(pos);
            m_objects.push_back(std::move(obj));
        }
    }
    fclose(f);
    m_completed = false;
    return true;
}

void LevelBase::Load() {
    m_completed = false;
}