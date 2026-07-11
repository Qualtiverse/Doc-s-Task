#pragma once

#include "CoreTypes.h"

class LevelBase;

class UIManager {
public:
    UIManager();
    
    void Update(float dt, GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level);
    void Draw(GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level, float levelTime);
    
    void ToggleHelp() { m_showHelp = !m_showHelp; m_helpTimer = 5.0f; }
    void SetHelpVisible(bool v) { m_showHelp = v; }
    
private:
    void DrawMainUI(GameState state, TimeMode timeMode, TimeRecorder& recorder, LevelBase* level, float levelTime);
    void DrawMenuUI();
    void DrawPauseUI();
    void DrawLevelCompleteUI();
    void DrawHelpUI();
    void DrawTimeline(TimeRecorder& recorder, float levelTime);
    void DrawModeIndicator(TimeMode mode);
    void DrawControlsHint();
    void DrawBuildUI(LevelBase* level);
    void DrawBuildHelpUI();
    
    bool m_showHelp = true;
    float m_helpTimer = 10.0f;
    float m_recordingFlashTimer = 0;
    bool m_draggingTimeline = false;
};