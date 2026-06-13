#define _CRT_SECURE_NO_WARNINGS

// 표준 입출력 함수 사용
#include <stdio.h>
// 동적 메모리 관련 함수 사용 및 system 함수 사용
#include <stdlib.h>
// _getch() 함수 사용
#include <conio.h>
// Windows API 사용
#include <Windows.h>
// bool 자료형 사용
#include <stdbool.h>

// =====================================================
// KMP STUDIO
// ASCII JUMP GAME - JUMP KING VERSION
// =====================================================

// 콘솔 화면 가로 크기
#define WIDTH 100
// 콘솔 화면 세로 크기
#define HEIGHT 30

#define MAX_GAUGE 2500
#define TRAIL_MAX 200
#define PLATFORM_COUNT 25

// 기본 색상 복구
#define COLOR_RESET "\x1b[0m"

// =====================
// 구조체 정의
// =====================
typedef struct
{
    float x;
    float y;
    float vx;
    float vy;
    bool isJumping;
} Player;

typedef struct
{
    int x;
    int y;
    int width;
} Platform;

typedef struct
{
    int x;
    int y;
    char* name;
    char* desc1;
    char* desc2;
} NPC;

// 함수 선언
void RenderHall();
int GetNearNpc();
void RunCreditHall();

// =====================
// 전역 변수
// =====================
Player players[2];
bool isCharging = false;
bool isMulti = false;
ULONGLONG chargeStart = 0;
float currentPower = 0;
int currentPlayer = 0;
int moveDir[2] = { 1, 1 };

int cameraY = 0;
int hallX = 10;
int hallY = 24;
int currentNpc = -1;

// 말풍선 렌더링용 변수 (현재 대화중인 NPC 인덱스, -1이면 대화 안함)
int activeBubbleNpc = -1;

// [수정 포인트 3] NPC들의 y좌표를 플레이어(24)와 동일하게 맞춰 바닥에 닿게 수정
NPC npcs[3] =
{
    { 20, 24, "김승주", "플랫폼 시스템 개발", "점프 물리 구현" },
    { 50, 24, "문용성", "맵 설계 담당", "UI 디자인 담당" },
    { 80, 24, "박정원", "게임 총괄", "크레딧 시스템 제작" }
};

// =====================
// 맵 데이터
// =====================
Platform platforms[PLATFORM_COUNT] =
{
    {0, 27, WIDTH},
    {40, 23, 10},
    {20, 19, 10},
    {55, 15, 12},
    {75, 11, 10},
    {50, 7, 10},
    {28, 3, 10},
    {8, -1, 10},
    {25, -5, 10},
    {48, -9, 10},
    {70, -13, 10},
    {45, -17, 10},
    {22, -21, 10},
    {5, -25, 10},
    {30, -29, 12},
    {55, -33, 10},
    {78, -37, 10},
    {60, -41, 10},
    {38, -45, 10},
    {15, -49, 10},
    {35, -53, 10},
    {58, -57, 10},
    {82, -61, 10},
    {50, -65, 15}
};

// =====================
// 유틸리티 함수
// =====================
void move_cursor(int x, int y)
{
    printf("\x1b[%d;%dH", y, x);
}

void set_font_color(int code)
{
    printf("\x1b[%dm", code);
}

void set_bg_color(int code)
{
    printf("\x1b[%dm", code);
}

void hide_cursor()
{
    printf("\x1b[?25l");
}

void show_cursor()
{
    printf("\x1b[?25h");
}

void clear_screen()
{
    // 화면 전체 초기화 및 스크롤바 정리
    printf("\x1b[2J\x1b[3J\x1b[H");
}

void ShowLogo()
{
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

void DrawCharacter(int x, int y, int index)
{
    Player* p = &players[index];

    if (y < -2 || y > HEIGHT) return;

    if (isCharging && currentPlayer == index)
    {
        move_cursor(x, y);     printf(" O ");
        move_cursor(x, y + 1); printf("└|┘");
        move_cursor(x, y + 2); printf("/ \\");
    }
    else if (p->isJumping)
    {
        move_cursor(x, y);     printf("\\O/");
        move_cursor(x, y + 1); printf(" |");
        move_cursor(x, y + 2); printf("\\ /");
    }
    else
    {
        move_cursor(x, y);     printf(" O ");
        move_cursor(x, y + 1); printf("(|)");
        move_cursor(x, y + 2); printf("/ \\");
    }
}

bool CheckPlatform(Player* p, float prevY)
{
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        Platform* pf = &platforms[i];

        if (p->x + 1 >= pf->x && p->x <= pf->x + pf->width)
        {
            if (prevY + 3 <= pf->y && p->y + 3 >= pf->y)
            {
                p->y = pf->y - 3;
                return true;
            }
        }
    }
    return false;
}

bool TrailHitPlatform(float x, float y)
{
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        Platform* pf = &platforms[i];
        if (x >= pf->x && x <= pf->x + pf->width)
        {
            if (y >= pf->y - 1 && y <= pf->y + 1) return true;
        }
    }
    return false;
}

void InitGame(bool multi)
{
    isMulti = multi;
    currentPlayer = 0;
    isCharging = false;
    currentPower = 0;
    cameraY = 0;

    players[0].x = 10;
    players[0].y = 24;
    players[0].vx = 0;
    players[0].vy = 0;
    players[0].isJumping = false;

    players[1].x = 60;
    players[1].y = 24;
    players[1].vx = 0;
    players[1].vy = 0;
    players[1].isJumping = false;

    clear_screen();
}

void Input()
{
    Player* p = &players[currentPlayer];
    int leftKey, rightKey, jumpKey;

    if (!isMulti)
    {
        leftKey = 'A';
        rightKey = 'D';
        jumpKey = VK_SPACE;
    }
    else
    {
        if (currentPlayer == 0)
        {
            leftKey = 'A'; rightKey = 'D'; jumpKey = 'W';
        }
        else
        {
            leftKey = VK_LEFT; rightKey = VK_RIGHT; jumpKey = VK_UP;
        }
    }

    if (GetAsyncKeyState(leftKey) & 0x8000)
    {
        moveDir[currentPlayer] = -1;
        if (!isCharging && !p->isJumping) p->x -= 1.0f;
    }

    if (GetAsyncKeyState(rightKey) & 0x8000)
    {
        moveDir[currentPlayer] = 1;
        if (!isCharging && !p->isJumping) p->x += 1.0f;
    }

    if (GetAsyncKeyState(jumpKey) & 0x8000)
    {
        if (!isCharging && !p->isJumping)
        {
            isCharging = true;
            chargeStart = GetTickCount64();
        }
    }
    else
    {
        if (isCharging)
        {
            ULONGLONG t = GetTickCount64() - chargeStart;
            if (t > MAX_GAUGE) t = MAX_GAUGE;

            currentPower = (float)t / MAX_GAUGE;
            currentPower = currentPower * currentPower;

            p->vx = currentPower * 2.8f * moveDir[currentPlayer];
            p->vy = -(currentPower * 13.5f);
            p->isJumping = true;
            isCharging = false;
        }
    }
}

void Update()
{
    Player* p = &players[currentPlayer];

    if (p->isJumping)
    {
        float prevY = p->y;
        p->vy += 0.15f;
        p->x += p->vx;
        p->y += p->vy;

        if (p->vy > 0)
        {
            if (CheckPlatform(p, prevY))
            {
                p->vy = 0;
                p->vx = 0;
                p->isJumping = false;

                if (isMulti)
                {
                    currentPlayer = (currentPlayer + 1) % 2;
                }
            }
        }
    }

    if (p->x < 0) p->x = 0;
    if (p->x > WIDTH - 4) p->x = WIDTH - 4;

    cameraY = (int)p->y - 12;
    if (cameraY > 0) cameraY = 0;
}

void Render()
{
    // [수정 포인트 1,2] 화면 전체를 공백으로 덮어쓰기 (잔상 방지)
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT; i++)
    {
        move_cursor(1, i + 1);
        printf("                                                                                                    ");
    }

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        int drawY = platforms[i].y - cameraY;
        if (drawY >= 0 && drawY < HEIGHT)
        {
            move_cursor(platforms[i].x, drawY);
            for (int j = 0; j < platforms[i].width; j++) printf("=");
        }
    }

    int count = isMulti ? 2 : 1;

    if (isCharging)
    {
        float power = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
        if (power > 1) power = 1;
        power = power * power;

        float vx = power * 2.8f * moveDir[currentPlayer];
        float vy = -(power * 13.5f);
        float simX = players[currentPlayer].x;
        float simY = players[currentPlayer].y;

        for (int k = 0; k < 60; k++)
        {
            vy += 0.15f;
            simX += vx;
            simY += vy;

            if (TrailHitPlatform(simX, simY + 3)) break;

            int ix = (int)simX;
            int iy = (int)(simY - cameraY);

            if (iy >= 0 && iy < HEIGHT)
            {
                move_cursor(ix, iy);
                printf(".");
            }
        }
    }

    for (int i = 0; i < count; i++)
    {
        int drawY = (int)players[i].y - cameraY;
        DrawCharacter((int)players[i].x, drawY, i);
    }

    move_cursor(1, 1);
    printf("%s", isMulti ? "멀티 모드" : "싱글 모드");
    move_cursor(1, 2);
    printf("높이 : %d", -cameraY);
    move_cursor(1, 3);
    printf("ESC : 메뉴로 돌아가기");

    // 화면 출력을 한 번에 방출 (깜빡임 완벽 제거)
    fflush(stdout);
}

void RunGame(bool multi)
{
    InitGame(multi);

    while (1)
    {
        Input();
        Update();
        Render();
        Sleep(16);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }
    clear_screen();
}

void ShowHowToPlay()
{
    clear_screen();
    set_font_color(96);
    move_cursor(45, 6); printf("게임 방법");
    printf(COLOR_RESET);
    move_cursor(30, 8); printf("◆=========================================◆");

    move_cursor(30, 10); printf("[싱글 플레이]");
    move_cursor(30, 11); printf("A / D : 좌우 이동");
    move_cursor(30, 12); printf("SPACE : 점프 차징");

    move_cursor(30, 14); printf("[멀티 플레이]");
    move_cursor(30, 15); printf("P1 : A / D / W");
    move_cursor(30, 16); printf("P2 : J / L / I");

    move_cursor(30, 18); printf("TIP");
    move_cursor(30, 19); printf("- 점프킹 스타일로 위로 계속 올라갑니다");
    move_cursor(30, 20); printf("- 발판 위에 정확히 착지해야 합니다");
    move_cursor(30, 21); printf("- 차징 세기에 따라 높이가 달라집니다");

    move_cursor(30, 28); printf("ESC : 메뉴로 돌아가기");
    fflush(stdout);
    (void)_getch();
}

// =====================
// 크레딧 (명예의 전당)
// =====================
void RenderHall()
{
    // [수정 포인트 1,2] 화면 전체를 공백으로 덮어쓰기 (잔상 방지)
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT; i++)
    {
        move_cursor(1, i + 1);
        printf("                                                                                                    ");
    }

    set_font_color(96);
    move_cursor(33, 2); printf("KMP STUDIO HALL OF FAME");
    printf(COLOR_RESET);

    move_cursor(1, 27);
    for (int i = 0; i < WIDTH; i++) printf("=");

    // NPC 출력
    for (int i = 0; i < 3; i++)
    {
        move_cursor(npcs[i].x, npcs[i].y);     printf(" O ");
        move_cursor(npcs[i].x, npcs[i].y + 1); printf("(|)");
        move_cursor(npcs[i].x, npcs[i].y + 2); printf("/ \\");
        move_cursor(npcs[i].x - 3, npcs[i].y + 4); printf("%s", npcs[i].name);
    }

    // [수정 포인트 4] E 키를 눌러 활성화된 말풍선이 있으면 NPC 위에 출력
    if (activeBubbleNpc != -1)
    {
        NPC* n = &npcs[activeBubbleNpc];
        int bx = n->x - 10;
        int by = n->y - 6;

        move_cursor(bx, by);     printf(" ------------------------- ");
        move_cursor(bx, by + 1); printf("  [ "); set_font_color(96); printf("%s", n->name); printf(COLOR_RESET); printf(" ]");
        move_cursor(bx, by + 2); printf("  %s", n->desc1);
        move_cursor(bx, by + 3); printf("  %s", n->desc2);
        move_cursor(bx, by + 4); printf(" ----------\\ /------------ ");
    }

    // 플레이어 출력
    move_cursor(hallX, hallY);     printf("\\O/");
    move_cursor(hallX, hallY + 1); printf(" | ");
    move_cursor(hallX, hallY + 2); printf("/ \\");

    // UI 문구 출력
    move_cursor(1, 1);
    printf("A, D: 이동   E: 대화(ON/OFF)   ESC: 종료");

    // 다가가면 상단 안내 추가
    if (currentNpc != -1 && activeBubbleNpc == -1)
    {
        move_cursor(1, 3);
        set_font_color(96);
        printf("가까이 왔습니다! E키를 눌러보세요.");
        printf(COLOR_RESET);
    }

    fflush(stdout); // 깜빡임 방지를 위해 화면 출력을 한 번에 쏨
}

int GetNearNpc()
{
    for (int i = 0; i < 3; i++)
    {
        if (abs(hallX - npcs[i].x) < 5) return i;
    }
    return -1;
}

void RunCreditHall()
{
    hallX = 10;
    hallY = 24;
    activeBubbleNpc = -1; // 진입 시 대화 초기화

    // 키 연속 입력 방지용 변수
    bool ePressed = false;

    while (1)
    {
        bool moved = false;

        if (GetAsyncKeyState('A') & 0x8000)
        {
            hallX--;
            moved = true;
        }

        if (GetAsyncKeyState('D') & 0x8000)
        {
            hallX++;
            moved = true;
        }

        if (hallX < 1) hallX = 1;
        if (hallX > WIDTH - 3) hallX = WIDTH - 3;

        currentNpc = GetNearNpc();

        // NPC에게서 멀어지면 자동으로 말풍선 닫기
        if (currentNpc != activeBubbleNpc)
        {
            activeBubbleNpc = -1;
        }

        // [수정 포인트 4] E 키 토글 기능 (말풍선 ON/OFF)
        if (GetAsyncKeyState('E') & 0x8000)
        {
            if (!ePressed && currentNpc != -1) // E 키를 방금 막 눌렀을 때만
            {
                if (activeBubbleNpc == currentNpc)
                    activeBubbleNpc = -1; // 켜져 있으면 끄기
                else
                    activeBubbleNpc = currentNpc; // 꺼져 있으면 켜기

                ePressed = true;
            }
        }
        else
        {
            ePressed = false; // E 키를 뗐을 때 리셋
        }

        RenderHall();

        if (moved) Sleep(30);
        else Sleep(16);

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }
    clear_screen();
}

void DrawMenu(int menu)
{
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

int main()
{
    // [수정 포인트 1,2] 콘솔 창 크기를 고정시켜 글자가 줄 바꿈 되면서 스크롤이 올라가는 현상 원천 차단
    system("mode con cols=105 lines=32");

    // [수정 포인트 1] printf가 콘솔에 바로 출력되지 않고, 버퍼에 모였다가 fflush할 때 한 번에 쏘게 만듦 (화면 깜빡임 완벽 제거)
    setvbuf(stdout, NULL, _IOFBF, 8192);

    int menu = 0;
    char input;

    hide_cursor();
    ShowLogo();

    while (1)
    {
        DrawMenu(menu);
        input = _getch();

        if (input == 27) break;
        if (input == 'w' || input == 'W') { if (menu > 0) menu--; }
        if (input == 's' || input == 'S') { if (menu < 4) menu++; }
        if (input == ' ')
        {
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