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
    Vector3 GetPlayerPos() const { return m_playerPosition; }
    
private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    void Update(float dt);
    void Draw();
    void UpdatePlayer(float dt);
    void UpdateCamera(float dt);
    void DrawPlayer();
    void PushObjects(LevelBase* level);
    
    GameState m_state = GameState::MENU;
    TimeMode m_timeMode = TimeMode::LIVE;
    
    TimeRecorder m_recorder;
    
    std::vector<std::unique_ptr<LevelBase>> m_levels;
    int m_currentLevelIndex = 0;
    
    Camera3D m_camera;
    Vector3 m_playerPosition{0, 2, 0};
    Vector3 m_playerVelocity{0, 0, 0};
    bool m_playerOnGround = false;
    float m_playerRadius = 0.4f;
    float m_moveSpeed = 6.0f;
    float m_jumpForce = 8.0f;
    Vector3 m_cameraTarget{0, 1, 0};
    float m_cameraDistance = 15.0f;
    float m_cameraAngle = 0.0f;
    float m_cameraHeight = 8.0f;
    
    float m_levelTime = 0.0f;
    float m_accumulator = 0.0f;
    
    std::unique_ptr<UIManager> m_ui;
};