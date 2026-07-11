#include "Levels.h"
#include "raylib.h"

void Level1::Load() {
    LevelBase::Load();
    
    CreateGround(30);
    
    AddStaticBox({0, 1.5f, -5}, {6, 3, 6}, DARKBLUE);
    AddStaticBox({0, 1.5f, 10}, {6, 3, 6}, DARKBLUE);
    
    AddBox({0, 5, -2}, {2, 2, 2}, 5.0f, true, RED);
    
    CreateGoal({0, 2, 12});
    m_playerStart = {0, 2, -7};
}

void Level1::DrawUI() {
    DrawText("Press [R] to START/STOP recording", 10, 10, 20, BLACK);
    DrawText("Press [P] to START/STOP replay", 10, 35, 20, BLACK);
    DrawText("Move with WASD, Jump with SPACE", 10, 60, 20, BLACK);
    DrawText("Goal: Reach the green cylinder", 10, 85, 20, DARKGREEN);
}

void Level2::Load() {
    LevelBase::Load();
    
    CreateGround(25);
    
    AddStaticBox({-6, 1.5f, 0}, {4, 3, 10}, DARKBLUE);
    AddStaticBox({6, 1.5f, 0}, {4, 3, 10}, DARKBLUE);
    
    AddBox({0, 8, 0}, {8, 1, 8}, 20.0f, true, ORANGE);
    
    CreateGoal({0, 10, 0});
    m_playerStart = {-8, 2, 0};
}

void Level2::DrawUI() {
    DrawText("Record the platform swing timing", 10, 10, 20, BLACK);
    DrawText("Jump on it during replay to reach goal", 10, 35, 20, BLACK);
}

void Level3::Load() {
    LevelBase::Load();
    
    CreateGround(20);
    
    AddStaticBox({0, 1.5f, -8}, {8, 3, 4}, DARKBLUE);
    
    AddBox({-2, 4, 0}, {2, 2, 2}, 3.0f, true, RED);
    AddBox({2, 4, 0}, {2, 2, 2}, 3.0f, true, BLUE);
    AddBox({0, 7, 0}, {2, 2, 2}, 3.0f, true, GREEN);
    AddBox({-2, 10, 2}, {2, 2, 2}, 3.0f, true, YELLOW);
    AddBox({2, 10, 2}, {2, 2, 2}, 3.0f, true, PURPLE);
    
    CreateGoal({0, 14, 5});
    m_playerStart = {0, 2, -10};
}

void Level3::DrawUI() {
    DrawText("Record boxes falling to create stairs", 10, 10, 20, BLACK);
    DrawText("Replay and climb the recorded boxes", 10, 35, 20, BLACK);
}

void Level4::Load() {
    LevelBase::Load();
    
    CreateGround(30);
    
    AddStaticBox({0, 1.5f, -5}, {6, 3, 6}, DARKBLUE);
    AddStaticBox({0, 1.5f, 10}, {6, 3, 6}, DARKBLUE);
    AddStaticBox({0, 1.5f, 25}, {6, 3, 6}, DARKBLUE);
    
    AddBox({0, 5, -2}, {2, 2, 2}, 2.0f, true, RED);
    AddBox({0, 8, 13}, {2, 2, 2}, 2.0f, true, BLUE);
    
    CreateGoal({0, 3, 27});
    m_playerStart = {0, 2, -7};
}

void Level4::DrawUI() {
    DrawText("Record first box, replay to reach second", 10, 10, 20, BLACK);
    DrawText("Record second box on top of first replay", 10, 35, 20, BLACK);
    DrawText("[T] to scrub timeline", 10, 85, 20, BLACK);
}

void Level5::Load() {
    LevelBase::Load();
    
    CreateGround(40);
    
    AddStaticBox({0, 1.5f, -5}, {8, 3, 6}, DARKBLUE);
    AddStaticBox({-10, 1.5f, 10}, {4, 3, 8}, DARKBLUE);
    AddStaticBox({10, 1.5f, 10}, {4, 3, 8}, DARKBLUE);
    AddStaticBox({0, 1.5f, 25}, {8, 3, 6}, DARKBLUE);
    
    AddBox({-3, 5, -2}, {2, 2, 2}, 3.0f, true, RED);
    AddBox({3, 5, -2}, {2, 2, 2}, 3.0f, true, BLUE);
    AddBox({-3, 8, 12}, {2, 2, 2}, 3.0f, true, GREEN);
    AddBox({3, 8, 12}, {2, 2, 2}, 3.0f, true, YELLOW);
    AddBox({0, 11, 20}, {3, 1, 3}, 10.0f, true, ORANGE);
    
    CreateGoal({0, 14, 27});
    m_playerStart = {0, 2, -7};
}

void Level5::DrawUI() {
    DrawText("FINAL TEST - Use all mechanics!", 10, 10, 20, MAROON);
    DrawText("Stack, time, and loop recordings", 10, 35, 20, BLACK);
}