#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <stdbool.h>

// =====================================================
// KMP STUDIO - ASCII JUMP GAME
// =====================================================

#define WIDTH 100
#define HEIGHT 30

#define MAX_GAUGE 1000.0f
#define PLATFORM_COUNT 56

// [수정 포인트 2, 3] 가로 점프력을 높이고 세로 점프력을 조정
#define GRAVITY 0.25f
#define MAX_JUMP_VX 4.8f 
#define MAX_JUMP_VY 8.5f 

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
int currentZone = -1;    // [수정 포인트 7] 초기값을 -1로 하여 시작 시 "지하"가 뜨도록 수정
int areaNotiTimer = 0;

int hallX = 10;
int hallY = 24;
int hallWalkTimer = 0;
bool hallIsMoving = false;
int currentNpc = -1;
int activeBubbleNpc = -1;

NPC npcs[3] = {
    { 20, 24, "김승주", "플랫폼 시스템 개발", "점프 물리 구현" },
    { 50, 24, "문용성", "맵 설계 담당", "UI 디자인 담당" },
    { 80, 24, "박정원", "게임 총괄", "크레딧 시스템 제작" }
};

// [수정 포인트 3] 가로 점프력 증가에 맞춘 플랫폼 X좌표 간격 확장
Platform platforms[PLATFORM_COUNT] = {
    // 지하
    {0, 27, WIDTH}, {30, 23, 15}, {70, 19, 10}, {40, 15, 12}, {10, 11, 10},
    {55, 7, 10}, {85, 3, 10}, {45, -1, 12}, {15, -5, 10}, {75, -9, 10},
    {20, -13, 10}, {60, -17, 12}, {85, -21, 10}, {40, -25, 12}, {10, -31, 15}, {70, -37, 15},
    // 지상
    {30, -43, 10}, {80, -47, 12}, {20, -51, 10}, {60, -55, 12}, {10, -59, 15},
    {50, -63, 10}, {85, -67, 10}, {35, -71, 10}, {5, -75, 12}, {75, -79, 10},
    {45, -83, 10}, {15, -87, 12}, {60, -91, 10}, {85, -95, 15}, {30, -99, 10},
    {70, -103, 10}, {20, -107, 12}, {60, -111, 10}, {10, -115, 10}, {50, -119, 15},
    // 하늘
    {80, -125, 10}, {30, -131, 12}, {60, -137, 10}, {15, -143, 10}, {75, -149, 10},
    {40, -155, 15}, {85, -161, 10}, {25, -167, 10}, {65, -173, 10}, {15, -179, 12},
    {55, -185, 10}, {85, -191, 12}, {35, -197, 10}, {5, -203, 10}, {45, -209, 10},
    {80, -215, 12}, {25, -221, 10}, {65, -227, 15}, {15, -233, 10}, {40, -240, 20}
};

// =====================
// 유틸리티 함수
// =====================
void move_cursor(int x, int y) { printf("\x1b[%d;%dH", y, x); }
void set_font_color(int code) { printf("\x1b[%dm", code); }
void set_bg_color(int code) { printf("\x1b[%dm", code); }
// [수정 포인트 8] RGB 방식을 사용한 실제 흙(갈색) 색상
void set_bg_brown() { printf("\x1b[48;5;130m"); }
void hide_cursor() { printf("\x1b[?25l"); }
void show_cursor() { printf("\x1b[?25h"); }
void clear_screen() { printf("\x1b[2J\x1b[3J\x1b[H"); }

int GetZone(float y) {
    if (y > -40) return 0;       // 지하
    if (y > -120) return 1;      // 지상
    return 2;                    // 하늘
}

void SetZoneColor(int y) {
    int zone = GetZone((float)y);
    if (zone == 0) set_bg_color(40);
    else if (zone == 1) set_bg_color(104);
    else set_bg_color(106);
}

void SetCharColor(int index, int y) {
    SetZoneColor(y);
    if (isMulti) {
        if (index == 0) set_font_color(92);
        else set_font_color(93);
    }
    else {
        set_font_color(97);
    }
}

void DrawCharacter(int x, int y, int index) {
    Player* p = &players[index];
    if (y < -2 || y > HEIGHT) return;

    if (isCharging && currentPlayer == index) {
        SetCharColor(index, y);     move_cursor(x, y);     printf(" O ");
        SetCharColor(index, y + 1); move_cursor(x, y + 1); printf("└|┘");
        SetCharColor(index, y + 2); move_cursor(x, y + 2); printf("/ \\");
    }
    else if (p->isJumping) {
        SetCharColor(index, y);     move_cursor(x, y);     printf("\\O/");
        SetCharColor(index, y + 1); move_cursor(x, y + 1); printf(" | ");
        SetCharColor(index, y + 2); move_cursor(x, y + 2); printf("/ \\");
    }
    else {
        SetCharColor(index, y);     move_cursor(x, y);     printf(" O ");
        SetCharColor(index, y + 1); move_cursor(x, y + 1); printf("(|)");
        SetCharColor(index, y + 2); move_cursor(x, y + 2);
        if (p->isMoving) {
            if ((p->walkTimer / 4) % 2 == 0) printf("/| ");
            else                             printf(" |\\");
        }
        else {
            printf("/ \\");
        }
    }
    printf(COLOR_RESET);
}

// [수정 포인트 6] 픽셀 단위로 정확하게 발이 플랫폼에 걸쳐있는지 확인
bool WillHitPlatform(float px, float py, float prev_py, float* hitY) {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        Platform* pf = &platforms[i];
        // 캐릭터의 넓이(3칸) 중 1칸이라도 플랫폼 위에 있으면 착지 인정
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
    currentZone = -1; // 시작 시 지역 알림을 띄우기 위해 -1로 초기화
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
    }
    else {
        if (isCharging) {
            // [수정 포인트 1] 점프 게이지 최대치 제한 (1.0f를 넘지 못함)
            float ratio = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
            if (ratio > 1.0f) ratio = 1.0f;

            // [수정 포인트 2] 살짝 누르면 낮게 뛰도록 최소 점프력 보정 (0.5f)
            p->vx = ratio * MAX_JUMP_VX * moveDir[currentPlayer];
            p->vy = -(ratio * MAX_JUMP_VY + 0.5f);

            p->isJumping = true;
            p->isMoving = false;
            isCharging = false;
        }
    }
}

void Update() {
    Player* p = &players[currentPlayer];
    float prevY = p->y;

    // [수정 포인트 4, 5] 벽 통과, 깜빡임, 하반신 버그 원천 차단 (위치 강제 고정)
    if (p->x < 1.0f) { p->x = 1.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }
    if (p->x > WIDTH - 3.0f) { p->x = WIDTH - 3.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }

    if (p->isJumping) {
        p->vy += GRAVITY;
        p->x += p->vx;
        p->y += p->vy;

        // 이동 후 다시 벽 체크
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
            }
        }
    }
    else {
        float dummyY;
        // [수정 포인트 6] 걸어갈 때 플랫폼 가장자리 판정 완벽 동기화
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
        printf("                                                                                                     ");
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
            else if (zone == 1) set_bg_brown();   // [수정 포인트 8] 흙 갈색 적용
            else { set_bg_color(107); set_font_color(30); }

            // [수정 포인트 9] 하늘 구름 밑부분도 괄호로 예쁘게 마무리
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
        if (ratio > 1.0f) ratio = 1.0f; // 궤적도 최대치 제한

        float simVx = ratio * MAX_JUMP_VX * moveDir[currentPlayer];
        float simVy = -(ratio * MAX_JUMP_VY + 0.5f);

        float simX = players[currentPlayer].x;
        float simY = players[currentPlayer].y;

        for (int k = 0; k < 60; k++) {
            float prevSimY = simY;
            simVy += GRAVITY;
            simX += simVx;
            simY += simVy;

            // 궤적 시뮬레이션도 정확한 벽 충돌 적용
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
    move_cursor(1, 1);
    set_font_color(97);
    char* zname = (currentZone == 0) ? "지하" : (currentZone == 1) ? "지상" : "하늘";
    int height_m = (27 - (int)players[currentPlayer].y) / 2;
    printf("[%s] 현재 높이 : %dm", zname, height_m);

    SetZoneColor(cameraY + 2); move_cursor(1, 2); printf("ESC : 메뉴로 돌아가기");

    if (areaNotiTimer > 0) {
        areaNotiTimer--;
        int cy = 10;
        SetZoneColor(cameraY + cy);
        set_font_color(97);
        move_cursor(40, cy);
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
    clear_screen();
    set_font_color(96);
    move_cursor(45, 6); printf("게임 방법");
    printf(COLOR_RESET);
    move_cursor(30, 8); printf("◆=========================================◆");

    move_cursor(30, 10); printf("[싱글 플레이]");
    move_cursor(30, 11); printf("A / D : 좌우 이동");
    move_cursor(30, 12); printf("SPACE : 점프 차징");

    move_cursor(30, 14); printf("[멀티 플레이]");
    move_cursor(30, 15); printf("P1 : A / D (이동) , W (점프)");
    move_cursor(30, 16); printf("P2 : ← / → (이동) , ↑ (점프)");

    move_cursor(30, 18); printf("TIP");
    move_cursor(30, 19); printf("- 차징 게이지에 비례하여 궤적이 자라납니다.");
    move_cursor(30, 20); printf("- 궤적 끝의 X 표시에 정확히 착지합니다.");
    move_cursor(30, 21); printf("- 테마별로 끝까지 올라가 하늘에 도달해보세요!");

    move_cursor(30, 28); printf("ESC : 메뉴로 돌아가기");
    fflush(stdout);
    (void)_getch();
}

void RenderHall() {
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT; i++) {
        set_bg_color(40);
        move_cursor(1, i + 1);
        printf("                                                                                                     ");
    }

    set_font_color(96);
    move_cursor(33, 2); printf("KMP STUDIO HALL OF FAME");
    printf(COLOR_RESET);

    set_bg_color(40);
    move_cursor(1, 27);
    for (int i = 0; i < WIDTH; i++) printf("=");

    for (int i = 0; i < 3; i++) {
        set_font_color(97);
        move_cursor(npcs[i].x, npcs[i].y);     printf(" O ");
        move_cursor(npcs[i].x, npcs[i].y + 1); printf("(|)");
        move_cursor(npcs[i].x, npcs[i].y + 2); printf("/ \\");
        move_cursor(npcs[i].x - 3, npcs[i].y + 4); printf("%s", npcs[i].name);
    }

    if (activeBubbleNpc != -1) {
        NPC* n = &npcs[activeBubbleNpc];
        int bx = n->x - 10;
        int by = n->y - 6;

        move_cursor(bx, by);     printf(" ------------------------- ");
        move_cursor(bx, by + 1); printf("  [ "); set_font_color(96); printf("%s", n->name); set_font_color(97); printf(" ]");
        move_cursor(bx, by + 2); printf("  %s", n->desc1);
        move_cursor(bx, by + 3); printf("  %s", n->desc2);
        move_cursor(bx, by + 4); printf(" ----------\\ /------------ ");
    }

    set_font_color(97);
    move_cursor(hallX, hallY);     printf(" O ");
    move_cursor(hallX, hallY + 1); printf("(|)");
    move_cursor(hallX, hallY + 2);
    if (hallIsMoving) {
        if ((hallWalkTimer / 4) % 2 == 0) printf("/| ");
        else                              printf(" |\\");
    }
    else {
        printf("/ \\");
    }

    move_cursor(1, 1);
    printf("A, D: 이동   E: 대화(ON/OFF)   ESC: 종료");

    if (currentNpc != -1 && activeBubbleNpc == -1) {
        move_cursor(1, 3);
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
    set_font_color(96);
    move_cursor(30, 6);  printf("  .__________________________. ");
    move_cursor(30, 7);  printf(" /                            \\ ");
    move_cursor(30, 8);  printf(" |    _  __  __  __   ____    |");
    move_cursor(30, 9);  printf(" |   | |/ / |  \\/  | |  _ \\   |");
    move_cursor(30, 10); printf(" |   | ' /  | |\\/| | | |_) |  |");
    move_cursor(30, 11); printf(" |   | . \\  | |  | | |  __/   |");
    move_cursor(30, 12); printf(" |   |_|\\_\\ |_|  |_| |_|      |");
    move_cursor(30, 13); printf(" \\                            / ");
    move_cursor(30, 14); printf("  .--------------------------. ");
    move_cursor(36, 15); printf("  == KMP STUDIO ==");
    fflush(stdout);
    Sleep(2000);
    clear_screen();
}

void DrawMenu(int menu) {
    clear_screen();
    set_font_color(96);
    move_cursor(32, 6); printf("◆==== ASCII JUMP GAME ====◆");
    printf(COLOR_RESET);

    move_cursor(38, 10);
    if (menu == 0) { set_bg_color(43); set_font_color(30); printf(" ▶ 싱글 게임 "); }
    else { printf(" 싱글 게임 "); }
    printf(COLOR_RESET);

    move_cursor(38, 12);
    if (menu == 1) { set_bg_color(43); set_font_color(30); printf(" ▶ 멀티 게임 "); }
    else { printf(" 멀티 게임 "); }
    printf(COLOR_RESET);

    move_cursor(38, 14);
    if (menu == 2) { set_bg_color(43); set_font_color(30); printf(" ▶ 플레이 방법 "); }
    else { printf(" 플레이 방법 "); }
    printf(COLOR_RESET);

    move_cursor(38, 16);
    if (menu == 3) { set_bg_color(43); set_font_color(30); printf(" ▶ 크레딧 "); }
    else { printf(" 크레딧 "); }
    printf(COLOR_RESET);

    move_cursor(38, 18);
    if (menu == 4) { set_bg_color(43); set_font_color(30); printf(" ▶ 나가기"); }
    else { printf(" 나가기 "); }
    printf(COLOR_RESET);

    fflush(stdout);
}

int main() {
    system("mode con cols=105 lines=32");
    setvbuf(stdout, NULL, _IOFBF, 8192);

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
    }

    show_cursor();
    clear_screen();
    move_cursor(40, 12);
    printf("GAME ENDED");
    fflush(stdout);

    return 0;
}