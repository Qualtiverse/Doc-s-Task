#include "GameManager.h"
#include "Level.h"
#include "Levels.h"
#include "UIManager.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <iostream>

// Forward declarations for status message helpers
void SetStatusMsg(const char* msg);
void DrawStatusMsg();

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
    
    strncpy(m_levelNameBuf, "My Level", sizeof(m_levelNameBuf)-1);
    m_levelNameBuf[sizeof(m_levelNameBuf)-1] = 0;
    m_levelNameLen = (int)strlen(m_levelNameBuf);
    m_levelHintBuf[0] = 0;
    m_levelHintLen = 0;
    
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
    if (m_state == GameState::BUILDING && newState != GameState::BUILDING) {
        ShowCursor();
    }
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
        if (IsKeyPressed(KEY_ENTER)) LoadLevel(0);
        if (IsKeyPressed(KEY_E)) EnterBuildMode();
        return;
    }
    
    if (m_state == GameState::BUILDING) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            ShowCursor();
            m_state = GameState::MENU;
            return;
        }
        UpdateBuildMode(dt);
        m_ui->Update(dt, m_state, m_timeMode, m_recorder, m_levels[m_currentLevelIndex].get());
        Vector3 forward;
        forward.x = cosf(m_buildCamPitch) * sinf(m_buildCamYaw);
        forward.y = sinf(m_buildCamPitch);
        forward.z = cosf(m_buildCamPitch) * cosf(m_buildCamYaw);
        m_camera.position = m_buildCamPos;
        m_camera.target = Vector3Add(m_buildCamPos, forward);
        m_camera.up = {0, 1, 0};
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
    
    if (IsKeyPressed(KEY_TAB) && m_state != GameState::LEVEL_COMPLETE) {
        if (m_state == GameState::PLAYING || m_state == GameState::RECORDING || m_state == GameState::REPLAYING) {
            EnterBuildMode();
            return;
        }
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
    } else if (m_state == GameState::BUILDING) {
        LevelBase* level = m_levels[m_currentLevelIndex].get();
        BeginMode3D(m_camera);
        if (level) level->Draw(m_camera);
        DrawGrid(40, 1.0f);
        DrawBuildMode();
        EndMode3D();
        m_ui->Draw(m_state, m_timeMode, m_recorder, level, m_levelTime);
    } else {
        LevelBase* level = m_levels[m_currentLevelIndex].get();
        BeginMode3D(m_camera);
        if (level) level->Draw(m_camera);
        DrawGrid(20, 1.0f);
        EndMode3D();
        DrawPlayer();
        m_ui->Draw(m_state, m_timeMode, m_recorder, level, m_levelTime);
    }
    
    DrawStatusMsg();
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
    
    if (m_state != GameState::BUILDING) {
        LevelBase* level = m_levels[m_currentLevelIndex].get();
        if (level && level->CheckPlayerGoal(m_playerPosition)) {
            ChangeState(GameState::LEVEL_COMPLETE);
        }
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

void GameManager::EnterBuildMode() {
    m_state = GameState::BUILDING;
    m_buildTool = BuildTool::PLACE_STATIC;
    m_selectedObject = -1;
    m_showBuildPalette = true;
    
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (level) {
        level->Load();
        Vector3 ps = level->GetPlayerStart();
        m_buildCamPos = Vector3Add(ps, Vector3{8, 8, 8});
        strncpy(m_levelNameBuf, level->EditorGetLevelName().c_str(), sizeof(m_levelNameBuf)-1);
        m_levelNameBuf[sizeof(m_levelNameBuf)-1] = 0;
        m_levelNameLen = (int)strlen(m_levelNameBuf);
        strncpy(m_levelHintBuf, level->EditorGetLevelHint().c_str(), sizeof(m_levelHintBuf)-1);
        m_levelHintBuf[sizeof(m_levelHintBuf)-1] = 0;
        m_levelHintLen = (int)strlen(m_levelHintBuf);
    }
    m_buildCamYaw = 0.8f;
    m_buildCamPitch = -0.5f;
    m_buildCamSpeed = 12.0f;
    HideCursor();
}

void GameManager::ExitBuildMode() {
    ShowCursor();
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (level) {
        level->Reset();
    }
    m_playerPosition = m_levels[m_currentLevelIndex]->GetPlayerStart();
    m_recorder.Clear();
    ChangeState(GameState::PLAYING);
}

Vector3 GameManager::GetMouseGroundPos() {
    Vector2 mp = GetMousePosition();
    Ray ray = GetMouseRay(mp, m_camera);
    if (ray.direction.y == 0) return m_playerPosition;
    float t = -ray.position.y / ray.direction.y;
    if (t < 0) return m_playerPosition;
    Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
    p.y = 0;
    p.x = roundf(p.x / m_buildSnap) * m_buildSnap;
    p.z = roundf(p.z / m_buildSnap) * m_buildSnap;
    return p;
}

int GameManager::PickObject() {
    Vector2 mp = GetMousePosition();
    Ray ray = GetMouseRay(mp, m_camera);
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (!level) return -1;
    float bestT = 1e9f;
    int best = -1;
    for (int i = 0; i < level->GetObjectCount(); i++) {
        auto* obj = level->EditorGetObject(i);
        if (!obj) continue;
        Vector3 half = Vector3Scale(obj->GetSize(), 0.5f);
        Vector3 min = Vector3Subtract(obj->GetPosition(), half);
        Vector3 max = Vector3Add(obj->GetPosition(), half);
        float tmin = -1e9f, tmax = 1e9f;
        for (int a = 0; a < 3; a++) {
            float* ro = (float*)&ray.position;
            float* rd = (float*)&ray.direction;
            float* bmin = (float*)&min;
            float* bmax = (float*)&max;
            float invD = 1.0f / rd[a];
            float t0 = (bmin[a] - ro[a]) * invD;
            float t1 = (bmax[a] - ro[a]) * invD;
            if (invD < 0) { float tmp = t0; t0 = t1; t1 = tmp; }
            if (t0 > tmin) tmin = t0;
            if (t1 < tmax) tmax = t1;
            if (tmin > tmax) goto skip;
        }
        if (tmin > 0 && tmin < bestT) { bestT = tmin; best = i; }
        skip:;
    }
    return best;
}

void GameManager::UpdateBuildMode(float dt) {
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (!level) return;

    // --- Free-flying camera ---
    if (!m_editingNameField && !m_editingHintField) {
        // Build direction vectors from yaw/pitch
        float yaw = m_buildCamYaw;
        float pitch = m_buildCamPitch;
        Vector3 forward = { cosf(pitch)*sinf(yaw), sinf(pitch), cosf(pitch)*cosf(yaw) };
        Vector3 flatFwd = { sinf(yaw), 0, cosf(yaw) };
        Vector3 right = { sinf(yaw - PI/2), 0, cosf(yaw - PI/2) };
        Vector3 up = { 0, 1, 0 };

        // Mouse look (right-click drag)
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            m_buildCamYaw -= delta.x * 0.004f;
            m_buildCamPitch += delta.y * 0.004f;
            m_buildCamPitch = Clamp(m_buildCamPitch, -1.5f, 1.5f);
        }

        // WASD movement (relative to camera)
        float speed = m_buildCamSpeed * dt;
        if (IsKeyDown(KEY_LEFT_SHIFT)) speed *= 3.0f;
        if (IsKeyDown(KEY_W)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(flatFwd, speed));
        if (IsKeyDown(KEY_S)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(flatFwd, -speed));
        if (IsKeyDown(KEY_D)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(right, speed));
        if (IsKeyDown(KEY_A)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(right, -speed));
        if (IsKeyDown(KEY_E)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(up, speed));
        if (IsKeyDown(KEY_Q)) m_buildCamPos = Vector3Add(m_buildCamPos, Vector3Scale(up, -speed));

        // Scroll to adjust speed
        float wheel = GetMouseWheelMove();
        if (wheel != 0) m_buildCamSpeed = Clamp(m_buildCamSpeed + wheel * 2.0f, 1.0f, 80.0f);
    }

    // --- Text input for name/hint ---
    if (m_editingNameField) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key < 127 && m_levelNameLen < 254) {
                m_levelNameBuf[m_levelNameLen++] = (char)key;
                m_levelNameBuf[m_levelNameLen] = 0;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && m_levelNameLen > 0) {
            m_levelNameBuf[--m_levelNameLen] = 0;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            m_editingNameField = false;
            level->EditorSetLevelName(m_levelNameBuf);
        }
    }
    if (m_editingHintField) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key < 127 && m_levelHintLen < 510) {
                m_levelHintBuf[m_levelHintLen++] = (char)key;
                m_levelHintBuf[m_levelHintLen] = 0;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && m_levelHintLen > 0) {
            m_levelHintBuf[--m_levelHintLen] = 0;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            m_editingHintField = false;
            level->EditorSetLevelHint(m_levelHintBuf);
        }
    }

    // --- Tool switching ---
    if (IsKeyPressed(KEY_ONE)) m_buildTool = BuildTool::PLACE_STATIC;
    if (IsKeyPressed(KEY_TWO)) m_buildTool = BuildTool::PLACE_DYNAMIC;
    if (IsKeyPressed(KEY_THREE)) m_buildTool = BuildTool::SET_PLAYER;
    if (IsKeyPressed(KEY_FOUR)) m_buildTool = BuildTool::SET_GOAL;
    if (IsKeyPressed(KEY_FIVE)) m_buildTool = BuildTool::DELETE;
    if (IsKeyPressed(KEY_SIX)) m_buildTool = BuildTool::SELECT;

    // --- Snap ---
    if (IsKeyPressed(KEY_COMMA)) m_buildSnap = fmaxf(0.25f, m_buildSnap * 0.5f);
    if (IsKeyPressed(KEY_PERIOD)) m_buildSnap = fminf(4.0f, m_buildSnap * 2.0f);

    // --- Toggle help ---
    if (IsKeyPressed(KEY_H)) m_ui->ToggleHelp();

    // --- Switch to play mode (test level) ---
    if (IsKeyPressed(KEY_TAB)) {
        ExitBuildMode();
        return;
    }

    // --- Save / Load ---
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_S)) {
            level->EditorSetLevelName(m_levelNameBuf);
            level->EditorSetLevelHint(m_levelHintBuf);
            if (level->EditorSaveToFile("level.lvl"))
                SetStatusMsg("Level saved to level.lvl");
            else
                SetStatusMsg("Failed to save!");
        }
        if (IsKeyPressed(KEY_L)) {
            if (level->EditorLoadFromFile("level.lvl")) {
                SetStatusMsg("Level loaded from level.lvl");
                strncpy(m_levelNameBuf, level->EditorGetLevelName().c_str(), sizeof(m_levelNameBuf)-1);
                m_levelNameBuf[sizeof(m_levelNameBuf)-1] = 0;
                m_levelNameLen = (int)strlen(m_levelNameBuf);
                strncpy(m_levelHintBuf, level->EditorGetLevelHint().c_str(), sizeof(m_levelHintBuf)-1);
                m_levelHintBuf[sizeof(m_levelHintBuf)-1] = 0;
                m_levelHintLen = (int)strlen(m_levelHintBuf);
            } else {
                SetStatusMsg("Failed to load!");
            }
        }
    }

    // --- Mouse interaction (only when not right-click-dragging camera) ---
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mp = GetMousePosition();
        bool uiHover = mp.x < 280;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !uiHover) {
            // Click on name/hint fields in UI area
            if (mp.x < 550 && mp.y > 10 && mp.y < 100) {
                Rectangle nameRect = {280, 30, 240, 24};
                Rectangle hintRect = {280, 76, 240, 24};
                if (CheckCollisionPointRec(mp, nameRect)) {
                    m_editingNameField = true; m_editingHintField = false;
                    return;
                }
                if (CheckCollisionPointRec(mp, hintRect)) {
                    m_editingHintField = true; m_editingNameField = false;
                    return;
                }
                m_editingNameField = false; m_editingHintField = false;
                return;
            }
            // Click mode buttons
            if (mp.x >= 280 && mp.x <= 520) {
                for (int i = 0; i < 6; i++) {
                    int by = 125 + i * 26;
                    if (mp.y >= by && mp.y <= by + 24) {
                        m_buildTool = (BuildTool)i;
                        return;
                    }
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !uiHover) {
            Vector3 gp = GetMouseGroundPos();

            if (m_buildTool == BuildTool::PLACE_STATIC || m_buildTool == BuildTool::PLACE_DYNAMIC) {
                gp.y = 0.5f;
                Color c = m_buildTool == BuildTool::PLACE_STATIC ?
                    (Color){120,120,130,255} : (Color){220,60,60,255};
                int idx = level->EditorAddObject(gp, Vector3{m_buildSnap,m_buildSnap,m_buildSnap},
                    m_buildTool == BuildTool::PLACE_STATIC ? 0 : 3.0f,
                    m_buildTool == BuildTool::PLACE_STATIC, c);
                m_selectedObject = idx;
                SetStatusMsg("Placed object");
            }
            else if (m_buildTool == BuildTool::SET_PLAYER) {
                gp.y = 2.0f;
                level->EditorSetPlayerStart(gp);
                SetStatusMsg("Player start set");
            }
            else if (m_buildTool == BuildTool::SET_GOAL) {
                gp.y = 1.0f;
                level->EditorSetGoal(gp, 1.5f);
                SetStatusMsg("Goal set");
            }
            else if (m_buildTool == BuildTool::DELETE) {
                int picked = PickObject();
                if (picked >= 0) {
                    level->EditorRemoveObject(picked);
                    m_selectedObject = -1;
                    SetStatusMsg("Deleted object");
                }
            }
            else if (m_buildTool == BuildTool::SELECT) {
                int picked = PickObject();
                m_selectedObject = picked;
                if (picked >= 0) {
                    char buf[64]; snprintf(buf, sizeof(buf), "Selected object %d", picked);
                    SetStatusMsg(buf);
                } else {
                    SetStatusMsg("Nothing selected");
                }
            }
        }
    }

    // --- Modify selected object ---
    if (m_selectedObject >= 0 && m_selectedObject < level->GetObjectCount()) {
        auto* obj = level->EditorGetObject(m_selectedObject);
        if (obj) {
            Vector3 pos = obj->GetPosition();
            Vector3 size = obj->GetSize();
            bool changed = false;

            if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
                size.x += m_buildSnap; size.y += m_buildSnap; size.z += m_buildSnap;
                changed = true;
            }
            if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
                size.x = fmaxf(0.5f, size.x - m_buildSnap);
                size.y = fmaxf(0.5f, size.y - m_buildSnap);
                size.z = fmaxf(0.5f, size.z - m_buildSnap);
                changed = true;
            }

            float moveAmt = IsKeyDown(KEY_LEFT_SHIFT) ? 0.1f : 0.5f;
            if (IsKeyDown(KEY_UP)) { pos.z -= moveAmt; changed = true; }
            if (IsKeyDown(KEY_DOWN)) { pos.z += moveAmt; changed = true; }
            if (IsKeyDown(KEY_LEFT)) { pos.x -= moveAmt; changed = true; }
            if (IsKeyDown(KEY_RIGHT)) { pos.x += moveAmt; changed = true; }
            if (IsKeyDown(KEY_PAGE_UP)) { pos.y += moveAmt; changed = true; }
            if (IsKeyDown(KEY_PAGE_DOWN)) { pos.y = fmaxf(0.25f, pos.y - moveAmt); changed = true; }

            if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                level->EditorRemoveObject(m_selectedObject);
                m_selectedObject = -1;
                SetStatusMsg("Deleted");
                return;
            }

            if (changed) {
                obj->SetPosition(pos);
                obj->SetSize(size);
                obj->SetStartPosition(pos);
            }
        }
    }
}

void GameManager::DrawBuildMode() {
    LevelBase* level = m_levels[m_currentLevelIndex].get();
    if (!level) return;

    // Ghost object at mouse position
    if (m_buildTool == BuildTool::PLACE_STATIC || m_buildTool == BuildTool::PLACE_DYNAMIC) {
        Vector3 ghost = GetMouseGroundPos();
        ghost.y = 0.5f;
        Color c = m_buildTool == BuildTool::PLACE_STATIC ?
            ColorAlpha(GRAY, 0.4f) : ColorAlpha(RED, 0.4f);
        DrawCube(ghost, m_buildSnap, m_buildSnap, m_buildSnap, c);
        DrawCubeWires(ghost, m_buildSnap, m_buildSnap, m_buildSnap, ColorAlpha(WHITE, 0.3f));
    }

    // Highlight selected object
    if (m_selectedObject >= 0 && m_selectedObject < level->GetObjectCount()) {
        auto* obj = level->EditorGetObject(m_selectedObject);
        if (obj) {
            Vector3 pos = obj->GetPosition();
            Vector3 size = obj->GetSize();
            DrawCubeWires(pos, size.x+0.05f, size.y+0.05f, size.z+0.05f, YELLOW);
            // Draw position handle on top
            DrawSphere(pos + Vector3{0, size.y*0.5f + 0.3f, 0}, 0.15f, YELLOW);
        }
    }

    // Player start marker
    Vector3 pp = level->GetPlayerStart();
    DrawCylinder(pp, 0.4f, 0.4f, 1.0f, 8, BLUE);
    DrawCylinderWires(pp, 0.4f, 0.4f, 1.0f, 8, ColorAlpha(BLUE, 0.5f));

    // Goal marker
    Vector3 gp = level->GetGoalPos();
    float gr = level->GetGoalRadius();
    DrawCylinder(gp, gr, gr, 0.2f, 16, ColorAlpha(GREEN, 0.3f));
    DrawCylinderWires(gp, gr, gr, 2.0f, 8, GREEN);
}

// Helper to set status message (defined locally for simplicity)
static char s_statusMsg[256] = "";
static float s_statusTimer = 0;

void SetStatusMsg(const char* msg) {
    strncpy(s_statusMsg, msg, sizeof(s_statusMsg)-1);
    s_statusMsg[sizeof(s_statusMsg)-1] = 0;
    s_statusTimer = 3.0f;
}

void DrawStatusMsg() {
    if (s_statusTimer > 0) {
        s_statusTimer -= GetFrameTime();
        DrawText(s_statusMsg, 10, SCREEN_HEIGHT-24, 16, ColorAlpha(GREEN, fminf(1, s_statusTimer)));
    }
}