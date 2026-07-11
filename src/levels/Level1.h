#pragma once

#include "CoreTypes.h"

class Level1 : public Level {
public:
    void Load() override;
    void Unload() override;
    void Update(float dt, TimeRecorder& recorder) override;
    void Draw(const raylib::Camera3D& camera) override;
    void DrawUI() override;
    bool IsComplete() const override;
    void Reset() override;
    const char* GetName() const override { return "The Button"; }
    const char* GetHint() const override { return "Press the button, then replay to hold it while you pass"; }
    
    void ProcessInput(float dt, TimeMode mode);
    int GetObjectCount() const { return (int)m_objects.size(); }
    void GetObjectStates(std::vector<TransformData>& states) const;
    void SetObjectStates(const std::vector<TransformData>& states);
    raylib::Vector3 GetPlayerStart() const { return m_playerStart; }
    
private:
    void CreateLevelGeometry();
    void CheckButtonPress();
    void CheckGoalReached();
    
    PhysicsObject* m_button = nullptr;
    PhysicsObject* m_door = nullptr;
    PhysicsObject* m_player = nullptr;
    PhysicsObject* m_ghost = nullptr;
    
    bool m_buttonPressed = false;
    bool m_doorOpen = false;
    bool m_levelComplete = false;
    float m_doorHeight = 0;
    float m_targetDoorHeight = 0;
    raylib::Vector3 m_buttonPos{0, 0.5f, -5};
    raylib::Vector3 m_doorPos{0, 2.5f, 5};
    raylib::Vector3 m_goalPos{0, 0.5f, 10};
};