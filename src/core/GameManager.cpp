#include "GameManager.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "Level5.h"
#include "UIManager.h"
#include "raylib-cpp.hpp"
#include <iostream>

GameManager& GameManager::Instance() {
    static GameManager instance;
    return instance;
}

bool GameManager::Initialize() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Doc's Task - Time Manipulation Puzzle");
    SetTargetFPS(TARGET_FPS);
    
    InitAudioDevice();
    
    m_camera = raylib::Camera3D({0});
    m_camera.position = {10, 10, 10};
    m_camera.target = {0, 0, 0};
    m_camera.up = {0, 1, 0};
    m_camera.fovy = 45.0f;
    m_camera.projection = CAMERA_PERSPECTIVE;
    
    m_ui = std::make_unique<UIManager>();
    
    m_levels.push_back(std::make_unique<Level1>());
    m_levels.push_back(std::make_unique<Level2>());
    m_levels.push_back(std::make_unique<Level3>());
    m_levels.push_back(std::make_unique<Level4>());
    m_levels.push_back(std::make_unique<Level5>());
    
    LoadLevel(0);
    
    return true;
}

void GameManager::Run() {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;
        
        Update(dt);
        Draw();
    }
    
    Shutdown();
}

void GameManager::Shutdown() {
    for (auto& level : m_levels) {
        level->Unload();
    }
    CloseAudioDevice();
    CloseWindow();
}

void GameManager::ChangeState(GameState newState) {
    m_state = newState;
    if (newState == GameState::PLAYING) {
        m_timeMode = TimeMode::LIVE;
        m_recorder.StopReplay();
        m_recorder.StopRecording();
    }
}

void GameManager::SetTimeMode(TimeMode mode) {
    m_timeMode = mode;
    if (mode == TimeMode::RECORDING) {
        m_recorder.StartRecording(m_levels[m_currentLevelIndex]->GetObjectCount());
    } else if (mode == TimeMode::REPLAYING) {
        m_recorder.StartReplay();
    } else {
        m_recorder.StopRecording();
        m_recorder.StopReplay();
    }
}

void GameManager::LoadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= (int)m_levels.size()) return;
    
    if (m_currentLevelIndex >= 0 && m_currentLevelIndex < (int)m_levels.size()) {
        m_levels[m_currentLevelIndex]->Unload();
    }
    
    m_currentLevelIndex = levelIndex;
    m_levels[m_currentLevelIndex]->Load();
    m_levelTime = 0;
    m_playerPosition = m_levels[m_currentLevelIndex]->GetPlayerStart();
    m_recorder.Clear();
    ChangeState(GameState::PLAYING);
}

void GameManager::ReloadCurrentLevel() {
    LoadLevel(m_currentLevelIndex);
}

void GameManager::NextLevel() {
    int next = (m_currentLevelIndex + 1) % m_levels.size();
    LoadLevel(next);
}

void GameManager::PreviousLevel() {
    int prev = (m_currentLevelIndex - 1 + m_levels.size()) % m_levels.size();
    LoadLevel(prev);
}

void GameManager::Update(float dt) {
    if (m_state == GameState::MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            LoadLevel(0);
        }
        return;
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (m_state == GameState::PLAYING || m_state == GameState::RECORDING || m_state == GameState::REPLAYING) {
            ChangeState(GameState::PAUSED);
        } else if (m_state == GameState::PAUSED) {
            ChangeState(GameState::PLAYING);
        } else if (m_state == GameState::MENU) {
            LoadLevel(0);
        }
        return;
    }
    
    if (m_state == GameState::PAUSED) return;
    
    if (IsKeyPressed(KEY_TAB)) {
        NextLevel();
    }
    
    if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) {
        ReloadCurrentLevel();
        return;
    }
    
    if (m_state == GameState::LEVEL_COMPLETE) {
        if (IsKeyPressed(KEY_TAB)) NextLevel();
        if (IsKeyPressed(KEY_R)) ReloadCurrentLevel();
        return;
    }
    
    Level* level = m_levels[m_currentLevelIndex].get();
    if (!level) return;
    
    level->ProcessInput(dt, m_timeMode);
    
    if (m_timeMode == TimeMode::LIVE) {
        if (IsKeyPressed(KEY_R)) {
            if (m_recorder.IsRecording()) {
                m_recorder.StopRecording();
                SetTimeMode(TimeMode::LIVE);
                ChangeState(GameState::PLAYING);
            } else {
                m_recorder.StartRecording(level->GetObjectCount());
                SetTimeMode(TimeMode::RECORDING);
                ChangeState(GameState::RECORDING);
            }
        }
        
        if (IsKeyPressed(KEY_P) && m_recorder.GetFrameCount() > 0) {
            m_levelTime = 0;
            level->Reset();
            m_recorder.StartReplay();
            SetTimeMode(TimeMode::REPLAYING);
            ChangeState(GameState::REPLAYING);
        }
    }
    else if (m_timeMode == TimeMode::RECORDING) {
        if (IsKeyPressed(KEY_R)) {
            m_recorder.StopRecording();
            SetTimeMode(TimeMode::LIVE);
            ChangeState(GameState::PLAYING);
        }
    }
    else if (m_timeMode == TimeMode::REPLAYING) {
        if (IsKeyPressed(KEY_T)) {
            m_recorder.StopReplay();
            SetTimeMode(TimeMode::LIVE);
            ChangeState(GameState::PLAYING);
        }
    }
    
    if (IsKeyPressed(KEY_H)) {
        m_ui->ToggleHelp();
    }
    
    m_accumulator += dt;
    while (m_accumulator >= FIXED_TIMESTEP) {
        if (m_state == GameState::PLAYING || m_state == GameState::RECORDING) {
            m_levelTime += FIXED_TIMESTEP;
            
            level->Update(FIXED_TIMESTEP, m_recorder);
            
            if (m_timeMode == TimeMode::RECORDING) {
                std::vector<TransformData> states;
                level->GetObjectStates(states);
                m_recorder.RecordFrame(states, m_levelTime);
            }
            
            if (level->IsComplete()) {
                ChangeState(GameState::LEVEL_COMPLETE);
            }
        }
        else if (m_timeMode == TimeMode::REPLAYING) {
            m_levelTime += FIXED_TIMESTEP;
            
            std::vector<TransformData> replayStates;
            if (m_recorder.GetReplayFrame(replayStates, m_levelTime)) {
                level->SetObjectStates(replayStates);
            } else {
                m_recorder.StopReplay();
                SetTimeMode(TimeMode::LIVE);
                ChangeState(GameState::PLAYING);
            }
        }
        
        m_accumulator -= FIXED_TIMESTEP;
    }
    
    m_ui->Update(dt, m_state, m_timeMode, m_recorder, level);
    UpdateCamera(dt);
}

void GameManager::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    if (m_state == GameState::MENU) {
        m_ui->Draw(m_state, m_timeMode, m_recorder, nullptr, m_levelTime);
    } else {
        Level* level = m_levels[m_currentLevelIndex].get();
        
        BeginMode3D(m_camera);
        if (level) level->Draw(m_camera);
        DrawGrid(20, 1.0f);
        EndMode3D();
        
        m_ui->Draw(m_state, m_timeMode, m_recorder, level, m_levelTime);
    }
    
    EndDrawing();
}

void GameManager::UpdateCamera(float dt) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        raylib::Vector2 delta = GetMouseDelta();
        m_cameraAngle -= delta.x * 0.003f;
        m_cameraHeight += delta.y * 0.05f;
        m_cameraHeight = Clamp(m_cameraHeight, 2.0f, 20.0f);
    }
    
    float wheel = GetMouseWheelMove();
    m_cameraDistance -= wheel * 1.0f;
    m_cameraDistance = Clamp(m_cameraDistance, 5.0f, 30.0f);
    
    Level* level = m_levels[m_currentLevelIndex].get();
    if (level) {
        m_cameraTarget = level->GetPlayerStart();
    }
    
    m_camera.position.x = m_cameraTarget.x + cosf(m_cameraAngle) * m_cameraDistance;
    m_camera.position.z = m_cameraTarget.z + sinf(m_cameraAngle) * m_cameraDistance;
    m_camera.position.y = m_cameraTarget.y + m_cameraHeight;
    m_camera.target = m_cameraTarget;
}