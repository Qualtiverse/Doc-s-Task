#pragma once

#include "CoreTypes.h"

class Level;

class UIManager {
public:
    UIManager();
    
    void Update(float dt, GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level);
    void Draw(GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level, float levelTime);
    
    void ToggleHelp() { m_showHelp = !m_showHelp; m_helpTimer = 5.0f; }
    
private:
    void DrawMainUI(GameState state, TimeMode timeMode, TimeRecorder& recorder, Level* level, float levelTime);
    void DrawMenuUI();
    void DrawPauseUI();
    void DrawLevelCompleteUI();
    void DrawHelpUI();
    void DrawTimeline(TimeRecorder& recorder, float levelTime);
    void DrawModeIndicator(TimeMode mode);
    void DrawControlsHint();
    
    bool m_showHelp = true;
    float m_helpTimer = 10.0f;
    float m_recordingFlashTimer = 0;
    int m_selectedTimelineFrame = -1;
    bool m_draggingTimeline = false;
};