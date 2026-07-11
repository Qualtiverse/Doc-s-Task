#include "UIManager.h"
#include "Level.h"
#include "raylib-cpp.hpp"
#include <string>

UIManager::UIManager() {}

void UIManager::Update(float dt, GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level) {
    if (m_showHelp) {
        m_helpTimer -= dt;
        if (m_helpTimer <= 0) m_showHelp = false;
    }
    
    if (timeMode == TimeMode::RECORDING) {
        m_recordingFlashTimer += dt;
        if (m_recordingFlashTimer > 1.0f) m_recordingFlashTimer = 0;
    }
    
    if (m_draggingTimeline && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        m_draggingTimeline = false;
    }
}

void UIManager::Draw(GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level, float levelTime) {
    DrawMainUI(state, timeMode, recorder, level, levelTime);
    
    if (state == GameState::MENU) DrawMenuUI();
    if (state == GameState::PAUSED) DrawPauseUI();
    if (state == GameState::LEVEL_COMPLETE) DrawLevelCompleteUI();
    if (m_showHelp) DrawHelpUI();
}

void UIManager::DrawMainUI(GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level, float levelTime) {
    int levelNum = 1;
    
    DrawText(TextFormat("Doc's Task - Level %d", levelNum), 20, 20, 28, DARKGRAY);
    DrawText(level ? level->GetName() : "Loading...", 20, 55, 20, DARKGRAY);
    
    DrawModeIndicator(timeMode);
    
    if (timeMode == TimeMode::RECORDING) {
        Color flashColor = (int)(m_recordingFlashTimer * 10) % 2 ? RED : MAROON;
        DrawCircle(SCREEN_WIDTH - 30, 30, 12, flashColor);
        DrawText("REC", SCREEN_WIDTH - 70, 22, 16, flashColor);
        DrawText(TextFormat("Frames: %d", recorder.GetFrameCount()), SCREEN_WIDTH - 180, 22, 16, RED);
    }
    
    if (recorder.GetFrameCount() > 0) {
        DrawTimeline(recorder, levelTime);
    }
    
    if (level && level->GetHint()) {
        DrawText(level->GetHint(), 20, SCREEN_HEIGHT - 30, 18, DARKGRAY);
    }
    
    DrawControlsHint();
    
    DrawFPS(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 30);
}

void UIManager::DrawModeIndicator(TimeMode mode) {
    const char* modeStr = "";
    Color modeColor = DARKGRAY;
    
    switch (mode) {
        case TimeMode::LIVE: modeStr = "LIVE"; modeColor = GREEN; break;
        case TimeMode::RECORDING: modeStr = "RECORDING"; modeColor = RED; break;
        case TimeMode::REPLAYING: modeStr = "REPLAYING"; modeColor = BLUE; break;
    }
    
    DrawRectangleRounded({15, 85, 150, 35}, 0.3f, 8, Fade(modeColor, 0.2f));
    DrawRectangleRoundedLines({15, 85, 150, 35}, 0.3f, 8, modeColor);
    DrawText(modeStr, 25, 92, 22, modeColor);
}

void UIManager::DrawTimeline(TimeRecorder& recorder, float levelTime) {
    float timelineWidth = 400;
    float timelineHeight = 24;
    float timelineX = (SCREEN_WIDTH - timelineWidth) / 2;
    float timelineY = SCREEN_HEIGHT - 50;
    
    float duration = recorder.GetRecordingDuration();
    if (duration <= 0) duration = 1;
    
    DrawRectangle(timelineX, timelineY, timelineWidth, timelineHeight, Fade(DARKGRAY, 0.3f));
    DrawRectangleLines(timelineX, timelineY, timelineWidth, timelineHeight, DARKGRAY);
    
    int frameCount = recorder.GetFrameCount();
    for (int i = 0; i < frameCount; i++) {
        float x = timelineX + (i / (float)std::max(1, frameCount - 1)) * timelineWidth;
        DrawLine(x, timelineY + 2, x, timelineY + timelineHeight - 2, Fade(LIGHTGRAY, 0.5f));
    }
    
    float progress = levelTime / duration;
    float progressX = timelineX + progress * timelineWidth;
    DrawRectangle(timelineX, timelineY, (int)progressX - timelineX, timelineHeight, Fade(BLUE, 0.4f));
    DrawLineEx({progressX, timelineY}, {progressX, timelineY + timelineHeight}, 3, BLUE);
    
    if (m_draggingTimeline || CheckCollisionPointRec(GetMousePosition(), {timelineX, timelineY, timelineWidth, timelineHeight})) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_draggingTimeline = true;
        }
        if (m_draggingTimeline) {
            float clickProgress = (GetMouseX() - timelineX) / timelineWidth;
            clickProgress = std::clamp(clickProgress, 0.0f, 1.0f);
            recorder.ScrubToTime(clickProgress * duration);
        }
    }
    
    DrawText("Click/drag to scrub", timelineX, timelineY - 20, 14, DARKGRAY);
}

void UIManager::DrawControlsHint() {
    int y = SCREEN_HEIGHT - 120;
    DrawText("CONTROLS:", 20, y, 16, DARKGRAY);
    DrawText("WASD - Move    SPACE - Jump    R - Record/Stop    P - Replay    T - Time Toggle", 20, y + 22, 14, GRAY);
    DrawText("TAB - Next Level    ESC - Pause/Menu    CTRL+R - Restart    H - Help", 20, y + 42, 14, GRAY);
}

void UIManager::DrawMenuUI() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARKBLUE, 0.95f));
    
    DrawText("DOC'S TASK", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 150, 60, WHITE);
    DrawText("Time Manipulation Puzzle Game", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 80, 28, LIGHTGRAY);
    
    DrawRectangleRounded({SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2, 240, 50}, 0.3f, 8, YELLOW);
    DrawText("PRESS ENTER TO START", SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 12, 22, DARKBROWN);
    
    DrawText("Record your actions, then replay them", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 70, 20, LIGHTGRAY);
    DrawText("to solve puzzles cooperatively with your past self!", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 100, 20, LIGHTGRAY);
    
    DrawText("Controls: WASD=Move, SPACE=Jump, R=Record, P=Replay, T=Time Toggle", SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT - 60, 18, GRAY);
}

void UIManager::DrawPauseUI() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));
    DrawText("PAUSED", SCREEN_WIDTH/2 - 70, SCREEN_HEIGHT/2 - 50, 50, WHITE);
    DrawText("Press ESC to Resume", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 20, 24, LIGHTGRAY);
    DrawText("Press TAB for Next Level", SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 60, 20, LIGHTGRAY);
    DrawText("Press CTRL+R to Restart", SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 90, 20, LIGHTGRAY);
}

void UIManager::DrawLevelCompleteUI() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(GREEN, 0.2f));
    DrawRectangleRounded({SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 60, 400, 120}, 0.3f, 8, DARKGREEN);
    DrawText("LEVEL COMPLETE!", SCREEN_WIDTH/2 - 130, SCREEN_HEIGHT/2 - 40, 40, DARKGREEN);
    DrawText("Press TAB for Next Level", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20, 24, DARKGREEN);
}

void UIManager::DrawHelpUI() {
    DrawRectangleRounded({SCREEN_WIDTH - 350, 20, 330, 350}, 0.1f, 8, Fade(LIGHTGRAY, 0.95f));
    DrawRectangleRoundedLines({SCREEN_WIDTH - 350, 20, 330, 350}, 0.1f, 8, DARKGRAY);
    
    int x = SCREEN_WIDTH - 340;
    int y = 30;
    
    DrawText("HELP (H to toggle)", x, y, 20, DARKGRAY); y += 30;
    DrawText("CONCEPT:", x, y, 16, DARKBLUE); y += 24;
    DrawText("Record physics objects moving", x, y, 14, DARKGRAY); y += 20;
    DrawText("Replay them to create platforms", x, y, 14, DARKGRAY); y += 20;
    DrawText("Solve puzzles with your past self", x, y, 14, DARKGRAY); y += 30;
    
    DrawText("RECORD MODE:", x, y, 16, RED); y += 24;
    DrawText("Press R to start recording", x, y, 14, DARKGRAY); y += 20;
    DrawText("Move objects, trigger switches", x, y, 14, DARKGRAY); y += 20;
    DrawText("Press R again to stop", x, y, 14, DARKGRAY); y += 30;
    
    DrawText("REPLAY MODE:", x, y, 16, BLUE); y += 24;
    DrawText("Press P to replay recording", x, y, 14, DARKGRAY); y += 20;
    DrawText("Objects repeat your actions", x, y, 14, DARKGRAY); y += 20;
    DrawText("Use them to reach the goal", x, y, 14, DARKGRAY); y += 30;
    
    DrawText("TIMELINE:", x, y, 16, PURPLE); y += 24;
    DrawText("Click/drag bottom bar to scrub", x, y, 14, DARKGRAY);
}