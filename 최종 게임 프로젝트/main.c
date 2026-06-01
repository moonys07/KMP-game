#define _CRT_SECURE_NO_WARNINGS

// 표준 입출력 함수 사용 (printf 등)
#include <stdio.h>

// 동적 메모리 관련 함수 사용
#include <stdlib.h>

// _getch() 함수 사용
#include <conio.h>

// Windows API 사용 (Sleep, GetAsyncKeyState 등)
#include <Windows.h>

// bool 자료형 사용
#include <stdbool.h>

// =====================================================
// KMP STUDIO
// ASCII JUMP GAME - JUMP KING VERSION
// =====================================================

// =====================
// 게임 화면 설정
// =====================

// 콘솔 화면 가로 크기
#define WIDTH 100

// 콘솔 화면 세로 크기
#define HEIGHT 30

// 점프 차징 최대 시간(ms)
#define MAX_GAUGE 2500

// 궤적 표시 최대 길이
#define TRAIL_MAX 200

// 플랫폼 개수
#define PLATFORM_COUNT 25

// =====================
// ANSI 색상 코드
// =====================

// 기본 색상으로 복구
#define COLOR_RESET "\x1b[0m"

// =====================
// 플레이어 구조체
// =====================

// 플레이어의 위치, 속도, 상태를
// 하나의 자료형으로 관리하기 위한 구조체
typedef struct
{
    // 플레이어 현재 위치
    float x;
    float y;

    // 플레이어 이동 속도
    float vx;
    float vy;

    // 점프 중인지 여부
    bool isJumping;

} Player;

// =====================
// 플랫폼 구조체
// =====================

// 플랫폼의 위치와 크기를 저장하는 구조체
typedef struct
{
    // 플랫폼 시작 좌표
    int x;
    int y;

    // 플랫폼 길이
    int width;

} Platform;

// =====================
// 전역 변수
// =====================

// 플레이어 정보를 저장하는 배열
// 싱글 : players[0]
// 멀티 : players[0], players[1]
Player players[2];

// 현재 점프 차징 중인지 저장
bool isCharging = false;

// 멀티 플레이 여부 저장
bool isMulti = false;

// 점프 차징 시작 시간 저장
ULONGLONG chargeStart = 0;

// 현재 차징 파워 저장
float currentPower = 0;

// 현재 조작 중인 플레이어 번호
int currentPlayer = 0;

// 플레이어가 바라보는 방향
// 1 = 오른쪽
// -1 = 왼쪽
int moveDir[2] = { 1,1 };

// =====================
// 카메라 변수
// =====================

// 화면이 따라갈 Y 좌표
int cameraY = 0;

// =====================
// 플랫폼 맵 데이터
// =====================

// 배열을 이용하여 모든 발판의
// 위치와 길이를 저장한다.
Platform platforms[PLATFORM_COUNT] =
{
    // 시작 바닥
    {0, 27, WIDTH},

    // 점프킹 스타일 발판 배치
    // x좌표, y좌표, 길이

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

    // 최종 목표 지점
    {50, -65, 15}
};

// =====================
// ANSI 함수
// =====================

// 커서를 특정 좌표(x,y)로 이동시키는 함수
// 콘솔에서 원하는 위치에 그림이나 글자를 출력할 때 사용
void move_cursor(int x, int y)
{
    printf("\x1b[%d;%dH", y, x);
}

// 글자 색상을 변경하는 함수
// ANSI 색상 코드를 이용하여 출력 색상을 설정
void set_font_color(int code)
{
    printf("\x1b[%dm", code);
}

// 배경 색상을 변경하는 함수
void set_bg_color(int code)
{
    printf("\x1b[%dm", code);
}

// 콘솔 커서를 숨기는 함수
// 게임 플레이 시 깜빡이는 커서를 보이지 않게 함
void hide_cursor()
{
    printf("\x1b[?25l");
}

// 콘솔 커서를 다시 표시하는 함수
void show_cursor()
{
    printf("\x1b[?25h");
}

// 콘솔 화면 전체를 지우는 함수
void clear_screen()
{
    printf("\x1b[2J");
}

// =====================
// 로고 출력
// =====================

// 게임 시작 시 KMP STUDIO 로고를 출력하는 함수
// ASCII 아트를 이용하여 제작하였다.
void ShowLogo()
{
    // 화면 초기화
    clear_screen();

    // 밝은 하늘색 적용
    set_font_color(96);

    // 지정된 좌표에 로고 출력
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

    // 2초 동안 로고 표시
    Sleep(2000);

    // 로고 화면 제거
    clear_screen();
}

// =====================
// 캐릭터 출력
// =====================

// 플레이어의 상태에 따라
// 다른 모습의 캐릭터를 출력하는 함수
void DrawCharacter(int x, int y, int index)
{
    // 현재 플레이어를 가리키는 포인터
    Player* p = &players[index];

    // 화면 밖이면 출력하지 않음
    if (y < -2 || y > HEIGHT)
    {
        return;
    }

    // 점프 차징 중인 모습
    if (isCharging && currentPlayer == index)
    {
        move_cursor(x, y);
        printf(" O ");

        move_cursor(x, y + 1);
        printf("└|┘");

        move_cursor(x, y + 2);
        printf("/ \\");
    }

    // 점프 중인 모습
    else if (p->isJumping)
    {
        move_cursor(x, y);
        printf("\\O/");

        move_cursor(x, y + 1);
        printf(" |");

        move_cursor(x, y + 2);
        printf("\\ /");
    }

    // 기본 대기 모습
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
// 플랫폼 충돌 판정
// =====================

// 플레이어가 플랫폼 위에 착지했는지 검사하는 함수
// 착지 성공 시 true 반환
bool CheckPlatform(Player* p, float prevY)
{
    int i;

    // 모든 플랫폼을 검사
    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        // 현재 플랫폼을 가리키는 포인터
        Platform* pf = &platforms[i];

        // X축 범위 충돌 검사
        if (
            p->x + 1 >= pf->x &&
            p->x <= pf->x + pf->width
            )
        {
            // Y축 범위 충돌 검사
            // 이전 위치와 현재 위치를 비교하여
            // 플랫폼을 통과하지 않도록 처리
            if (
                prevY + 3 <= pf->y &&
                p->y + 3 >= pf->y
                )
            {
                // 플레이어를 플랫폼 위에 올려놓음
                p->y = pf->y - 3;

                return true;
            }
        }
    }

    // 충돌하지 않은 경우
    return false;
}

// =====================
// 궤적 충돌 판정
// =====================

// 점프 예상 경로가 플랫폼과 만나는지 검사
// 궤적 표시를 중간에 멈추기 위해 사용
bool TrailHitPlatform(float x, float y)
{
    int i;

    // 모든 플랫폼 검사
    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        Platform* pf = &platforms[i];

        // X축 충돌 검사
        if (
            x >= pf->x &&
            x <= pf->x + pf->width
            )
        {
            // Y축 충돌 검사
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

// 게임 시작 시 필요한 변수들을
// 기본 상태로 설정하는 함수
void InitGame(bool multi)
{
    // 싱글/멀티 모드 저장
    isMulti = multi;

    // 첫 번째 플레이어부터 시작
    currentPlayer = 0;

    // 점프 상태 초기화
    isCharging = false;

    // 차징 파워 초기화
    currentPower = 0;

    // 카메라 위치 초기화
    cameraY = 0;

    // =====================
    // 플레이어 1 초기화
    // =====================

    // 시작 위치 설정
    players[0].x = 10;
    players[0].y = 24;

    // 속도 초기화
    players[0].vx = 0;
    players[0].vy = 0;

    // 점프 상태 초기화
    players[0].isJumping = false;

    // =====================
    // 플레이어 2 초기화
    // =====================

    players[1].x = 60;
    players[1].y = 24;

    players[1].vx = 0;
    players[1].vy = 0;

    players[1].isJumping = false;

    // 화면 초기화
    clear_screen();
}

// =====================
// 입력
// =====================
void Input()
{
    // 현재 턴인 플레이어를 가리키는 포인터
    // players[currentPlayer]를 간단하게 사용하기 위해 저장
    Player* p = &players[currentPlayer];

    // 플레이어별 입력 키 저장용 변수
    int leftKey;
    int rightKey;
    int jumpKey;

    // -------------------------
    // 싱글 플레이 키 설정
    // -------------------------
    if (!isMulti)
    {
        leftKey = 'A';        // 왼쪽 이동
        rightKey = 'D';       // 오른쪽 이동
        jumpKey = VK_SPACE;   // 점프 차징
    }
    else
    {
        // -------------------------
        // 멀티 플레이 키 설정
        // -------------------------

        // Player1
        if (currentPlayer == 0)
        {
            leftKey = 'A';
            rightKey = 'D';
            jumpKey = 'W';
        }
        else
        {
            // Player2
            leftKey = VK_LEFT;
            rightKey = VK_RIGHT;
            jumpKey = VK_UP;
        }
    }

    // =================================================
    // 왼쪽 이동 입력
    // =================================================

    // 현재 왼쪽 키가 눌려있는지 검사
    if (GetAsyncKeyState(leftKey) & 0x8000)
    {
        // 바라보는 방향을 왼쪽으로 변경
        moveDir[currentPlayer] = -1;

        // 차징 중이 아니고 점프 중도 아닐 때만 이동
        if (!isCharging && !p->isJumping)
        {
            // x좌표 감소
            p->x -= 1.0f;
        }
    }

    // =================================================
    // 오른쪽 이동 입력
    // =================================================

    if (GetAsyncKeyState(rightKey) & 0x8000)
    {
        // 바라보는 방향을 오른쪽으로 변경
        moveDir[currentPlayer] = 1;

        if (!isCharging && !p->isJumping)
        {
            // x좌표 증가
            p->x += 1.0f;
        }
    }

    // =================================================
    // 점프 차징 시작
    // =================================================

    // 점프 키를 누르고 있는 동안
    if (GetAsyncKeyState(jumpKey) & 0x8000)
    {
        // 아직 차징 시작 안했고 공중도 아닐 때
        if (!isCharging && !p->isJumping)
        {
            // 차징 상태 진입
            isCharging = true;

            // 현재 시간을 저장
            // 나중에 얼마나 오래 눌렀는지 계산
            chargeStart = GetTickCount64();
        }
    }
    else
    {
        // =================================================
        // 점프 키를 떼는 순간
        // =================================================

        if (isCharging)
        {
            // 눌렀던 시간 계산
            ULONGLONG t =
                GetTickCount64()
                - chargeStart;

            // 최대 차징시간 초과 방지
            if (t > MAX_GAUGE)
            {
                t = MAX_GAUGE;
            }

            // 0 ~ 1 사이 비율로 변환
            currentPower =
                (float)t / MAX_GAUGE;

            // 제곱 처리
            // 짧게 누르면 약하게
            // 길게 누르면 훨씬 강하게 점프
            currentPower =
                currentPower * currentPower;

            // =================================================
            // 점프 물리값 계산
            // =================================================

            // 수평 속도
            // moveDir이 -1이면 왼쪽
            // moveDir이 1이면 오른쪽
            p->vx =
                currentPower
                * 2.8f
                * moveDir[currentPlayer];

            // 수직 속도
            // 음수 방향이 위쪽
            p->vy =
                -(
                    currentPower
                    * 13.5f
                    );

            // 공중 상태 진입
            p->isJumping = true;

            // 차징 종료
            isCharging = false;
        }
    }
}

// =====================
// 업데이트
// =====================
void Update()
{
    // 현재 플레이어를 가리키는 포인터
    Player* p = &players[currentPlayer];

    // =================================================
    // 점프 중일 때만 물리 계산 수행
    // =================================================
    if (p->isJumping)
    {
        // 이전 y좌표 저장
        // 발판 충돌 검사에 사용
        float prevY = p->y;

        // =================================================
        // 중력 적용
        // =================================================

        // 매 프레임마다 아래 방향 속도 증가
        // 숫자가 클수록 더 빨리 떨어짐
        p->vy += 0.15f;

        // =================================================
        // 위치 이동
        // =================================================

        // x축 이동
        p->x += p->vx;

        // y축 이동
        p->y += p->vy;

        // =================================================
        // 발판 충돌 검사
        // =================================================

        // 떨어지는 중일 때만 착지 가능
        if (p->vy > 0)
        {
            // 발판에 닿았는지 검사
            if (CheckPlatform(p, prevY))
            {
                // 수직 속도 제거
                p->vy = 0;

                // 수평 속도 제거
                p->vx = 0;

                // 점프 종료
                p->isJumping = false;

                // =================================================
                // 멀티 플레이 턴 교체
                // =================================================

                if (isMulti)
                {
                    // 0 -> 1
                    // 1 -> 0
                    currentPlayer =
                        (currentPlayer + 1) % 2;
                }
            }
        }
    }

    // =================================================
    // 맵 좌측 경계
    // =================================================

    if (p->x < 0)
    {
        p->x = 0;
    }

    // =================================================
    // 맵 우측 경계
    // =================================================

    if (p->x > WIDTH - 4)
    {
        p->x = WIDTH - 4;
    }

    // =================================================
    // 카메라 위치 계산
    // =================================================

    // 플레이어를 화면 중앙 근처에 유지
    cameraY =
        (int)p->y - 12;

    // =================================================
    // 바닥 아래는 카메라가 내려가지 않음
    // =================================================

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

    // 화면 커서를 맨 위로 이동
    // 화면 전체를 다시 그리는 방식
    printf("\x1b[H");

    // =================================================
    // 화면 초기화
    // =================================================

    // 이전 프레임의 그림을 지우기 위해
    // 화면 전체를 공백으로 채움
    for (i = 0; i < HEIGHT; i++)
    {
        move_cursor(1, i + 1);

        printf("                                                                                                    ");
    }

    // =================================================
    // 플랫폼 출력
    // =================================================

    for (i = 0; i < PLATFORM_COUNT; i++)
    {
        // 카메라 위치를 적용한 화면상의 y좌표
        int drawY =
            platforms[i].y
            - cameraY;

        int j;

        // 화면 안에 있을 때만 그림
        if (drawY >= 0 && drawY < HEIGHT)
        {
            move_cursor(
                platforms[i].x,
                drawY
            );

            // 발판 길이만큼 '=' 출력
            for (j = 0; j < platforms[i].width; j++)
            {
                printf("=");
            }
        }
    }

    // =================================================
    // 플레이어 수 결정
    // =================================================

    count = isMulti ? 2 : 1;

    // =================================================
    // 궤적 예측 시스템
    // =================================================

    if (isCharging)
    {
        float power;
        float vx;
        float vy;

        float simX;
        float simY;

        int k;

        // -------------------------
        // 현재 차징 비율 계산
        // -------------------------

        power =
            (float)(
                GetTickCount64()
                - chargeStart
                )
            / MAX_GAUGE;

        // 최대값 제한
        if (power > 1)
        {
            power = 1;
        }

        // 실제 점프와 동일하게 제곱 적용
        power = power * power;

        // -------------------------
        // 점프 예상 속도 계산
        // -------------------------

        vx =
            power
            * 2.8f
            * moveDir[currentPlayer];

        vy =
            -(
                power
                * 13.5f
                );

        // -------------------------
        // 현재 위치 복사
        // -------------------------

        simX =
            players[currentPlayer].x;

        simY =
            players[currentPlayer].y;

        // =================================================
        // 미래 위치 시뮬레이션
        // =================================================

        for (k = 0; k < 60; k++)
        {
            int ix;
            int iy;

            // 실제 게임 물리와 동일한 중력 적용
            vy += 0.15f;

            // 예상 위치 계산
            simX += vx;
            simY += vy;

            // 발판에 닿으면 궤적 종료
            if (TrailHitPlatform(simX, simY + 3))
            {
                break;
            }

            // 정수 좌표 변환
            ix = (int)simX;
            iy = (int)(simY - cameraY);

            // 화면 내부일 때만 출력
            if (iy >= 0 && iy < HEIGHT)
            {
                move_cursor(ix, iy);

                // 점(.)으로 궤적 표시
                printf(".");
            }
        }
    }

    // =================================================
    // 캐릭터 출력
    // =================================================

    // 플레이어 수만큼 반복
    for (i = 0; i < count; i++)
    {
        // 월드 좌표를 화면 좌표로 변환
        // 카메라가 위로 이동하면 캐릭터도 같이 이동해 보임
        int drawY =
            (int)players[i].y
            - cameraY;

        // 캐릭터 그리기 함수 호출
        DrawCharacter(
            (int)players[i].x, // x좌표
            drawY,             // 화면 y좌표
            i                  // 플레이어 번호
        );
    }

    // =================================================
    // UI 출력
    // =================================================

    // 화면 좌측 상단으로 이동
    move_cursor(1, 1);

    // 현재 게임 모드 표시
    if (isMulti)
    {
        printf("멀티 모드");
    }
    else
    {
        printf("싱글 모드");
    }

    // =================================================
    // 높이 표시
    // =================================================

    // 두 번째 줄로 이동
    move_cursor(1, 2);

    // cameraY는 위로 갈수록 음수가 되므로
    // 음수를 다시 양수로 바꿔서 표시
    printf("높이 : %d", -cameraY);

    // =================================================
    // ESC 안내 문구
    // =================================================

    // 세 번째 줄로 이동
    move_cursor(1, 3);

    printf("ESC : 메뉴로 돌아가기");

    // =================================================
    // 출력 강제 갱신
    // =================================================

    // 버퍼에 있는 내용을 즉시 화면에 출력
    fflush(stdout);
}

// =====================
// 게임 실행
// =====================
void RunGame(bool multi)
{
    // 게임 시작 시 플레이어, 카메라, 변수 초기화
    InitGame(multi);

    // 게임이 종료될 때까지 무한 반복
    while (1)
    {
        // ==========================
        // 1. 입력 처리
        // ==========================
        Input();

        // ==========================
        // 2. 게임 상태 업데이트
        // ==========================
        Update();

        // ==========================
        // 3. 화면 출력
        // ==========================
        Render();

        // ==========================
        // 프레임 속도 조절
        // ==========================
        // 약 16ms 대기
        // 1000 ÷ 16 ≒ 60FPS
        Sleep(16);

        // ==========================
        // ESC 키 입력 시 종료
        // ==========================
        if (GetAsyncKeyState(VK_ESCAPE) & 1)
        {
            break;
        }
    }

    // 게임 종료 후 화면 정리
    clear_screen();
}

// =====================
// 플레이 방법
// =====================
void ShowHowToPlay()
{
    // 기존 화면 제거
    clear_screen();

    // 글자색을 밝은 하늘색으로 변경
    set_font_color(96);

    // 제목 출력
    move_cursor(45, 6);
    printf("게임 방법");

    // 색상 초기화
    printf(COLOR_RESET);

    // 구분선 출력
    move_cursor(30, 8);
    printf("◆=========================================◆");

    // -------------------------
    // 싱글 플레이 설명
    // -------------------------
    move_cursor(30, 10);
    printf("[싱글 플레이]");

    move_cursor(30, 11);
    printf("A / D : 좌우 이동");

    move_cursor(30, 12);
    printf("SPACE : 점프 차징");

    // -------------------------
    // 멀티 플레이 설명
    // -------------------------
    move_cursor(30, 14);
    printf("[멀티 플레이]");

    move_cursor(30, 15);
    printf("P1 : A / D / W");

    move_cursor(30, 16);
    printf("P2 : J / L / I");

    // -------------------------
    // 게임 팁 출력
    // -------------------------
    move_cursor(30, 18);
    printf("TIP");

    move_cursor(30, 19);
    printf("- 점프킹 스타일로 위로 계속 올라갑니다");

    move_cursor(30, 20);
    printf("- 발판 위에 정확히 착지해야 합니다");

    move_cursor(30, 21);
    printf("- 차징 세기에 따라 높이가 달라집니다");

    // 하단 안내 문구
    move_cursor(30, 28);
    printf("ESC : 메뉴로 돌아가기");

    // 색상 초기화
    printf(COLOR_RESET);

    // 아무 키나 누를 때까지 대기
    (void)_getch();
}

// =====================
// 크레딧 화면
// =====================
void ShowCredit()
{
    // 화면을 지워 이전 화면 내용 제거
    clear_screen();

    // 제목 색상을 밝은 하늘색으로 설정
    set_font_color(96);

    // 지정 좌표에 프로젝트 이름 출력
    move_cursor(38, 10);
    printf("KMP STUDIO");

    // 색상 초기화
    printf(COLOR_RESET);

    // 팀원 이름 출력
    move_cursor(35, 14);
    printf("Kim: 김승주");

    move_cursor(35, 15);
    printf("Moon: 문용성");

    move_cursor(35, 16);
    printf("Park: 박정원");

    // 사용자에게 아무 키나 누르면 돌아간다는 안내 출력
    move_cursor(28, 24);
    printf("Press any key to return");

    // 키 입력을 받을 때까지 대기
    (void)_getch();
}

// =====================
// 메뉴 출력
// =====================
void DrawMenu(int menu)
{
    // 메뉴를 다시 그리기 위해 화면 전체 초기화
    clear_screen();

    // 메뉴 제목 색상 설정 (하늘색)
    set_font_color(96);

    // 제목 출력 위치 지정
    move_cursor(32, 6);
    printf("◆==== ASCII JUMP GAME ====◆");

    // 색상 초기화
    printf(COLOR_RESET);

    // -----------------
    // 싱글 게임 메뉴
    // -----------------
    move_cursor(38, 10);

    // 현재 선택된 메뉴라면 배경색을 넣어 강조
    if (menu == 0)
    {
        set_bg_color(43);      // 노란 배경
        set_font_color(30);    // 검정 글씨
        printf(" ▶ 싱글 게임 ");
    }
    else
    {
        printf(" 싱글 게임 ");
    }

    printf(COLOR_RESET);

    // -----------------
    // 멀티 게임 메뉴
    // -----------------
    move_cursor(38, 12);

    if (menu == 1)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" ▶ 멀티 게임 ");
    }
    else
    {
        printf(" 멀티 게임 ");
    }

    printf(COLOR_RESET);

    // -----------------
    // 플레이 방법 메뉴
    // -----------------
    move_cursor(38, 14);

    if (menu == 2)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" ▶ 플레이 방법 ");
    }
    else
    {
        printf(" 플레이 방법 ");
    }

    printf(COLOR_RESET);

    // -----------------
    // 크레딧 메뉴
    // -----------------
    move_cursor(38, 16);

    if (menu == 3)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" ▶ 크레딧 ");
    }
    else
    {
        printf(" 크레딧 ");
    }

    printf(COLOR_RESET);

    // -----------------
    // 나가기 메뉴
    // -----------------
    move_cursor(38, 18);

    if (menu == 4)
    {
        set_bg_color(43);
        set_font_color(30);
        printf(" ▶ 나가기");
    }
    else
    {
        printf(" 나가기 ");
    }

    // 마지막으로 색상 복구
    printf(COLOR_RESET);
}

// =====================
// 메인 함수
// =====================
int main()
{
    // 현재 선택된 메뉴 번호 저장
    int menu = 0;

    // 사용자가 입력한 키 저장
    char input;

    // 커서를 숨겨 화면 깜빡임 감소
    hide_cursor();

    // 게임 시작 시 로고 출력
    ShowLogo();

    // 프로그램이 종료될 때까지 반복
    while (1)
    {
        // 현재 메뉴 상태를 화면에 출력
        DrawMenu(menu);

        // 키보드 입력 대기
        input = _getch();

        // ESC 입력 시 프로그램 종료
        if (input == 27)
        {
            break;
        }

        // W 키를 누르면 메뉴 위로 이동
        if (input == 'w' || input == 'W')
        {
            // 메뉴의 최상단이 아닐 때만 이동
            if (menu > 0)
            {
                menu--;
            }
        }

        // S 키를 누르면 메뉴 아래로 이동
        if (input == 's' || input == 'S')
        {
            // 메뉴의 최하단이 아닐 때만 이동
            if (menu < 4)
            {
                menu++;
            }
        }

        // 스페이스바를 누르면 현재 선택 메뉴 실행
        if (input == ' ')
        {
            // 싱글 플레이 실행
            if (menu == 0)
            {
                RunGame(false);
            }

            // 멀티 플레이 실행
            else if (menu == 1)
            {
                RunGame(true);
            }

            // 게임 방법 화면 실행
            else if (menu == 2)
            {
                ShowHowToPlay();
            }

            // 크레딧 화면 실행
            else if (menu == 3)
            {
                ShowCredit();
            }

            // 종료 메뉴 선택 시 프로그램 종료
            else if (menu == 4)
            {
                break;
            }
        }
    }

    // 종료 전 커서를 다시 표시
    show_cursor();

    // 화면 정리
    clear_screen();

    // 종료 메시지 출력 위치 설정
    move_cursor(40, 12);

    printf("GAME ENDED");

    // 정상 종료
    return 0;
}//202618606