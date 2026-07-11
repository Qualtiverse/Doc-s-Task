#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define SCREEN_W 1280
#define SCREEN_H 720

struct EditorObject {
    Vector3 pos{0,0,0};
    Vector3 size{1,1,1};
    Color color{255,255,255,255};
    float mass = 1.0f;
    bool isStatic = false;
};

struct EditorLevel {
    std::string name = "Untitled";
    std::string hint = "";
    Vector3 playerStart{0,2,0};
    Vector3 goalPos{0,1,10};
    float goalRadius = 1.5f;
    std::vector<EditorObject> objects;
};

enum Mode { M_STATIC, M_DYNAMIC, M_PLAYER, M_GOAL, M_COUNT };

static const char* ModeNames[M_COUNT] = {
    "Place Static Box", "Place Dynamic Box", "Set Player Start", "Set Goal Zone"
};
static Color ModeColors[M_COUNT] = {
    GRAY, RED, BLUE, GREEN
};

static EditorLevel level;
static Mode mode = M_STATIC;
static int selected = -1;
static bool showHelp = true;

static Camera3D cam;
static float camDist = 18.0f;
static float camAngle = 0.4f;
static float camHeight = 10.0f;
static Vector3 camTarget{0,0,0};

static bool orbiting = false;
static bool panning = false;

static float snap = 0.5f;
static char nameBuf[256] = "Untitled";
static char hintBuf[512] = "";
static int nameLen = 8;
static int hintLen = 0;
static bool editingName = false;
static bool editingHint = false;

static char statusMsg[256] = "Ready";
static float statusTimer = 0;

static void SetStatus(const char* msg) {
    strncpy(statusMsg, msg, sizeof(statusMsg)-1);
    statusMsg[sizeof(statusMsg)-1] = 0;
    statusTimer = 3.0f;
}

static Vector3 Snap(const Vector3& v, float s) {
    return { roundf(v.x/s)*s, roundf(v.y/s)*s, roundf(v.z/s)*s };
}

static Vector3 GetGroundPos() {
    Vector2 mp = GetMousePosition();
    Ray ray = GetMouseRay(mp, cam);
    if (ray.direction.y == 0) return {0,0,0};
    float t = -ray.position.y / ray.direction.y;
    if (t < 0) return {0,0,0};
    Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
    p.y = 0;
    return Snap(p, snap);
}

static int PickObject() {
    Vector2 mp = GetMousePosition();
    Ray ray = GetMouseRay(mp, cam);
    float bestT = 1e9f;
    int best = -1;
    for (int i = 0; i < (int)level.objects.size(); i++) {
        auto& o = level.objects[i];
        Vector3 half = Vector3Scale(o.size, 0.5f);
        Vector3 min = Vector3Subtract(o.pos, half);
        Vector3 max = Vector3Add(o.pos, half);
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

static void SaveLevel(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) { SetStatus("Failed to save"); return; }
    fprintf(f, "name=%s\n", level.name.c_str());
    fprintf(f, "hint=%s\n", level.hint.c_str());
    fprintf(f, "player=%.2f %.2f %.2f\n", level.playerStart.x, level.playerStart.y, level.playerStart.z);
    fprintf(f, "goal=%.2f %.2f %.2f %.2f\n", level.goalPos.x, level.goalPos.y, level.goalPos.z, level.goalRadius);
    for (auto& o : level.objects) {
        fprintf(f, "%s=%.2f %.2f %.2f %.2f %.2f %.2f %d %d %d %d %.2f\n",
            o.isStatic ? "STATIC" : "DYNAMIC",
            o.pos.x, o.pos.y, o.pos.z,
            o.size.x, o.size.y, o.size.z,
            o.color.r, o.color.g, o.color.b, o.color.a,
            o.mass);
    }
    fclose(f);
    char buf[256];
    snprintf(buf, sizeof(buf), "Saved to %s", path);
    SetStatus(buf);
}

static void LoadLevel(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) { SetStatus("Failed to load"); return; }
    level.objects.clear();
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char* nl = strchr(line, '\n'); if (nl) *nl = 0;
        if (line[0] == 0 || line[0] == '#') continue;
        char* val = strchr(line, '=');
        if (!val) continue;
        *val++ = 0;
        if (strcmp(line, "name") == 0) {
            level.name = val;
            strncpy(nameBuf, val, sizeof(nameBuf)-1); nameBuf[sizeof(nameBuf)-1] = 0;
            nameLen = (int)strlen(nameBuf);
        } else if (strcmp(line, "hint") == 0) {
            level.hint = val;
            strncpy(hintBuf, val, sizeof(hintBuf)-1); hintBuf[sizeof(hintBuf)-1] = 0;
            hintLen = (int)strlen(hintBuf);
        } else if (strcmp(line, "player") == 0) {
            sscanf(val, "%f %f %f", &level.playerStart.x, &level.playerStart.y, &level.playerStart.z);
        } else if (strcmp(line, "goal") == 0) {
            sscanf(val, "%f %f %f %f", &level.goalPos.x, &level.goalPos.y, &level.goalPos.z, &level.goalRadius);
        } else if (strcmp(line, "STATIC") == 0 || strcmp(line, "DYNAMIC") == 0) {
            EditorObject o;
            int r,g,b,a;
            o.isStatic = (strcmp(line, "STATIC") == 0);
            sscanf(val, "%f %f %f %f %f %f %d %d %d %d %f",
                &o.pos.x, &o.pos.y, &o.pos.z,
                &o.size.x, &o.size.y, &o.size.z,
                &r, &g, &b, &a, &o.mass);
            o.color = {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
            level.objects.push_back(o);
        }
    }
    fclose(f);
    camTarget = level.playerStart;
    SetStatus("Level loaded");
}

static void HandleTextInput() {
    if (editingName) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key < 127 && nameLen < 254) {
                nameBuf[nameLen++] = (char)key;
                nameBuf[nameLen] = 0;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && nameLen > 0) {
            nameBuf[--nameLen] = 0;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            editingName = false;
            level.name = nameBuf;
        }
    }
    if (editingHint) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key < 127 && hintLen < 510) {
                hintBuf[hintLen++] = (char)key;
                hintBuf[hintLen] = 0;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && hintLen > 0) {
            hintBuf[--hintLen] = 0;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            editingHint = false;
            level.hint = hintBuf;
        }
    }
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "Doc's Task - Level Editor");
    SetTargetFPS(60);

    cam.position = {15,12,15};
    cam.target = {0,0,0};
    cam.up = {0,1,0};
    cam.fovy = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    strncpy(nameBuf, "Untitled", sizeof(nameBuf)-1);
    nameBuf[sizeof(nameBuf)-1] = 0;
    nameLen = 8;

    {
        EditorObject ground;
        ground.pos = {0,-0.5f,0};
        ground.size = {20,1,20};
        ground.color = DARKGRAY;
        ground.isStatic = true;
        level.objects.push_back(ground);

        EditorObject plat;
        plat.pos = {0,1.5f,-5};
        plat.size = {6,3,6};
        plat.color = {0,0,139,255};
        plat.isStatic = true;
        level.objects.push_back(plat);

        EditorObject box;
        box.pos = {0,5,0};
        box.size = {2,2,2};
        box.color = {255,0,0,255};
        box.mass = 3.0f;
        box.isStatic = false;
        level.objects.push_back(box);
    }

    SetStatus("Press H for help");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- Text input for name/hint ---
        HandleTextInput();

        // --- Camera ---
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            if (!orbiting) { orbiting = true; }
            Vector2 delta = GetMouseDelta();
            camAngle -= delta.x * 0.003f;
            camHeight += delta.y * 0.05f;
        } else { orbiting = false; }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            if (!panning) { panning = true; }
            Vector2 delta = GetMouseDelta();
            Vector3 right = Vector3CrossProduct(
                Vector3Subtract(cam.target, cam.position), cam.up);
            right = Vector3Normalize(right);
            camTarget = Vector3Add(camTarget, Vector3Scale(right, -delta.x * 0.03f));
            camTarget = Vector3Add(camTarget, Vector3Scale((Vector3){0,1,0}, delta.y * 0.03f));
        } else { panning = false; }

        float wheel = GetMouseWheelMove();
        camDist -= wheel * 1.5f;
        camDist = Clamp(camDist, 4.0f, 40.0f);
        camHeight = Clamp(camHeight, 2.0f, 25.0f);

        cam.position.x = camTarget.x + cosf(camAngle) * camDist;
        cam.position.z = camTarget.z + sinf(camAngle) * camDist;
        cam.position.y = camTarget.y + camHeight;
        cam.target = camTarget;

        // --- Help toggle ---
        if (IsKeyPressed(KEY_H) && !editingName && !editingHint) showHelp = !showHelp;

        // --- Mode switching ---
        if (IsKeyPressed(KEY_ONE)) mode = M_STATIC;
        if (IsKeyPressed(KEY_TWO)) mode = M_DYNAMIC;
        if (IsKeyPressed(KEY_THREE)) mode = M_PLAYER;
        if (IsKeyPressed(KEY_FOUR)) mode = M_GOAL;

        // --- Snap size ---
        if (IsKeyPressed(KEY_LEFT)) snap = (snap > 0.25f) ? snap * 0.5f : 0.25f;
        if (IsKeyPressed(KEY_RIGHT)) snap = (snap < 4.0f) ? snap * 2.0f : 4.0f;

        // --- UI click handling ---
        bool uiClicked = false;
        Vector2 mp = GetMousePosition();

        // Check name/hint field clicks
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Rectangle nameRect = {270, 30, 220, 24};
            Rectangle hintRect = {270, 76, 220, 24};
            if (CheckCollisionPointRec(mp, nameRect)) {
                editingName = true;
                editingHint = false;
                uiClicked = true;
            } else if (CheckCollisionPointRec(mp, hintRect)) {
                editingHint = true;
                editingName = false;
                uiClicked = true;
            } else {
                editingName = false;
                editingHint = false;
                if (editingName) level.name = nameBuf;
                if (editingHint) level.hint = hintBuf;
            }
        }

        // Check mode button clicks (drawn later, but we handle clicks early)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !uiClicked && mp.x >= 270 && mp.x <= 510) {
            for (int i = 0; i < M_COUNT; i++) {
                int by = 120 + i * 26;
                if (mp.y >= by && mp.y <= by + 24) {
                    mode = (Mode)i;
                    uiClicked = true;
                    break;
                }
            }
        }

        // --- 3D interaction ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !uiClicked && !orbiting && !panning) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                int picked = PickObject();
                if (picked >= 0) {
                    selected = picked;
                    char buf[64]; snprintf(buf, sizeof(buf), "Selected object %d", picked);
                    SetStatus(buf);
                } else {
                    selected = -1;
                }
            } else {
                Vector3 gp = GetGroundPos();
                if (mode == M_STATIC || mode == M_DYNAMIC) {
                    EditorObject obj;
                    obj.pos = gp; obj.pos.y = 0.5f;
                    obj.size = {1,1,1};
                    obj.color = mode == M_STATIC ? (Color){100,100,100,255} : (Color){255,0,0,255};
                    obj.mass = mode == M_STATIC ? 0 : 3.0f;
                    obj.isStatic = (mode == M_STATIC);
                    level.objects.push_back(obj);
                    selected = (int)level.objects.size() - 1;
                    SetStatus(mode == M_STATIC ? "Placed static box" : "Placed dynamic box");
                } else if (mode == M_PLAYER) {
                    level.playerStart = gp; level.playerStart.y = 2.0f;
                    camTarget = level.playerStart;
                    SetStatus("Player start set");
                } else if (mode == M_GOAL) {
                    level.goalPos = gp; level.goalPos.y = 1.0f;
                    SetStatus("Goal set");
                }
            }
        }

        // --- Modify selected object ---
        if (selected >= 0 && selected < (int)level.objects.size()) {
            auto& obj = level.objects[selected];
            if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
                obj.size.x += snap; obj.size.y += snap; obj.size.z += snap;
            }
            if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
                obj.size.x = fmaxf(0.5f, obj.size.x - snap);
                obj.size.y = fmaxf(0.5f, obj.size.y - snap);
                obj.size.z = fmaxf(0.5f, obj.size.z - snap);
            }
            float moveAmt = 0.5f;
            if (IsKeyDown(KEY_UP)) obj.pos.z -= moveAmt;
            if (IsKeyDown(KEY_DOWN)) obj.pos.z += moveAmt;
            if (IsKeyDown(KEY_LEFT)) obj.pos.x -= moveAmt;
            if (IsKeyDown(KEY_RIGHT)) obj.pos.x += moveAmt;
            if (IsKeyDown(KEY_PAGE_UP)) obj.pos.y += moveAmt;
            if (IsKeyDown(KEY_PAGE_DOWN)) obj.pos.y = fmaxf(0.25f, obj.pos.y - moveAmt);

            if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                level.objects.erase(level.objects.begin() + selected);
                selected = -1;
                SetStatus("Deleted object");
            }
        }

        // --- Save / Load ---
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            if (IsKeyPressed(KEY_S)) SaveLevel("level.lvl");
            if (IsKeyPressed(KEY_L)) LoadLevel("level.lvl");
        }

        // --- Status timer ---
        if (statusTimer > 0) statusTimer -= dt;

        // --- Draw ---
        BeginDrawing();
        ClearBackground({25,25,35,255});

        BeginMode3D(cam);
        DrawGrid(40, 1.0f);

        for (int i = 0; i < (int)level.objects.size(); i++) {
            auto& o = level.objects[i];
            bool isSel = (i == selected);
            Color c = isSel ? YELLOW : o.color;
            DrawCube(o.pos, o.size.x, o.size.y, o.size.z, c);
            DrawCubeWires(o.pos, o.size.x, o.size.y, o.size.z, isSel ? ORANGE : BLACK);
        }

        // Player marker (blue capsule)
        {
            Vector3 pp = level.playerStart;
            DrawCylinder(pp, 0.4f, 0.4f, 1.0f, 8, BLUE);
            DrawCylinderWires(pp, 0.4f, 0.4f, 1.0f, 8, DARKBLUE);
        }

        // Goal marker (green cylinder)
        {
            Vector3 gp = level.goalPos;
            DrawCylinder(gp, level.goalRadius, level.goalRadius, 0.2f, 16, ColorAlpha(GREEN, 0.3f));
            DrawCylinderWires(gp, level.goalRadius, level.goalRadius, 2.0f, 8, GREEN);
        }

        // Snap ghost at mouse position
        if (mode == M_STATIC || mode == M_DYNAMIC) {
            Vector3 ghost = GetGroundPos();
            ghost.y = 0.5f;
            Color ghostCol = mode == M_STATIC ? ColorAlpha(GRAY, 0.4f) : ColorAlpha(RED, 0.4f);
            DrawCube(ghost, snap, snap, snap, ghostCol);
            DrawCubeWires(ghost, snap, snap, snap, ColorAlpha(WHITE, 0.3f));
        }

        EndMode3D();

        // --- UI ---
        int ux = 270, uy = 10;

        // Level name
        DrawText("NAME:", ux, uy, 14, LIGHTGRAY); uy += 18;
        DrawRectangle(ux, uy, 220, 24, ColorAlpha(WHITE, editingName ? 0.25f : 0.12f));
        DrawRectangleLines(ux, uy, 220, 24, editingName ? YELLOW : ColorAlpha(WHITE, 0.3f));
        DrawText(nameBuf, ux+4, uy+4, 16, WHITE); uy += 32;

        // Hint
        DrawText("HINT:", ux, uy, 14, LIGHTGRAY); uy += 18;
        DrawRectangle(ux, uy, 220, 24, ColorAlpha(WHITE, editingHint ? 0.25f : 0.12f));
        DrawRectangleLines(ux, uy, 220, 24, editingHint ? YELLOW : ColorAlpha(WHITE, 0.3f));
        DrawText(hintBuf[0] ? hintBuf : "(click to edit)", ux+4, uy+4, 16, hintBuf[0] ? WHITE : (Color){100,100,100,255});
        uy += 36;

        // Mode buttons
        DrawText("MODES:", ux, uy, 14, LIGHTGRAY); uy += 18;
        for (int i = 0; i < M_COUNT; i++) {
            bool active = (i == mode);
            Color c = active ? ModeColors[i] : ColorAlpha(ModeColors[i], 0.5f);
            DrawRectangle(ux, uy, 240, 24, ColorAlpha(c, active ? 0.3f : 0.08f));
            DrawRectangleLines(ux, uy, 240, 24, c);
            DrawText(TextFormat("[%d] %s", i+1, ModeNames[i]), ux+6, uy+4, 14, c);
            uy += 26;
        }

        uy += 8;
        DrawText(TextFormat("SNAP: %.2f  (< >)", snap), ux, uy, 14, LIGHTGRAY); uy += 20;
        DrawText(TextFormat("OBJECTS: %d", (int)level.objects.size()), ux, uy, 14, LIGHTGRAY); uy += 20;
        DrawText("[Ctrl+S] Save  [Ctrl+L] Load", ux, uy, 13, LIGHTGRAY); uy += 20;

        // Selected object properties
        if (selected >= 0 && selected < (int)level.objects.size()) {
            auto& o = level.objects[selected];
            uy += 6;
            DrawRectangle(ux-2, uy, 244, 100, ColorAlpha(YELLOW, 0.1f));
            DrawRectangleLines(ux-2, uy, 244, 100, ColorAlpha(YELLOW, 0.3f));
            DrawText(TextFormat("SELECTED [%d]", selected), ux+4, uy+4, 14, YELLOW); uy += 22;
            DrawText(TextFormat("Pos: %.1f %.1f %.1f", o.pos.x, o.pos.y, o.pos.z), ux+4, uy, 13, LIGHTGRAY); uy += 16;
            DrawText(TextFormat("Size: %.1f %.1f %.1f", o.size.x, o.size.y, o.size.z), ux+4, uy, 13, LIGHTGRAY); uy += 16;
            DrawText(o.isStatic ? "STATIC" : TextFormat("DYNAMIC (mass=%.1f)", o.mass), ux+4, uy, 13, LIGHTGRAY); uy += 16;
            DrawText("+/- size | Arrows move | Del delete", ux+4, uy, 12, GRAY); uy += 16;
        }

        // Status message
        if (statusTimer > 0) {
            DrawText(statusMsg, 10, SCREEN_H-24, 16, ColorAlpha(GREEN, fminf(1.0f, statusTimer)));
        }

        // Help overlay
        if (showHelp) {
            int hx = SCREEN_W-390, hy = 10;
            DrawRectangle(hx, hy, 380, 400, ColorAlpha(BLACK, 0.88f));
            DrawRectangleLines(hx, hy, 380, 400, ColorAlpha(WHITE, 0.2f));
            hx += 10; hy += 15;
            DrawText("HELP (H to toggle)", hx, hy, 20, WHITE); hy += 30;

            DrawText("MOUSE:", hx, hy, 16, YELLOW); hy += 22;
            DrawText("L-click ground  = Place object / Set marker", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Shift+L-click   = Select existing object", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Right+drag      = Orbit camera", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Middle+drag     = Pan camera", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Scroll          = Zoom", hx, hy, 13, LIGHTGRAY); hy += 22;

            DrawText("MODES:", hx, hy, 16, YELLOW); hy += 22;
            DrawText("[1] Place Static   [2] Place Dynamic", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("[3] Set Player     [4] Set Goal", hx, hy, 13, LIGHTGRAY); hy += 22;

            DrawText("EDITING:", hx, hy, 16, YELLOW); hy += 22;
            DrawText("+/-       = Resize selected object", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Arrows    = Move selected (xy plane)", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("PgUp/Dn   = Move selected (up/down)", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Del/Bksp  = Delete selected", hx, hy, 13, LIGHTGRAY); hy += 22;

            DrawText("OTHER:", hx, hy, 16, YELLOW); hy += 22;
            DrawText("< / >     = Change snap size", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Ctrl+S    = Save to level.lvl", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Ctrl+L    = Load from level.lvl", hx, hy, 13, LIGHTGRAY); hy += 18;
            DrawText("Click name/hint fields to edit", hx, hy, 13, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
