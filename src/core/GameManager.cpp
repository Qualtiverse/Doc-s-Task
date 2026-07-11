#include "GameManager.h"
#include "Level.h"
#include "Levels.h"
#include "UIManager.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <iostream>

GameManager& GameManager::Instance() {
    static GameManager instance;
    return instance;
}

bool GameManager::Initialize() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Doc's Task - Time Manipulation Puzzle");
    SetTargetFPS(60);
    
    InitAudioDevice();
    
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
        m_recorder.StopReplay();
        m_recorder.StopRecording();
        m_timeMode = TimeMode::LIVE;
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
    int next = (m_currentLevelIndex + 1) % (int)m_levels.size();
    LoadLevel(next);
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
    
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (!level) return;
    
    level->ProcessInput(dt, m_timeMode);
    
    if (m_timeMode == TimeMode::LIVE) {
        if (IsKeyPressed(KEY_R)) {
            if (m_recorder.IsRecording()) {
                m_recorder.StopRecording();
                m_timeMode = TimeMode::LIVE;
                ChangeState(GameState::PLAYING);
            } else {
                m_recorder.StartRecording(level->GetObjectCount());
                m_timeMode = TimeMode::RECORDING;
                ChangeState(GameState::RECORDING);
            }
        }
        
        if (IsKeyPressed(KEY_P) && m_recorder.GetFrameCount() > 0) {
            m_levelTime = 0;
            level->Reset();
            m_recorder.StartReplay();
            m_timeMode = TimeMode::REPLAYING;
            ChangeState(GameState::REPLAYING);
        }
    }
    else if (m_timeMode == TimeMode::RECORDING) {
        if (IsKeyPressed(KEY_R)) {
            m_recorder.StopRecording();
            m_timeMode = TimeMode::LIVE;
            ChangeState(GameState::PLAYING);
        }
    }
    else if (m_timeMode == TimeMode::REPLAYING) {
        if (IsKeyPressed(KEY_T)) {
            m_recorder.StopReplay();
            m_timeMode = TimeMode::LIVE;
            ChangeState(GameState::PLAYING);
        }
    }
    
        if (IsKeyPressed(KEY_H)) {
            m_ui->ToggleHelp();
        }
        
        if (m_state == GameState::PLAYING || m_state == GameState::RECORDING || m_state == GameState::REPLAYING) {
            UpdatePlayer(dt);
        }
        
        m_accumulator += dt;
        while (m_accumulator >= FIXED_TIMESTEP) {
            if (m_state == GameState::PLAYING || m_state == GameState::RECORDING) {
                m_levelTime += FIXED_TIMESTEP;
                
                if (m_timeMode == TimeMode::LIVE || m_timeMode == TimeMode::RECORDING) {
                    PushObjects(level);
                }
                level->Update(FIXED_TIMESTEP, m_recorder);
                
                if (m_timeMode == TimeMode::RECORDING) {
                    std::vector<TransformData> states;
                    level->GetObjectStates(states);
                    m_recorder.RecordFrame(states, m_levelTime);
                }
            }
            else if (m_timeMode == TimeMode::REPLAYING) {
                m_levelTime += FIXED_TIMESTEP;
                
                std::vector<TransformData> replayStates;
                if (m_recorder.GetReplayFrame(replayStates, m_levelTime)) {
                    level->SetObjectStates(replayStates);
                } else {
                    m_recorder.StopReplay();
                    m_timeMode = TimeMode::LIVE;
                    ChangeState(GameState::PLAYING);
                }
            }
            
            m_accumulator -= FIXED_TIMESTEP;
        }
    
    m_ui->Update(dt, m_state, m_timeMode, m_recorder, m_levels[m_currentLevelIndex].get());
    UpdateCamera(dt);
}

void GameManager::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    if (m_state == GameState::MENU) {
        m_ui->Draw(m_state, m_timeMode, m_recorder, nullptr, m_levelTime);
    } else {
        LevelBase* level = m_levels[m_currentLevelIndex].get();
        
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
        Vector2 delta = GetMouseDelta();
        m_cameraAngle -= delta.x * 0.003f;
        m_cameraHeight += delta.y * 0.05f;
        m_cameraHeight = Clamp(m_cameraHeight, 2.0f, 20.0f);
    }
    
    float wheel = GetMouseWheelMove();
    m_cameraDistance -= wheel * 1.0f;
    m_cameraDistance = Clamp(m_cameraDistance, 5.0f, 30.0f);
    
    m_camera.position.x = m_cameraTarget.x + cosf(m_cameraAngle) * m_cameraDistance;
    m_camera.position.z = m_cameraTarget.z + sinf(m_cameraAngle) * m_cameraDistance;
    m_camera.position.y = m_cameraTarget.y + m_cameraHeight;
    m_camera.target = m_cameraTarget;
}

void GameManager::UpdatePlayer(float dt) {
    if (m_state == GameState::PAUSED || m_state == GameState::LEVEL_COMPLETE || m_state == GameState::MENU) return;
    
    Vector3 moveDir{0, 0, 0};
    if (IsKeyDown(KEY_W)) moveDir.z -= 1;
    if (IsKeyDown(KEY_S)) moveDir.z += 1;
    if (IsKeyDown(KEY_A)) moveDir.x -= 1;
    if (IsKeyDown(KEY_D)) moveDir.x += 1;
    
    float len = sqrtf(moveDir.x*moveDir.x + moveDir.z*moveDir.z);
    if (len > 0) {
        moveDir.x /= len;
        moveDir.z /= len;
    }
    
    m_playerVelocity.x = moveDir.x * m_moveSpeed;
    m_playerVelocity.z = moveDir.z * m_moveSpeed;
    
    if (m_playerOnGround && IsKeyPressed(KEY_SPACE)) {
        m_playerVelocity.y = m_jumpForce;
        m_playerOnGround = false;
    }
    
    m_playerVelocity.y -= 9.81f * dt;
    
    m_playerPosition.x += m_playerVelocity.x * dt;
    m_playerPosition.y += m_playerVelocity.y * dt;
    m_playerPosition.z += m_playerVelocity.z * dt;
    
    if (m_playerPosition.y < 0.5f) {
        m_playerPosition.y = 0.5f;
        m_playerVelocity.y = 0;
        m_playerOnGround = true;
    }
    
    m_cameraTarget = m_playerPosition;
    m_cameraTarget.y += 1.0f;
    
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (level && level->CheckGoal(m_playerPosition)) {
        ChangeState(GameState::LEVEL_COMPLETE);
    }
}

void GameManager::DrawPlayer() {
    if (m_state == GameState::MENU) return;
    Vector3 pos = m_playerPosition;
    DrawCube(pos, m_playerRadius*2, 1.0f, m_playerRadius*2, BLUE);
    DrawCubeWires(pos, m_playerRadius*2, 1.0f, m_playerRadius*2, DARKBLUE);
}

void GameManager::PushObjects(LevelBase* level) {
    if (!level) return;
    for (int i = 0; i < level->GetObjectCount(); i++) {
        std::vector<TransformData> states;
        level->GetObjectStates(states);
        if (i >= (int)states.size()) break;
        Vector3 objPos = states[i].position;
        float dx = m_playerPosition.x - objPos.x;
        float dz = m_playerPosition.z - objPos.z;
        float dist = sqrtf(dx*dx + dz*dz);
        float pushDist = m_playerRadius + 0.5f;
        if (dist < pushDist && dist > 0.001f) {
            float pushForce = 5.0f;
            dx /= dist;
            dz /= dist;
            states[i].velocity.x += dx * pushForce * FIXED_TIMESTEP;
            states[i].velocity.z += dz * pushForce * FIXED_TIMESTEP;
        }
        level->SetObjectStates(states);
    }
}