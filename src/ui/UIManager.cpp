#include "UIManager.h"
#include "Level.h"
#include "raylib.h"
#include "raymath.h"
#include <string>

UIManager::UIManager() {}

void UIManager::Update(float dt, GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level) {
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

void UIManager::Draw(GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level, float levelTime) {
    if (state == GameState::BUILDING) {
        DrawBuildUI(level);
        if (m_showHelp) DrawBuildHelpUI();
        return;
    }
    
    DrawMainUI(state, timeMode, recorder, level, levelTime);
    
    if (state == GameState::MENU) DrawMenuUI();
    if (state == GameState::PAUSED) DrawPauseUI();
    if (state == GameState::LEVEL_COMPLETE) DrawLevelCompleteUI();
    if (m_showHelp) DrawHelpUI();
}

void UIManager::DrawMainUI(GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level, float levelTime) {
    DrawText("Doc's Task", 20, 20, 28, DARKGRAY);
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
    for (int i = 0; i < frameCount && frameCount > 1; i++) {
        float x = timelineX + (i / (float)(frameCount - 1)) * timelineWidth;
        DrawLineEx({x, timelineY + 2}, {x, timelineY + timelineHeight - 2}, 1, Fade(LIGHTGRAY, 0.5f));
    }
    
    float progress = levelTime / duration;
    Rectangle progressRect = {timelineX, timelineY, progress * timelineWidth, timelineHeight};
    DrawRectangleRec(progressRect, Fade(BLUE, 0.4f));
    DrawLineEx({timelineX + progress * timelineWidth, timelineY}, 
               {timelineX + progress * timelineWidth, timelineY + timelineHeight}, 3, BLUE);
    
    Rectangle timelineRect = {timelineX, timelineY, timelineWidth, timelineHeight};
    if (CheckCollisionPointRec(GetMousePosition(), timelineRect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_draggingTimeline = true;
        }
        if (m_draggingTimeline) {
            float clickProgress = (GetMouseX() - timelineX) / timelineWidth;
            clickProgress = Clamp(clickProgress, 0.0f, 1.0f);
            recorder.ScrubToTime(clickProgress * duration);
        }
    }
    
    DrawText("Click/drag to scrub timeline", timelineX, timelineY - 20, 14, DARKGRAY);
}

void UIManager::DrawControlsHint() {
    DrawText("CONTROLS:", 20, SCREEN_HEIGHT - 100, 16, DARKGRAY);
    DrawText("WASD=Move SPACE=Jump R=Record/Stop P=Replay T=TimeToggle", 20, SCREEN_HEIGHT - 78, 14, GRAY);
    DrawText("TAB=NextLevel ESC=Pause/Menu CTRL+R=Restart H=Help", 20, SCREEN_HEIGHT - 58, 14, GRAY);
}

void UIManager::DrawMenuUI() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(DARKBLUE, 0.95f));
    
    DrawText("DOC'S TASK", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 160, 60, WHITE);
    DrawText("Time Manipulation Puzzle Game", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 90, 28, LIGHTGRAY);
    
    float cyf = (float)(SCREEN_HEIGHT/2 - 20);
    DrawRectangleRounded({(float)(SCREEN_WIDTH/2 - 120), cyf, 240, 44}, 0.3f, 8, YELLOW);
    DrawText("[ENTER]  PLAY LEVELS", SCREEN_WIDTH/2 - 100, (int)cyf + 10, 18, DARKBROWN);
    
    cyf += 60;
    DrawRectangleRounded({(float)(SCREEN_WIDTH/2 - 120), cyf, 240, 44}, 0.3f, 8, ColorAlpha(BLUE, 0.8f));
    DrawText("[E]  LEVEL EDITOR", SCREEN_WIDTH/2 - 85, (int)cyf + 10, 18, WHITE);
    
    DrawText("Record physics, replay time,", SCREEN_WIDTH/2 - 150, cy + 70, 18, LIGHTGRAY);
    DrawText("solve puzzles with your past self!", SCREEN_WIDTH/2 - 160, cy + 95, 18, LIGHTGRAY);
    
    DrawText("WASD=Move SPACE=Jump R=Record P=Replay", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT - 40, 16, GRAY);
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

void UIManager::DrawBuildUI(LevelBase* level) {
    DrawRectangle(0, 0, SCREEN_WIDTH, 26, ColorAlpha(BLACK, 0.5f));
    DrawText("BUILD MODE - WASD=Move SPACE=Jump [1-6]=Tools F=Palette TAB=Test Ctrl+S=Save Ctrl+L=Load H=Help",
        10, 4, 14, ColorAlpha(WHITE, 0.8f));
    
    if (level) {
        DrawText(TextFormat("Level: %s", level->EditorGetLevelName().c_str()),
            SCREEN_WIDTH/2 - 100, 4, 14, YELLOW);
    }
}

void UIManager::DrawBuildHelpUI() {
    int x = SCREEN_WIDTH - 370, y = 32;
    DrawRectangle(x-10, y-8, 360, 310, ColorAlpha(BLACK, 0.88f));
    DrawRectangleLines(x-10, y-8, 360, 310, ColorAlpha(WHITE, 0.2f));
    
    DrawText("BUILD HELP (H)", x, y, 18, YELLOW); y += 26;
    DrawText("[1] Place Static    [2] Place Dynamic", x, y, 14, LIGHTGRAY); y += 18;
    DrawText("[3] Set Player      [4] Set Goal", x, y, 14, LIGHTGRAY); y += 18;
    DrawText("[5] Delete Mode     [6] Select Mode", x, y, 14, LIGHTGRAY); y += 22;
    DrawText("L-click        = Place / Delete / Select", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("Arrows         = Move selected (hold Shift=slow)", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("+/-            = Resize selected", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("PgUp/Dn        = Move up/down", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("Del/Bksp       = Delete selected", x, y, 13, LIGHTGRAY); y += 18;
    DrawText(",/.            = Decrease/Increase snap", x, y, 13, LIGHTGRAY); y += 22;
    DrawText("TAB            = Test level (play mode)", x, y, 14, GREEN); y += 18;
    DrawText("Ctrl+S         = Save to level.lvl", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("Ctrl+L         = Load from level.lvl", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("F              = Toggle palette", x, y, 13, LIGHTGRAY); y += 18;
    DrawText("Click name/hint fields to edit text", x, y, 13, LIGHTGRAY);
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