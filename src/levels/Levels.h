#pragma once

#include "Level.h"

class Level1 : public Level {
public:
    void Load() override;
    const char* GetName() const override { return "Level 1: Recording Basics"; }
    const char* GetHint() const override { return "Record the box falling, then replay to ride it across the gap"; }
    void DrawUI() override;
};

class Level2 : public Level {
public:
    void Load() override;
    const char* GetName() const override { return "Level 2: Timing"; }
    const char* GetHint() const override { return "Record the swinging platform at the right moment"; }
    void DrawUI() override;
};

class Level3 : public Level {
public:
    void Load() override;
    const char* GetName() const override { return "Level 3: Multi-Object"; }
    const char* GetHint() const override { return "Record multiple boxes to build a staircase"; }
    void DrawUI() override;
};

class Level4 : public Level {
public:
    void Load() override;
    const char* GetName() const override { return "Level 4: Loop Puzzle"; }
    const char* GetHint() const override { return "Create a loop - record, replay, then record again on top"; }
    void DrawUI() override;
};

class Level5 : public Level {
public:
    void Load() override;
    const char* GetName() const override { return "Level 5: Final Test"; }
    const char* GetHint() const override { return "Combine all mechanics - timing, stacking, and looping"; }
    void DrawUI() override;
};