#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <stdbool.h>

// =====================================================
// KMP STUDIO
// ASCII JUMP GAME - JUMP KING VERSION
// =====================================================
//202618622
// =====================
// 설정
// =====================
#define WIDTH 100
#define HEIGHT 30

#define MAX_GAUGE 2500
#define TRAIL_MAX 200

#define PLATFORM_COUNT 25

// =====================
// 색상
// =====================
#define COLOR_RESET "\x1b[0m"

// =====================
// 플레이어
// =====================
typedef struct
{
    float x;
    float y;

    float vx;
    float vy;

    bool isJumping;

} Player;

// =====================
// 플랫폼
// =====================
typedef struct
{
    int x;
    int y;
    int width;

} Platform;

// =====================
// 전역 변수
// =====================
Player players[2];

bool isCharging = false;
bool isMulti = false;

ULONGLONG chargeStart = 0;

float currentPower = 0;

int currentPlayer = 0;

int moveDir[2] = { 1,1 };

// 카메라
int cameraY = 0;

// =====================
// 플랫폼 맵
// =====================
Platform platforms[PLATFORM_COUNT] =
{
    // 바닥
    {0, 27, WIDTH},

    // 점프킹 스타일
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
// ANSI 함수
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
    printf("\x1b[2J");
}

// =====================
// 로고
// =====================
void ShowLogo()
{
    clear_screen();

    set_font_color(96);

    move_cursor(30, 6);
    printf("  .__________________________. ");

    move_cursor(30, 7);
    printf(" /                            \\ ");

    move_cursor(30, 8);
    printf(" |    _  __  __  __   ____    |");

    move_cursor(30, 9);
    printf(" |   | |/ / |  \\/  | |  _ \\   |");

    move_cursor(30, 10);
    printf(" |   | ' /  | |\\/| | | |_) |  |");

    move_cursor(30, 11);
    printf(" |   | . \\  | |  | | |  __/   |");

    move_cursor(30, 12);
    printf(" |   |_|\\_\\ |_|  |_| |_|      |");

    move_cursor(30, 13);
    printf(" \\                            / ");

    move_cursor(30, 14);
    printf("  .--------------------------. ");

    move_cursor(36, 15);
    printf("  == KMP STUDIO ==");

    Sleep(2000);

    clear_screen();
}

// =====================
// 캐릭터 출력
// =====================
void DrawCharacter(int x, int y, int index)
{
    Player* p = &players[index];

    if (y < -2 || y > HEIGHT)
    {
        return;
    }

    if (isCharging && currentPlayer == index)
    {
        move_cursor(x, y);
        printf(" O ");

        move_cursor(x, y + 1);
        printf("└|┘");

        move_cursor(x, y + 2);
        printf("/ \\");
    }
    else if (p->isJumping)
    {
        move_cursor(x, y);
        printf("\\O/");

        move_cursor(x, y + 1);
        printf(" |");

        move_cursor(x, y + 2);
        printf("\\ /");
    }
    else
    {
        move_cursor(x, y);
        printf(" O ");

        move_cursor(x, y + 1);
        printf("(|)");

        move_cursor(x, y + 2);
        printf("/ \\");
    }
}

// =====================
// 플랫폼 충돌
// =====================
bool CheckPlatform(Player* p, float prevY)
{
    int i;

    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        Platform* pf = &platforms[i];

        if (
            p->x + 1 >= pf->x &&
            p->x <= pf->x + pf->width
            )
        {
            if (
                prevY + 3 <= pf->y &&
                p->y + 3 >= pf->y
                )
            {
                p->y = pf->y - 3;

                return true;
            }
        }
    }

    return false;
}

// =====================
// 궤적 충돌
// =====================
bool TrailHitPlatform(float x, float y)
{
    int i;

    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        Platform* pf = &platforms[i];

        if (
            x >= pf->x &&
            x <= pf->x + pf->width
            )
        {
            if (
                y >= pf->y - 1 &&
                y <= pf->y + 1
                )
            {
                return true;
            }
        }
    }

    return false;
}

// =====================
// 게임 초기화
// =====================
void InitGame(bool multi)
{
    isMulti = multi;

    currentPlayer = 0;

    isCharging = false;

    currentPower = 0;

    cameraY = 0;

    // Player1
    players[0].x = 10;
    players[0].y = 24;
    players[0].vx = 0;
    players[0].vy = 0;
    players[0].isJumping = false;

    // Player2
    players[1].x = 60;
    players[1].y = 24;
    players[1].vx = 0;
    players[1].vy = 0;
    players[1].isJumping = false;

    clear_screen();
}

// =====================
// 입력
// =====================
void Input()
{
    Player* p = &players[currentPlayer];

    int leftKey;
    int rightKey;
    int jumpKey;

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
            leftKey = 'A';
            rightKey = 'D';
            jumpKey = 'W';
        }
        else
        {
            leftKey = VK_LEFT;
            rightKey = VK_RIGHT;
            jumpKey = VK_UP;
                ;
        }
    }

    // 좌측
    if (GetAsyncKeyState(leftKey) & 0x8000)
    {
        moveDir[currentPlayer] = -1;

        if (!isCharging && !p->isJumping)
        {
            p->x -= 1.0f;
        }
    }

    // 우측
    if (GetAsyncKeyState(rightKey) & 0x8000)
    {
        moveDir[currentPlayer] = 1;

        if (!isCharging && !p->isJumping)
        {
            p->x += 1.0f;
        }
    }

    // 점프 차징
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
        ////////////////////////////////////////////////////// 점프
        if (isCharging)
        {
            ULONGLONG t =
                GetTickCount64()
                - chargeStart;

            if (t > MAX_GAUGE)
            {
                t = MAX_GAUGE;
            }

            currentPower =
                (float)t / MAX_GAUGE;

            currentPower =
                currentPower * currentPower;

            // 실제 물리
            p->vx =
                currentPower
                * 2.8f
                * moveDir[currentPlayer];

            p->vy =
                -(
                    currentPower
                    * 13.5f
                    );

            p->isJumping = true;

            isCharging = false;
        }
    }
}

// =====================
// 업데이트
// =====================
void Update()
{
    Player* p = &players[currentPlayer];

    if (p->isJumping)
    {
        float prevY = p->y;

        /////////////////////////// 중력
        p->vy += 0.15f;

        // 이동
        p->x += p->vx;
        p->y += p->vy;

        // 플랫폼 충돌
        if (p->vy > 0)
        {
            if (CheckPlatform(p, prevY))
            {
                p->vy = 0;
                p->vx = 0;

                p->isJumping = false;

                if (isMulti)
                {
                    currentPlayer =
                        (currentPlayer + 1) % 2;
                }
            }
        }
    }

    // 좌우 경계
    if (p->x < 0)
    {
        p->x = 0;
    }

    if (p->x > WIDTH - 4)
    {
        p->x = WIDTH - 4;
    }

    // 카메라
    cameraY =
        (int)p->y - 12;

    if (cameraY > 0)
    {
        cameraY = 0;
    }
}

// =====================
// 렌더링
// =====================
void Render()
{
    int i;
    int count;

    // 화면 맨 위로 이동
    printf("\x1b[H");

    // 배경 다시 그리기
    for (i = 0; i < HEIGHT; i++)
    {
        move_cursor(1, i + 1);

        printf("                                                                                                    ");
    }

    // 플랫폼
    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        int drawY =
            platforms[i].y
            - cameraY;

        int j;

        if (drawY >= 0 && drawY < HEIGHT)
        {
            move_cursor(
                platforms[i].x,
                drawY
            );

            for (j = 0; j < platforms[i].width; j++)
            {
                printf("=");
            }
        }
    }

    // 플레이어 수
    count = isMulti ? 2 : 1;

    //////////////////////////////// 궤적
    if (isCharging)
    {
        float power;
        float vx;
        float vy;

        float simX;
        float simY;

        int k;

        power =
            (float)(
                GetTickCount64()
                - chargeStart
                )
            / MAX_GAUGE;

        if (power > 1)
        {
            power = 1;
        }

        power = power * power;

        // 실제 점프와 완전히 동일
        vx =
            power
            * 2.8f
            * moveDir[currentPlayer];

        vy =
            -(
                power
                * 13.5f
                );

        simX =
            players[currentPlayer].x;

        simY =
            players[currentPlayer].y;

        for (k = 0; k < 60; k++)
        {
            int ix;
            int iy;

            vy += 0.15f;

            simX += vx;
            simY += vy;

            // 발판 충돌 시 종료
            if (TrailHitPlatform(simX, simY + 3))
            {
                break;
            }

            ix = (int)simX;
            iy = (int)(simY - cameraY);

            if (iy >= 0 && iy < HEIGHT)
            {
                move_cursor(ix, iy);
                printf(".");
            }
        }
    }

    // 캐릭터 출력
    for (i = 0; i < count; i++)
    {
        int drawY =
            (int)players[i].y
            - cameraY;

        DrawCharacter(
            (int)players[i].x,
            drawY,
            i
        );
    }

    // UI
    move_cursor(1, 1);

    if (isMulti)
    {
        printf("멀티 모드");
    }
    else
    {
        printf("싱글 모드");
    }

    move_cursor(1, 2);

    printf("높이 : %d", -cameraY);

    move_cursor(1, 3);

    printf("ESC : 메뉴로 돌아가기");

    fflush(stdout);
}

// =====================
// 게임 실행
// =====================
void RunGame(bool multi)
{
    InitGame(multi);

    while (1)
    {
        Input();

        Update();

        Render();

        Sleep(16);

        // ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 1)
        {
            break;
        }
    }

    clear_screen();
}

// =====================
// 플레이 방법
// =====================
void ShowHowToPlay()
{
    clear_screen();

    set_font_color(96);

    move_cursor(40, 6);
    printf("게임 방법");

    printf(COLOR_RESET);

    move_cursor(30, 8);
    printf("====================");

    move_cursor(30, 10);
    printf("[싱글 플레이]");

    move_cursor(30, 11);
    printf("A / D : 좌우 이동");

    move_cursor(30, 12);
    printf("SPACE : 점프 차징");

    move_cursor(30, 14);
    printf("[멀티 플레이]");

    move_cursor(30, 15);
    printf("P1 : A / D / W");

    move_cursor(30, 16);
    printf("P2 : J / L / I");

    move_cursor(30, 18);
    printf("TIP");

    move_cursor(30, 19);
    printf("- 점프킹 스타일로 위로 계속 올라갑니다");

    move_cursor(30, 20);
    printf("- 발판 위에 정확히 착지해야 합니다");

    move_cursor(30, 21);
    printf("- 차징 세기에 따라 높이가 달라집니다");

    move_cursor(30, 28);
    printf("ESC : 메뉴로 돌아가기");

    printf(COLOR_RESET);

    (void)_getch();
}

// =====================
// 크레딧
// =====================
void ShowCredit()
{
    clear_screen();

    set_font_color(96);

    move_cursor(38, 10);
    printf("KMP STUDIO");

    printf(COLOR_RESET);

    move_cursor(35, 14);
    printf("Kim: 김승주");

    move_cursor(35, 15);
    printf("Moon: 문용성");

    move_cursor(35, 16);
    printf("Park: 박정원");

    move_cursor(28, 24);
    printf("Press any key to return");

    (void)_getch();
}

// =====================
// 메뉴
// =====================
void DrawMenu(int menu)
{
    clear_screen();

    set_font_color(96);

    move_cursor(32, 6);
    printf("◆==== ASCII JUMP GAME ====◆");

    printf(COLOR_RESET);

    move_cursor(38, 10);

    if (menu == 0)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" > 싱글 게임 ");
    }
    else
    {
        printf(" 싱글 게임 ");
    }

    printf(COLOR_RESET);

    move_cursor(38, 12);

    if (menu == 1)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" > 멀티 게임 ");
    }
    else
    {
        printf(" 멀티 게임 ");
    }

    printf(COLOR_RESET);

    move_cursor(38, 14);

    if (menu == 2)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" > 플레이 방법 ");
    }
    else
    {
        printf(" 플레이 방법 ");
    }

    printf(COLOR_RESET);

    move_cursor(38, 16);

    if (menu == 3)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" > 크레딧 ");
    }
    else
    {
        printf(" 크레딧 ");
    }

    printf(COLOR_RESET);

    move_cursor(38, 18);

    if (menu == 4)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" > 나가기");
    }
    else
    {
        printf(" 나가기 ");
    }

    printf(COLOR_RESET);
}

// =====================
// 메인
// =====================
int main()
{
    int menu = 0;
    char input;

    hide_cursor();

    ShowLogo();

    while (1)
    {
        DrawMenu(menu);

        input = _getch();

        if (input == 27)
        {
            break;
        }

        if (input == 'w' || input == 'W')
        {
            if (menu > 0)
            {
                menu--;
            }
        }

        if (input == 's' || input == 'S')
        {
            if (menu < 4)
            {
                menu++;
            }
        }

        if (input == ' ')
        {
            if (menu == 0)
            {
                RunGame(false);
            }
            else if (menu == 1)
            {
                RunGame(true);
            }
            else if (menu == 2)
            {
                ShowHowToPlay();
            }
            else if (menu == 3)
            {
                ShowCredit();
            }
            else if (menu == 4)
            {
                break;
            }
        }
    }

    show_cursor();

    clear_screen();

    move_cursor(40, 12);

    printf("GAME ENDED");

    return 0;
}