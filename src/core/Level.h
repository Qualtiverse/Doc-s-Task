#pragma once

#include "CoreTypes.h"
#include <vector>
#include <memory>

class Level : public ILevel {
public:
    Level() = default;
    virtual ~Level() = default;
    
    void Load() override;
    void Unload() override;
    void Update(float dt, TimeRecorder& recorder) override;
    void Draw(const raylib::Camera3D& camera) override;
    void DrawUI() override;
    bool IsComplete() override;
    void Reset() override;
    
    virtual const char* GetName() const override { return "Base Level"; }
    virtual const char* GetHint() const override { return "Press R to record, P to replay"; }
    
    virtual raylib::Vector3 GetPlayerStart() const { return {0, 2, 0}; }
    virtual int GetObjectCount() const { return (int)m_objects.size(); }
    virtual void GetObjectStates(std::vector<TransformData>& states) const;
    virtual void SetObjectStates(const std::vector<TransformData>& states);
    
protected:
    std::vector<std::unique_ptr<PhysicsObject>> m_objects;
    raylib::Vector3 m_playerStart{0, 2, 0};
    raylib::Vector3 m_goalPosition{0, 1, 10};
    float m_goalRadius = 1.5f;
    raylib::Color m_goalColor = GREEN;
    bool m_completed = false;
    float m_timeLimit = 60.0f;
    
    void AddObject(std::unique_ptr<PhysicsObject> obj);
    void AddBox(raylib::Vector3 pos, raylib::Vector3 size, float mass = 1.0f, bool dynamic = true, raylib::Color color = WHITE);
    void AddStaticBox(raylib::Vector3 pos, raylib::Vector3 size, raylib::Color color = GRAY);
    void CreateGround(float size = 20.0f);
    void CreateGoal(raylib::Vector3 pos, float radius = 1.5f);
    bool CheckGoal(raylib::Vector3 playerPos) const;
};