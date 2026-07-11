#pragma once

#include "CoreTypes.h"
#include <vector>
#include <memory>

// Base implementation of the ILevel interface from CoreTypes.h
class LevelBase : public ILevel {
public:
    LevelBase() = default;
    virtual ~LevelBase() = default;
    
    void Load() override;
    void Unload() override;
    void Update(float dt, TimeRecorder& recorder) override;
    void Draw(const Camera3D& camera) override;
    void DrawUI() override;
    void ProcessInput(float dt, TimeMode mode) override {}
    bool IsComplete() const override;
    void Reset() override;
    
    const char* GetName() const override { return "Base Level"; }
    const char* GetHint() const override { return "Press R to record, P to replay"; }
    
    Vector3 GetPlayerStart() const override { return m_playerStart; }
    int GetObjectCount() const override { return (int)m_objects.size(); }
    void GetObjectStates(std::vector<TransformData>& states) const override;
    void SetObjectStates(const std::vector<TransformData>& states) override;
    
    // Editor / game methods
    bool CheckPlayerGoal(Vector3 playerPos) const { return CheckGoal(playerPos); }
    
    // Editor methods
    int EditorAddObject(Vector3 pos, Vector3 size, float mass, bool isStatic, Color color);
    void EditorRemoveObject(int index);
    void EditorClearObjects();
    void EditorSetGoal(Vector3 pos, float radius);
    void EditorSetPlayerStart(Vector3 pos);
    void EditorSetLevelName(const char* name) { m_levelName = name; }
    void EditorSetLevelHint(const char* hint) { m_levelHint = hint; }
    const std::string& EditorGetLevelName() const { return m_levelName; }
    const std::string& EditorGetLevelHint() const { return m_levelHint; }
    PhysicsObject* EditorGetObject(int index);
    bool EditorSaveToFile(const char* path);
    bool EditorLoadFromFile(const char* path);
    Vector3 GetGoalPos() const { return m_goalPosition; }
    float GetGoalRadius() const { return m_goalRadius; }

protected:
    std::vector<std::unique_ptr<PhysicsObject>> m_objects;
    Vector3 m_playerStart{0, 2, 0};
    Vector3 m_goalPosition{0, 1, 10};
    float m_goalRadius = 1.5f;
    Color m_goalColor = GREEN;
    bool m_completed = false;
    float m_timeLimit = 60.0f;
    std::string m_levelName = "Untitled";
    std::string m_levelHint = "";
    
    void AddObject(std::unique_ptr<PhysicsObject> obj);
    void AddBox(Vector3 pos, Vector3 size, float mass = 1.0f, bool dynamic = true, Color color = WHITE);
    void AddStaticBox(Vector3 pos, Vector3 size, Color color = GRAY);
    void CreateGround(float size = 20.0f);
    void CreateGoal(Vector3 pos, float radius = 1.5f);
    bool CheckGoal(Vector3 playerPos) const;
};