#pragma once

#include "CoreTypes.h"
#include "Level.h"

class Level1 : public LevelBase {
public:
    void Load() override;
    const char* GetName() const override { return "The Button"; }
    const char* GetHint() const override { return "Record a box falling on the button, then replay to hold it while you walk through the door to the goal!"; }
    void DrawUI() override;
};