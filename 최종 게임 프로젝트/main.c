#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <stdbool.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// =====================================================
// KMP STUDIO - ASCII JUMP GAME
// =====================================================

// [수정 포인트 4] 콘솔 가로 길이에 맞춰 WIDTH를 120으로 확장 (빈 공간 제거)
#define WIDTH 120
#define HEIGHT 30

#define MAX_GAUGE 1000.0f
#define PLATFORM_COUNT 56

// [수정 포인트 5] 너무 높고 팍 튀어오르는 점프 물리 엔진을 부드럽게 완화
#define GRAVITY 0.20f
#define MAX_JUMP_VX 2.8f 
#define MAX_JUMP_VY 3.8f 

#define COLOR_RESET "\x1b[0m"

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool isJumping;
    bool isMoving;
    int walkTimer;
} Player;

typedef struct {
    int x;
    int y;
    int width;
} Platform;

typedef struct {
    int x;
    int y;
    char* name;
    char* desc1;
    char* desc2;
} NPC;

// =====================
// 전역 변수
// =====================
Player players[2];
bool isCharging = false;
bool isMulti = false;
ULONGLONG chargeStart = 0;
int currentPlayer = 0;
int moveDir[2] = { 1, 1 };

int cameraY = 0;
int currentZone = -1;
int areaNotiTimer = 0;

int hallX = 10;
int hallY = 24;
int hallWalkTimer = 0;
bool hallIsMoving = false;
int currentNpc = -1;
int activeBubbleNpc = -1;

// [수정 포인트 3] 크레딧 NPC 
NPC npcs[3] = {
    { 25, 24, "김승주", "플랫폼 시스템 개발", "점프 물리 구현" },
    { 60, 24, "문용성", "맵 설계 담당", "UI 디자인 담당" },
    { 95, 24, "박정원", "게임 총괄", "크레딧 시스템 제작" }
};

// [수정 포인트 4] WIDTH 120에 맞춰 맵 플랫폼들을 넓게 재배치
Platform platforms[PLATFORM_COUNT] = {
    // 지하
    {0, 27, WIDTH}, {30, 23, 15}, {80, 19, 10}, {45, 15, 12}, {15, 11, 10},
    {65, 7, 10}, {95, 3, 10}, {50, -1, 12}, {20, -5, 10}, {85, -9, 10},
    {30, -13, 10}, {70, -17, 12}, {95, -21, 10}, {45, -25, 12}, {15, -31, 15}, {80, -37, 15},
    // 지상
    {35, -43, 10}, {90, -47, 12}, {25, -51, 10}, {70, -55, 12}, {15, -59, 15},
    {55, -63, 10}, {95, -67, 10}, {40, -71, 10}, {10, -75, 12}, {85, -79, 10},
    {50, -83, 10}, {20, -87, 12}, {70, -91, 10}, {95, -95, 15}, {35, -99, 10},
    {80, -103, 10}, {25, -107, 12}, {65, -111, 10}, {15, -115, 10}, {55, -119, 15},
    // 하늘
    {90, -125, 10}, {35, -131, 12}, {70, -137, 10}, {20, -143, 10}, {85, -149, 10},
    {45, -155, 15}, {95, -161, 10}, {30, -167, 10}, {75, -173, 10}, {20, -179, 12},
    {60, -185, 10}, {95, -191, 12}, {40, -197, 10}, {10, -203, 10}, {50, -209, 10},
    {90, -215, 12}, {30, -221, 10}, {75, -227, 15}, {20, -233, 10}, {45, -240, 20}
};

char printBuffer[65536]; // [수정 포인트 3] 화면 깜빡임(Flickering) 방지를 위한 대용량 버퍼

// =====================
// 유틸리티 함수
// =====================
void move_cursor(int x, int y) { printf("\x1b[%d;%dH", y, x); }
void set_font_color(int code) { printf("\x1b[%dm", code); }
void set_bg_color(int code) { printf("\x1b[%dm", code); }
void set_bg_brown() { printf("\x1b[48;5;130m"); }
void hide_cursor() { printf("\x1b[?25l"); }
void show_cursor() { printf("\x1b[?25h"); }
void clear_screen() { printf("\x1b[2J\x1b[3J\x1b[H"); }

int GetZone(float y) {
    if (y > -40) return 0;
    if (y > -120) return 1;
    return 2;
}

void SetZoneColor(int y) {
    int zone = GetZone((float)y);
    if (zone == 0) set_bg_color(40);
    else if (zone == 1) set_bg_color(104);
    else set_bg_color(106);
}

void DrawPart(int screenX, int screenY, float worldY, int playerIndex, const char* str) {
    if (screenY < 0 || screenY >= HEIGHT || screenX < 1 || screenX > WIDTH) return;

    int bgColor = -1;
    bool isBrown = false;

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        int py = platforms[i].y - cameraY;
        if (screenY == py || screenY == py + 1) {
            if (screenX >= platforms[i].x && screenX < platforms[i].x + platforms[i].width) {
                int zone = GetZone((float)platforms[i].y);
                if (screenY == py) {
                    if (zone == 0) bgColor = 100;
                    else if (zone == 1) bgColor = 42;
                    else bgColor = 107;
                }
                else {
                    if (zone == 0) bgColor = 100;
                    else if (zone == 1) { isBrown = true; bgColor = 0; }
                    else bgColor = 107;
                }
                break;
            }
        }
    }

    if (bgColor != -1) {
        if (bgColor == 107) set_bg_color(107);
        else set_bg_color(bgColor);
    }
    else if (isBrown) {
        set_bg_brown();
    }
    else {
        SetZoneColor(worldY);
    }

    if (bgColor == 107) {
        set_font_color(30);
    }
    else {
        if (isMulti) {
            if (playerIndex == 0) set_font_color(92);
            else set_font_color(93);
        }
        else {
            set_font_color(97);
        }
    }

    move_cursor(screenX, screenY);
    printf("%s", str);
}

void DrawCharacter(int x, int y, int index) {
    Player* p = &players[index];
    float wy = p->y;

    if (isCharging && currentPlayer == index) {
        DrawPart(x + 1, y, wy, index, "O");
        DrawPart(x, y + 1, wy + 1, index, "└|┘");
        DrawPart(x, y + 2, wy + 2, index, "/"); DrawPart(x + 2, y + 2, wy + 2, index, "\\");
    }
    else if (p->isJumping) {
        DrawPart(x, y, wy, index, "\\O/");
        DrawPart(x + 1, y + 1, wy + 1, index, "|");
        DrawPart(x, y + 2, wy + 2, index, "/"); DrawPart(x + 2, y + 2, wy + 2, index, "\\");
    }
    else {
        DrawPart(x + 1, y, wy, index, "O");
        DrawPart(x, y + 1, wy + 1, index, "(|)");
        if (p->isMoving) {
            if ((p->walkTimer / 4) % 2 == 0) {
                DrawPart(x, y + 2, wy + 2, index, "/"); DrawPart(x + 1, y + 2, wy + 2, index, "|");
            }
            else {
                DrawPart(x + 1, y + 2, wy + 2, index, "|"); DrawPart(x + 2, y + 2, wy + 2, index, "\\");
            }
        }
        else {
            DrawPart(x, y + 2, wy + 2, index, "/"); DrawPart(x + 2, y + 2, wy + 2, index, "\\");
        }
    }
    printf(COLOR_RESET);
}

bool WillHitPlatform(float px, float py, float prev_py, float* hitY) {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        Platform* pf = &platforms[i];
        if (px + 2.0f >= pf->x && px <= pf->x + pf->width) {
            if (prev_py + 3.0f <= pf->y && py + 3.0f >= pf->y) {
                *hitY = (float)(pf->y - 3);
                return true;
            }
        }
    }
    return false;
}

void InitGame(bool multi) {
    isMulti = multi;
    currentPlayer = 0;
    isCharging = false;
    cameraY = 0;
    currentZone = -1;
    areaNotiTimer = 0;

    players[0] = (Player){ 10, 24, 0, 0, false, false, 0 };
    players[1] = (Player){ 60, 24, 0, 0, false, false, 0 };
    clear_screen();
}

void Input() {
    Player* p = &players[currentPlayer];
    int leftKey = (!isMulti || currentPlayer == 0) ? 'A' : VK_LEFT;
    int rightKey = (!isMulti || currentPlayer == 0) ? 'D' : VK_RIGHT;
    int jumpKey = (!isMulti) ? VK_SPACE : (currentPlayer == 0 ? 'W' : VK_UP);

    bool movingThisFrame = false;

    if (GetAsyncKeyState(leftKey) & 0x8000) {
        moveDir[currentPlayer] = -1;
        if (!isCharging && !p->isJumping) { p->x -= 1.0f; movingThisFrame = true; }
    }
    if (GetAsyncKeyState(rightKey) & 0x8000) {
        moveDir[currentPlayer] = 1;
        if (!isCharging && !p->isJumping) { p->x += 1.0f; movingThisFrame = true; }
    }

    p->isMoving = movingThisFrame;

    if (GetAsyncKeyState(jumpKey) & 0x8000) {
        if (!isCharging && !p->isJumping) {
            isCharging = true;
            chargeStart = GetTickCount64();
        }
        else if (isCharging) {
            float ratio = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
            if (ratio >= 1.0f) {
                // [수정 포인트 5] 기본 점프력(0.8f) 및 가속도를 줄여 부드럽게 점프
                p->vx = 1.0f * MAX_JUMP_VX * moveDir[currentPlayer];
                p->vy = -(1.0f * MAX_JUMP_VY + 0.8f);
                p->isJumping = true;
                p->isMoving = false;
                isCharging = false;
                PlaySound(TEXT("jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
            }
        }
    }
    else {
        if (isCharging) {
            float ratio = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
            if (ratio > 1.0f) ratio = 1.0f;

            p->vx = ratio * MAX_JUMP_VX * moveDir[currentPlayer];
            p->vy = -(ratio * MAX_JUMP_VY + 0.8f);
            p->isJumping = true;
            p->isMoving = false;
            isCharging = false;

            PlaySound(TEXT("jump.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }
    }
}

void Update() {
    Player* p = &players[currentPlayer];
    float prevY = p->y;

    if (p->x < 1.0f) { p->x = 1.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }
    if (p->x > WIDTH - 3.0f) { p->x = WIDTH - 3.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }

    if (p->isJumping) {
        p->vy += GRAVITY;
        p->x += p->vx;
        p->y += p->vy;

        if (p->x < 1.0f) { p->x = 1.0f; p->vx = -p->vx * 0.5f; }
        if (p->x > WIDTH - 3.0f) { p->x = WIDTH - 3.0f; p->vx = -p->vx * 0.5f; }

        if (p->vy < 0) {
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                Platform* pf = &platforms[i];
                if (p->x + 2.0f >= pf->x && p->x <= pf->x + pf->width) {
                    if (prevY >= pf->y && p->y <= pf->y) {
                        p->y = (float)(pf->y + 1);
                        p->vy = 0.5f;
                        p->vx *= 0.5f;
                        break;
                    }
                }
            }
        }

        if (p->vy > 0) {
            float hitY;
            if (WillHitPlatform(p->x, p->y, prevY, &hitY)) {
                p->y = hitY;
                p->vy = 0; p->vx = 0; p->isJumping = false;
                if (isMulti) currentPlayer = (currentPlayer + 1) % 2;

                PlaySound(TEXT("land.wav"), NULL, SND_ASYNC | SND_FILENAME);
            }
        }
    }
    else {
        float dummyY;
        if (!WillHitPlatform(p->x, p->y + 0.1f, p->y, &dummyY)) {
            p->isJumping = true;
            p->vy = 0.15f;
            p->vx = 0;
        }
    }

    if (p->isMoving) p->walkTimer++; else p->walkTimer = 0;

    cameraY = (int)p->y - 12;
    if (cameraY > 0) cameraY = 0;

    int newZone = GetZone(p->y);
    if (newZone != currentZone) {
        currentZone = newZone;
        areaNotiTimer = 120;
    }
}

void Render() {
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT; i++) {
        int realY = cameraY + i;
        SetZoneColor(realY);
        move_cursor(1, i + 1);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        int drawY = platforms[i].y - cameraY;
        int zone = GetZone((float)platforms[i].y);

        if (drawY >= 0 && drawY < HEIGHT) {
            move_cursor(platforms[i].x, drawY);
            if (zone == 0) set_bg_color(100);
            else if (zone == 1) set_bg_color(42);
            else { set_bg_color(107); set_font_color(30); }

            if (zone == 2) {
                printf("("); for (int j = 1; j < platforms[i].width - 1; j++) printf(" "); printf(")");
            }
            else {
                for (int j = 0; j < platforms[i].width; j++) printf(" ");
            }
        }
        if (drawY + 1 >= 0 && drawY + 1 < HEIGHT) {
            move_cursor(platforms[i].x, drawY + 1);
            if (zone == 0) set_bg_color(100);
            else if (zone == 1) set_bg_brown();
            else { set_bg_color(107); set_font_color(30); }

            if (zone == 2) {
                printf("("); for (int j = 1; j < platforms[i].width - 1; j++) printf(" "); printf(")");
            }
            else {
                for (int j = 0; j < platforms[i].width; j++) printf(" ");
            }
        }
    }

    if (isCharging) {
        float ratio = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
        if (ratio > 1.0f) ratio = 1.0f;

        float simVx = ratio * MAX_JUMP_VX * moveDir[currentPlayer];
        float simVy = -(ratio * MAX_JUMP_VY + 0.8f);

        float simX = players[currentPlayer].x;
        float simY = players[currentPlayer].y;

        for (int k = 0; k < 60; k++) {
            float prevSimY = simY;
            simVy += GRAVITY;
            simX += simVx;
            simY += simVy;

            if (simX < 1.0f) { simX = 1.0f; simVx = -simVx * 0.5f; }
            if (simX > WIDTH - 3.0f) { simX = WIDTH - 3.0f; simVx = -simVx * 0.5f; }

            float hitY;
            if (simVy > 0 && WillHitPlatform(simX, simY, prevSimY, &hitY)) {
                int ix = (int)simX;
                int iy = (int)(hitY - cameraY);
                if (iy >= 0 && iy < HEIGHT) {
                    SetZoneColor((int)hitY);
                    set_font_color(91);
                    move_cursor(ix + 1, iy); printf("X");
                }
                break;
            }

            int ix = (int)simX;
            int iy = (int)(simY - cameraY);
            if (iy >= 0 && iy < HEIGHT && k % 3 == 0) {
                SetZoneColor((int)simY);
                set_font_color(97);
                move_cursor(ix + 1, iy); printf(".");
            }
        }
    }

    int count = isMulti ? 2 : 1;
    for (int i = 0; i < count; i++) {
        int drawY = (int)players[i].y - cameraY;
        DrawCharacter((int)players[i].x, drawY, i);
    }

    SetZoneColor(cameraY + 1);
    move_cursor(2, 2);
    set_font_color(97);
    char* zname = (currentZone == 0) ? "지하" : (currentZone == 1) ? "지상" : "하늘";
    int height_m = (27 - (int)players[currentPlayer].y) / 2;
    printf("[%s] 현재 높이 : %dm", zname, height_m);

    SetZoneColor(cameraY + 2); move_cursor(2, 3); printf("ESC : 메뉴로 돌아가기");

    if (areaNotiTimer > 0) {
        areaNotiTimer--;
        int cy = 10;
        SetZoneColor(cameraY + cy);
        set_font_color(97);
        move_cursor(50, cy);
        if (currentZone == 0) printf("=== [ 지 하 ] ===");
        else if (currentZone == 1) printf("=== [ 지 상 ] ===");
        else if (currentZone == 2) printf("=== [ 하 늘 ] ===");
    }

    printf(COLOR_RESET);
    fflush(stdout);
}

void RunGame(bool multi) {
    InitGame(multi);
    while (1) {
        Input();
        Update();
        Render();
        Sleep(16);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }
    clear_screen();
}

void ShowHowToPlay() {
    printf("\x1b[H");
    for (int i = 0; i < HEIGHT; i++) {
        move_cursor(1, i + 1);
        set_bg_color(44);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }
    set_font_color(96);
    move_cursor(56, 6); printf("게임 방법");
    printf(COLOR_RESET); set_bg_color(44); set_font_color(97);
    move_cursor(40, 8); printf("◆=========================================◆");

    move_cursor(40, 10); printf("[싱글 플레이]");
    move_cursor(40, 11); printf("A / D : 좌우 이동");
    move_cursor(40, 12); printf("SPACE : 점프 차징");

    move_cursor(40, 14); printf("[멀티 플레이]");
    move_cursor(40, 15); printf("P1 : A / D (이동) , W (점프)");
    move_cursor(40, 16); printf("P2 : ← / → (이동) , ↑ (점프)");

    move_cursor(40, 18); printf("TIP");
    move_cursor(40, 19); printf("- 차징 게이지에 비례하여 궤적이 자라납니다.");
    move_cursor(40, 20); printf("- 궤적 끝의 X 표시에 정확히 착지합니다.");
    move_cursor(40, 21); printf("- 테마별로 끝까지 올라가 하늘에 도달해보세요!");

    move_cursor(40, 28); printf("ESC : 메뉴로 돌아가기");
    fflush(stdout);
    (void)_getch();
}

// [수정 포인트 3] 크레딧 룸: 깜빡임 제거 및 배경 색상 단순화
void RenderHall() {
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT; i++) {
        move_cursor(1, i + 1);
        set_bg_color(40); // 칠하지 않고 기본 검정색 배경 사용
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }

    set_bg_color(40); set_font_color(93);
    move_cursor(55, 4); printf("C R E D I T");
    move_cursor(47, 6); set_font_color(97); printf("- 게임 제작에 참여한 사람들 -");

    for (int i = 0; i < 3; i++) {
        if (i == 0) set_font_color(92);
        else if (i == 1) set_font_color(93);
        else set_font_color(95);

        move_cursor(npcs[i].x, npcs[i].y);
        if (i == 0) { printf("\\O/"); move_cursor(npcs[i].x + 1, npcs[i].y + 1); printf("|"); }
        else if (i == 1) { printf(">O<"); move_cursor(npcs[i].x, npcs[i].y + 1); printf("(|)"); }
        else { printf(" O "); move_cursor(npcs[i].x, npcs[i].y + 1); printf("]I["); }

        move_cursor(npcs[i].x, npcs[i].y + 2); printf("/"); move_cursor(npcs[i].x + 2, npcs[i].y + 2); printf("\\");
        move_cursor(npcs[i].x - 1, npcs[i].y + 4); set_font_color(97); printf("%s", npcs[i].name);
    }

    if (activeBubbleNpc != -1) {
        NPC* n = &npcs[activeBubbleNpc];
        int bx = n->x - 10;
        int by = n->y - 6;

        set_font_color(97);
        move_cursor(bx, by);     printf(" ------------------------- ");
        move_cursor(bx, by + 1); printf("  [ "); set_font_color(96); printf("%s", n->name); set_font_color(97); printf(" ]");
        move_cursor(bx, by + 2); printf("  %s", n->desc1);
        move_cursor(bx, by + 3); printf("  %s", n->desc2);
        move_cursor(bx, by + 4); printf(" ----------\\ /------------ ");
    }

    int px = hallX;
    int py = hallY;
    set_font_color(97);
    move_cursor(px + 1, py); printf("O");
    move_cursor(px, py + 1); printf("(|)");
    if (hallIsMoving) {
        if ((hallWalkTimer / 4) % 2 == 0) {
            move_cursor(px, py + 2); printf("/"); move_cursor(px + 1, py + 2); printf("|");
        }
        else {
            move_cursor(px + 1, py + 2); printf("|"); move_cursor(px + 2, py + 2); printf("\\");
        }
    }
    else {
        move_cursor(px, py + 2); printf("/"); move_cursor(px + 2, py + 2); printf("\\");
    }

    set_font_color(90);
    move_cursor(1, 27); for (int i = 0; i < WIDTH; i++) putchar('-');

    set_font_color(97);
    move_cursor(2, 2);
    printf("A, D: 이동   E: 대화(ON/OFF)   ESC: 종료");

    if (currentNpc != -1 && activeBubbleNpc == -1) {
        move_cursor(2, 4);
        set_font_color(96);
        printf("가까이 왔습니다! E키를 눌러보세요.");
    }

    printf(COLOR_RESET);
    fflush(stdout);
}

int GetNearNpc() {
    for (int i = 0; i < 3; i++) {
        if (abs(hallX - npcs[i].x) < 5) return i;
    }
    return -1;
}

void RunCreditHall() {
    hallX = 10;
    hallY = 24;
    hallWalkTimer = 0;
    activeBubbleNpc = -1;
    bool ePressed = false;

    while (1) {
        hallIsMoving = false;

        if (GetAsyncKeyState('A') & 0x8000) { hallX--; hallIsMoving = true; }
        if (GetAsyncKeyState('D') & 0x8000) { hallX++; hallIsMoving = true; }

        if (hallX < 1) hallX = 1;
        if (hallX > WIDTH - 3) hallX = WIDTH - 3;

        if (hallIsMoving) hallWalkTimer++;
        else hallWalkTimer = 0;

        currentNpc = GetNearNpc();
        if (currentNpc != activeBubbleNpc) activeBubbleNpc = -1;

        if (GetAsyncKeyState('E') & 0x8000) {
            if (!ePressed && currentNpc != -1) {
                if (activeBubbleNpc == currentNpc) activeBubbleNpc = -1;
                else activeBubbleNpc = currentNpc;
                ePressed = true;
            }
        }
        else {
            ePressed = false;
        }

        RenderHall();

        if (hallIsMoving) Sleep(30);
        else Sleep(16);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }
    clear_screen();
}

void ShowLogo() {
    clear_screen();
    for (int i = 0; i < HEIGHT; i++) {
        move_cursor(1, i + 1); set_bg_color(40);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }
    set_font_color(96);
    move_cursor(45, 10); printf("  .__________________________. ");
    move_cursor(45, 11); printf(" /                            \\ ");
    move_cursor(45, 12); printf(" |    _  __  __  __   ____    |");
    move_cursor(45, 13); printf(" |   | |/ / |  \\/  | |  _ \\   |");
    move_cursor(45, 14); printf(" |   | ' /  | |\\/| | | |_) |  |");
    move_cursor(45, 15); printf(" |   | . \\  | |  | | |  __/   |");
    move_cursor(45, 16); printf(" |   |_|\\_\\ |_|  |_| |_|      |");
    move_cursor(45, 17); printf(" \\                            / ");
    move_cursor(45, 18); printf("  .--------------------------. ");
    move_cursor(51, 19); printf("  == KMP STUDIO ==");
    fflush(stdout);
    PlaySound(TEXT("logo.wav"), NULL, SND_ASYNC | SND_FILENAME);
    Sleep(2000);
    clear_screen();
}

// [수정 포인트 2] 하늘색 테마와 흙/잔디 디테일 추가
void DrawMenu(int menu) {
    printf("\x1b[H");

    // 하늘 (Sky Blue)
    for (int i = 0; i < HEIGHT - 2; i++) {
        move_cursor(1, i + 1);
        set_bg_color(104);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }

    // 잔디 (Light Green)
    move_cursor(1, HEIGHT - 1);
    set_bg_color(102); set_font_color(32);
    for (int j = 0; j < WIDTH; j++) {
        if (j % 3 == 0) putchar('v'); else putchar(' ');
    }

    // 흙 (Brown)
    move_cursor(1, HEIGHT);
    set_bg_brown(); set_font_color(30);
    for (int j = 0; j < WIDTH; j++) {
        if (j % 4 == 0) putchar('%'); else if (j % 7 == 0) putchar('.'); else putchar(' ');
    }

    set_bg_color(104); set_font_color(97);
    move_cursor(25, 6); printf("   . . .   ");
    move_cursor(23, 7); printf(" .         . ");
    move_cursor(22, 8); printf(".   CLOUD   .");
    move_cursor(23, 9); printf(" .         . ");
    move_cursor(25, 10); printf("   . . .   ");

    move_cursor(85, 12); printf("   . . .   ");
    move_cursor(83, 13); printf(" .         . ");
    move_cursor(82, 14); printf(".   CLOUD   .");
    move_cursor(83, 15); printf(" .         . ");
    move_cursor(85, 16); printf("   . . .   ");

    // [수정 포인트 1] 딱딱한 검은 박스 제거, 텍스트만 깔끔하게 출력
    set_font_color(97);
    move_cursor(43, 6); printf("A S C I I   J U M P   G A M E");

    int startY = 13;
    char* menus[] = { "싱글 게임", "멀티 게임", "플레이 방법", "크레딧", "나가기" };
    for (int i = 0; i < 5; i++) {
        move_cursor(53, startY + i * 2);
        if (menu == i) {
            set_bg_color(104); set_font_color(93); // 선택 시 노란색 텍스트
            printf("▶ %s", menus[i]);
        }
        else {
            set_bg_color(104); set_font_color(97); // 비선택 시 흰색 텍스트
            printf("   %s", menus[i]);
        }
    }

    // 잔디 위 캐릭터 장식
    set_bg_color(104); set_font_color(92);
    move_cursor(93, 26); printf("\\O/");
    move_cursor(94, 27); printf("|");
    move_cursor(93, 28); printf("/"); move_cursor(95, 28); printf("\\");

    printf(COLOR_RESET);
    fflush(stdout);
}

int main() {
    system("mode con cols=120 lines=32");

    // [수정 포인트 3] 출력 버퍼를 65536 바이트로 대폭 증가시켜 화면 깜빡임 완벽 제거
    setvbuf(stdout, printBuffer, _IOFBF, sizeof(printBuffer));

    int menu = 0;
    char input;

    hide_cursor();
    ShowLogo();

    while (1) {
        DrawMenu(menu);
        input = _getch();

        if (input == 27) break;
        if (input == 'w' || input == 'W') { if (menu > 0) menu--; }
        if (input == 's' || input == 'S') { if (menu < 4) menu++; }
        if (input == ' ') {
            if (menu == 0) RunGame(false);
            else if (menu == 1) RunGame(true);
            else if (menu == 2) ShowHowToPlay();
            else if (menu == 3) RunCreditHall();
            else if (menu == 4) break;
        }
        PlaySound(TEXT("menu.wav"), NULL, SND_ASYNC | SND_FILENAME);
    }

    show_cursor();
    clear_screen();
    move_cursor(55, 12);
    printf("GAME ENDED");
    fflush(stdout);

    return 0;
}