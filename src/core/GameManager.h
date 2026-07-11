#pragma once

#include "CoreTypes.h"
#include <memory>
#include <vector>

class GameManager {
public:
    static GameManager& Instance();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    void ChangeState(GameState newState);
    void SetTimeMode(TimeMode mode);
    GameState GetState() const { return m_state; }
    TimeMode GetTimeMode() const { return m_timeMode; }
    
    void LoadLevel(int levelIndex);
    void ReloadCurrentLevel();
    void NextLevel();
    void PreviousLevel();
    int GetCurrentLevelIndex() const { return m_currentLevelIndex; }
    Level* GetCurrentLevel() { return m_levels[m_currentLevelIndex].get(); }
    int GetLevelCount() const { return (int)m_levels.size(); }
    
    TimeRecorder& GetRecorder() { return m_recorder; }
    raylib::Camera3D& GetCamera() { return m_camera; }
    raylib::Vector3 GetPlayerPosition() const { return m_playerPosition; }
    void SetPlayerPosition(const raylib::Vector3& pos) { m_playerPosition = pos; }
    
    void TogglePause() { m_paused = !m_paused; }
    bool IsPaused() const { return m_paused; }
    
    float GetLevelTime() const { return m_levelTime; }
    void ResetLevelTimer() { m_levelTime = 0.0f; }
    
private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    void Update(float dt);
    void Draw();
    void DrawUI();
    void ProcessInput(float dt);
    
    void InitCamera();
    void UpdateCamera(float dt);
    void RegisterLevels();
    
    GameState m_state = GameState::MENU;
    TimeMode m_timeMode = TimeMode::LIVE;
    bool m_paused = false;
    
    TimeRecorder m_recorder;
    
    std::vector<std::unique_ptr<Level>> m_levels;
    int m_currentLevelIndex = 0;
    
    raylib::Camera3D m_camera;
    raylib::Vector3 m_playerPosition{0, 2, 0};
    raylib::Vector3 m_cameraTarget{0, 1, 0};
    float m_cameraDistance = 10.0f;
    float m_cameraAngle = 0.0f;
    float m_cameraHeight = 5.0f;
    
    float m_levelTime = 0.0f;
    float m_accumulator = 0.0f;
    
    // UI state
    bool m_showHelp = true;
    float m_helpTimer = 5.0f;
};