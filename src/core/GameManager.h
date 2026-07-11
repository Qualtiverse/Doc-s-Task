#pragma once

#include "CoreTypes.h"
#include <memory>
#include <vector>

class LevelBase;
class UIManager;

class GameManager {
public:
    static GameManager& Instance();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    void ChangeState(GameState newState);
    GameState GetState() const { return m_state; }
    
    void LoadLevel(int levelIndex);
    void ReloadCurrentLevel();
    void NextLevel();
    
    Camera3D& GetCamera() { return m_camera; }
    
private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    void Update(float dt);
    void Draw();
    void UpdateCamera(float dt);
    
    GameState m_state = GameState::MENU;
    TimeMode m_timeMode = TimeMode::LIVE;
    
    TimeRecorder m_recorder;
    
    std::vector<std::unique_ptr<LevelBase>> m_levels;
    int m_currentLevelIndex = 0;
    
    Camera3D m_camera;
    Vector3 m_playerPosition{0, 2, 0};
    Vector3 m_cameraTarget{0, 1, 0};
    float m_cameraDistance = 15.0f;
    float m_cameraAngle = 0.0f;
    float m_cameraHeight = 8.0f;
    
    float m_levelTime = 0.0f;
    float m_accumulator = 0.0f;
    
    std::unique_ptr<UIManager> m_ui;
};