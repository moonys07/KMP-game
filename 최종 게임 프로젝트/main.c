#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <stdbool.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// =====================================================
// KMP STUDIO - ASCII JUMP GAME (Timer & Balance Updated)
// =====================================================

#define WIDTH 120
#define HEIGHT 30

#define MAX_GAUGE 1000.0f
#define PLATFORM_COUNT 64

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
    int height;
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

// 현재 재생 중인 맵 BGM 구역을 저장하는 변수
// -1 = 아직 맵 BGM이 재생되지 않은 상태
//  0 = 지하 BGM
//  1 = 지상 BGM
//  2 = 하늘 BGM
int currentMapBgmZone = -1;

// [추가] 타이머 및 게임 종료 관련 변수
ULONGLONG playTime[2] = { 0, 0 }; // 각 플레이어의 누적 활성 시간 (ms)
ULONGLONG lastFrameTime = 0;      // 델타 타임 계산용
bool gameFinished = false;        // 최고 존 도달 완료 플래그
int winnerPlayer = -1;            // 승리자 인덱스

int hallX = 10;
int hallY = 24;
int hallWalkTimer = 0;
bool hallIsMoving = false;
int currentNpc = -1;
int activeBubbleNpc = -1;

NPC npcs[3] = {
    { 25, 24, "김승주", "플랫폼 시스템 개발", "점프 물리 구현" },
    { 60, 24, "문용성", "맵 설계 담당", "UI 디자인 담당" },
    { 95, 24, "박정원", "게임 총괄", "크레딧 시스템 제작" }
};

// [밸런스 수정] 머리가 부딪히지 않도록 위아래 간격을 7~9칸으로 여유롭게 재설계한 64개 플랫폼
Platform platforms[PLATFORM_COUNT] = {
    // 지하 영역 (Underground) - Y: 27 ~ -113
    {0, 27, WIDTH, 4}, // 시작 바닥
    {20, 20, 16, 2},
    {55, 12, 16, 2},
    {90, 4, 16, 2},
    {50, -4, 16, 2},
    {15, -12, 16, 2},
    {75, -20, 16, 2},
    {40, -28, 16, 2},
    {10, -36, 16, 2},
    {65, -44, 16, 2},
    {95, -52, 16, 2},
    {55, -60, 16, 2},
    {20, -68, 16, 2},
    {80, -76, 16, 2},
    {45, -84, 16, 2},
    {15, -92, 16, 2},
    {70, -100, 16, 2},
    {35, -108, 16, 2},
    {85, -116, 16, 2},
    {50, -124, 16, 2},
    {15, -132, 16, 2},

    // 지상 영역 (Ground) - Y: -141 ~ -293
    {75, -141, 14, 2},
    {40, -150, 14, 2},
    {10, -159, 14, 2},
    {65, -168, 14, 2},
    {95, -177, 14, 2},
    {50, -186, 14, 2},
    {20, -195, 14, 2},
    {75, -204, 14, 2},
    {40, -213, 14, 2},
    {10, -222, 14, 2},
    {60, -231, 14, 2},
    {90, -240, 14, 2},
    {45, -249, 14, 2},
    {15, -258, 14, 2},
    {70, -267, 14, 2},
    {35, -276, 14, 2},
    {85, -285, 14, 2},
    {50, -294, 14, 2},
    {15, -303, 14, 2},
    {75, -312, 14, 2},
    {40, -321, 14, 2},

    // 하늘 영역 (Sky) - Y: -331 ~ -511
    {10, -331, 12, 2},
    {65, -341, 12, 2},
    {95, -351, 12, 2},
    {55, -361, 12, 2},
    {20, -371, 12, 2},
    {75, -381, 12, 2},
    {40, -391, 12, 2},
    {10, -401, 12, 2},
    {60, -411, 12, 2},
    {90, -421, 12, 2},
    {45, -431, 12, 2},
    {15, -441, 12, 2},
    {70, -451, 12, 2},
    {35, -461, 12, 2},
    // 하늘 영역 후반부 - 최종 플랫폼에 올라가기 쉽게 재배치
    {85, -471, 12, 2},
    {55, -481, 12, 2},
    {25, -491, 12, 2},
    {60, -501, 14, 2},
    {88, -511, 14, 2},
    {70, -521, 14, 2},

    // 최종 플랫폼 바로 아래 발판
    // 최종 플랫폼 바로 밑이 아니라 오른쪽 옆에 배치해서
    // 점프할 때 머리가 최종 플랫폼 밑면에 부딪히지 않게 함
    {88, -531, 14, 2},

    // 최종 도착 플랫폼
    // 코드에서 i == PLATFORM_COUNT - 1일 때 승리 처리하기 때문
    {35, -540, 50, 2}
};

char printBuffer[65536];

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
    if (y > -135) return 0;
    if (y > -325) return 1;
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

    // [1번 요구사항 수정] 인덱스 정밀 보정 (-1 추가)을 통해 경계면 색상 불일치 버그 수정
    SetZoneColor(cameraY + screenY - 1);

    if (isMulti) {
        if (playerIndex == 0) set_font_color(92);
        else set_font_color(93);
    }
    else {
        set_font_color(97);
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

bool CheckCollision(float px, float py) {
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        Platform* pf = &platforms[i];
        if (px + 2.0f >= pf->x && px <= pf->x + pf->width - 1) {
            if (py + 2.0f >= pf->y && py <= pf->y + pf->height - 1) {
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

    // 타이머 데이터 초기화
    playTime[0] = 0;
    playTime[1] = 0;
    gameFinished = false;
    winnerPlayer = -1;

    players[0] = (Player){ 10, 24, 0, 0, false, false, 0 };
    players[1] = (Player){ 60, 24, 0, 0, false, false, 0 };
    clear_screen();
    lastFrameTime = GetTickCount64();
}

void Input() {
    Player* p = &players[currentPlayer];
    int leftKey = (!isMulti || currentPlayer == 0) ? 'A' : VK_LEFT;
    int rightKey = (!isMulti || currentPlayer == 0) ? 'D' : VK_RIGHT;
    int jumpKey = (!isMulti) ? VK_SPACE : (currentPlayer == 0 ? 'W' : VK_UP);

    bool movingThisFrame = false;

    if (p->isJumping && isCharging) {
        isCharging = false;
    }

    if (!p->isJumping) {
        if (isCharging) {
            p->vx = 0.0f;
            movingThisFrame = false;

            if (GetAsyncKeyState(leftKey) & 0x8000) moveDir[currentPlayer] = -1;
            if (GetAsyncKeyState(rightKey) & 0x8000) moveDir[currentPlayer] = 1;
        }
        else {
            p->vx = 0.0f;
            if (GetAsyncKeyState(leftKey) & 0x8000) {
                moveDir[currentPlayer] = -1;
                p->vx = -1.0f;
                movingThisFrame = true;
            }
            if (GetAsyncKeyState(rightKey) & 0x8000) {
                moveDir[currentPlayer] = 1;
                p->vx = 1.0f;
                movingThisFrame = true;
            }
        }
    }
    else {
        if (GetAsyncKeyState(leftKey) & 0x8000) moveDir[currentPlayer] = -1;
        if (GetAsyncKeyState(rightKey) & 0x8000) moveDir[currentPlayer] = 1;
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

// =====================
// 맵별 배경음악 함수
// =====================

// 현재 재생 중인 맵 BGM을 정지하고 닫는 함수
void StopMapBGM() {
    // mapbgm이라는 별칭으로 재생 중인 음악 정지
    mciSendStringA("stop mapbgm", NULL, 0, NULL);

    // 열려 있는 mapbgm 파일 닫기
    mciSendStringA("close mapbgm", NULL, 0, NULL);
}

// 특정 파일을 맵 BGM으로 반복 재생하는 함수
void StartMapBGM(const char* filename) {
    char command[256];
    MCIERROR err;
    char errorText[256];

    // 기존 맵 BGM이 재생 중이면 먼저 정지
    StopMapBGM();

    // mp3 파일을 mapbgm이라는 별칭으로 열기
    // mp3는 type mpegvideo를 사용하는 것이 안정적임
    sprintf(command, "open \"%s\" type mpegvideo alias mapbgm", filename);
    err = mciSendStringA(command, NULL, 0, NULL);

    // BGM 파일 열기에 실패했을 때 오류창 출력
    if (err != 0) {
        mciGetErrorStringA(err, errorText, sizeof(errorText));
        MessageBoxA(NULL, errorText, "Map BGM open 실패", MB_OK);
        return;
    }

    // 맵 BGM 볼륨 설정
    // 숫자를 낮출수록 소리가 작아짐
    mciSendStringA("setaudio mapbgm volume to 300", NULL, 0, NULL);

    // mapbgm을 반복 재생
    // repeat 옵션 때문에 노래가 끝나도 다시 처음부터 재생됨
    err = mciSendStringA("play mapbgm repeat", NULL, 0, NULL);

    // BGM 재생에 실패했을 때 오류창 출력
    if (err != 0) {
        mciGetErrorStringA(err, errorText, sizeof(errorText));
        MessageBoxA(NULL, errorText, "Map BGM play 실패", MB_OK);
        return;
    }
}

// 현재 구역에 맞게 BGM을 변경하는 함수
void ChangeMapBGM(int zone) {
    // 이미 같은 구역의 BGM이 재생 중이면 다시 재생하지 않음
    if (currentMapBgmZone == zone) return;

    // 현재 BGM 구역 정보 갱신
    currentMapBgmZone = zone;

    // 구역 번호에 따라 다른 BGM 재생
    if (zone == 0) {
        StartMapBGM("under.mp3");   // 지하 구역 BGM
    }
    else if (zone == 1) {
        StartMapBGM("Park.mp3");    // 지상 구역 BGM
    }
    else if (zone == 2) {
        StartMapBGM("sky.mp3");     // 하늘 구역 BGM
    }
}

void Update() {
    // 델타 타임을 구하여 현재 차례인 플레이어에게만 시간 누적 (상대방 타이머 일시정지 효과)
    ULONGLONG now = GetTickCount64();
    ULONGLONG dt = now - lastFrameTime;
    lastFrameTime = now;

    if (!gameFinished) {
        playTime[currentPlayer] += dt;
    }

    Player* p = &players[currentPlayer];
    float prevX = p->x;
    float prevY = p->y;

    p->x += p->vx;

    if (p->x < 1.0f) { p->x = 1.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }
    if (p->x > WIDTH - 3.0f) { p->x = WIDTH - 3.0f; if (p->isJumping) p->vx = -p->vx * 0.5f; }

    if (CheckCollision(p->x, prevY)) {
        p->x = prevX;
        if (p->isJumping) p->vx = -p->vx * 0.5f;
        else p->vx = 0.0f;
    }

    if (p->isJumping) {
        p->vy += GRAVITY;
        p->y += p->vy;

        bool landed = false;

        if (p->vy > 0) {
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                Platform* pf = &platforms[i];
                if (p->x + 2.0f >= pf->x && p->x <= pf->x + pf->width - 1) {
                    if (prevY + 2.0f <= pf->y && p->y + 2.0f >= pf->y) {
                        p->y = (float)(pf->y - 3);
                        p->vy = 0; p->vx = 0; p->isJumping = false;
                        landed = true;

                        // [추가] 최상단 플랫폼 착지 확인 시 게임 종료 처리
                        if (i == PLATFORM_COUNT - 1) {
                            gameFinished = true;
                            winnerPlayer = currentPlayer;
                            PlaySound(TEXT("land.wav"), NULL, SND_ASYNC | SND_FILENAME);
                            break;
                        }

                        if (isMulti) currentPlayer = (currentPlayer + 1) % 2;

                        PlaySound(TEXT("land.wav"), NULL, SND_ASYNC | SND_FILENAME);
                        break;
                    }
                }
            }
        }
        else if (p->vy < 0) {
            for (int i = 0; i < PLATFORM_COUNT; i++) {
                Platform* pf = &platforms[i];
                if (p->x + 2.0f >= pf->x && p->x <= pf->x + pf->width - 1) {
                    float bottom = pf->y + pf->height - 1;
                    if (prevY >= bottom && p->y <= bottom) {
                        p->y = bottom + 1.0f;
                        p->vy = 0.5f;
                        p->vx *= 0.5f;
                        break;
                    }
                }
            }
        }

        if (!landed && CheckCollision(p->x, p->y)) {
            p->y = prevY;
            p->vy = 0;
        }
    }
    else {
        bool onGround = false;
        float checkY = p->y + 3.0f;

        for (int i = 0; i < PLATFORM_COUNT; i++) {
            Platform* pf = &platforms[i];
            if (p->x + 2.0f >= pf->x && p->x <= pf->x + pf->width - 1) {
                if (checkY >= pf->y && checkY <= pf->y + pf->height - 1) {
                    onGround = true;
                    break;
                }
            }
        }

        if (!onGround) {
            p->isJumping = true;
            p->vy = 0.0f;
        }
    }

    if (p->isMoving) p->walkTimer++; else p->walkTimer = 0;

    cameraY = (int)p->y - 12;
    if (cameraY > 0) cameraY = 0;

    // 플레이어의 현재 y좌표를 기준으로 현재 구역 계산
    // GetZone 기준:
    // 0 = 지하, 1 = 지상, 2 = 하늘
    int newZone = GetZone(p->y);

    // 이전 구역과 현재 구역이 다르면 구역이 바뀐 것
    if (newZone != currentZone) {
        // 현재 구역 정보 갱신
        currentZone = newZone;

        // 화면 중앙에 "지하", "지상", "하늘" 알림을 띄우는 타이머
        areaNotiTimer = 120;

        // 구역이 바뀌었으므로 해당 구역의 BGM으로 변경
        ChangeMapBGM(newZone);
    }
}

// =====================
// 포물선 색상 함수
// =====================

// 현재 월드 Y좌표가 어떤 구역인지 확인해서 포물선 색상을 다르게 설정
void SetTrajectoryColor(float worldY) {
    int zone = GetZone(worldY);

    if (zone == 0) {
        set_font_color(97); // 지하: 흰색
    }
    else if (zone == 1) {
        set_font_color(93); // 지상: 노란색
    }
    else {
        set_font_color(91); // 하늘: 빨간색
    }
}

// 구역에 따라 포물선 모양을 다르게 출력
char GetTrajectoryChar(float worldY) {
    int zone = GetZone(worldY);

    if (zone == 2) {
        return '*'; // 하늘맵에서는 별표
    }

    return '*'; // 지하, 지상도 잘 보이게 별표
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
        int zone = GetZone((float)platforms[i].y);

        for (int h = 0; h < platforms[i].height; h++) {
            int drawY = platforms[i].y + h - cameraY;
            if (drawY >= 0 && drawY < HEIGHT) {
                move_cursor(platforms[i].x, drawY);

                if (zone == 0) set_bg_color(100);
                else if (zone == 1) {
                    if (h == 0) set_bg_color(42);
                    else set_bg_brown();
                }
                else { set_bg_color(107); set_font_color(30); }

                if (zone == 2) {
                    printf("("); for (int j = 1; j < platforms[i].width - 1; j++) printf(" "); printf(")");
                }
                else {
                    for (int j = 0; j < platforms[i].width; j++) printf(" ");
                }
            }
        }
    }

    if (isCharging && !gameFinished) {
        float ratio = (float)(GetTickCount64() - chargeStart) / MAX_GAUGE;
        if (ratio > 1.0f) ratio = 1.0f;

        float simVx = ratio * MAX_JUMP_VX * moveDir[currentPlayer];
        float simVy = -(ratio * MAX_JUMP_VY + 0.8f);

        float simX = players[currentPlayer].x;
        float simY = players[currentPlayer].y;

        for (int k = 0; k < 60; k++) {
            float prevSimX = simX;
            float prevSimY = simY;
            simVy += GRAVITY;

            simX += simVx;
            if (simX < 1.0f) { simX = 1.0f; simVx = -simVx * 0.5f; }
            if (simX > WIDTH - 3.0f) { simX = WIDTH - 3.0f; simVx = -simVx * 0.5f; }

            for (int i = 0; i < PLATFORM_COUNT; i++) {
                Platform* pf = &platforms[i];
                if (simX + 2.0f >= pf->x && simX <= pf->x + pf->width - 1) {
                    if (prevSimY + 2.0f >= pf->y && prevSimY <= pf->y + pf->height - 1) {
                        simX = prevSimX;
                        simVx = -simVx * 0.5f;
                        break;
                    }
                }
            }

            simY += simVy;
            bool hitFloor = false;

            if (simVy > 0) {
                for (int i = 0; i < PLATFORM_COUNT; i++) {
                    Platform* pf = &platforms[i];
                    if (simX + 2.0f >= pf->x && simX <= pf->x + pf->width - 1) {
                        if (prevSimY + 2.0f <= pf->y && simY + 2.0f >= pf->y) {
                            simY = pf->y - 3.0f;
                            hitFloor = true;
                            break;
                        }
                    }
                }
            }
            else if (simVy < 0) {
                for (int i = 0; i < PLATFORM_COUNT; i++) {
                    Platform* pf = &platforms[i];
                    if (simX + 2.0f >= pf->x && simX <= pf->x + pf->width - 1) {
                        float bottom = pf->y + pf->height - 1;
                        if (prevSimY >= bottom && simY <= bottom) {
                            simY = bottom + 1.0f;
                            simVy = 0.5f;
                            simVx *= 0.5f;
                            break;
                        }
                    }
                }
            }

            if (hitFloor) {
                int ix = (int)simX;
                int iy = (int)simY - cameraY + 2;
                if (iy >= 0 && iy < HEIGHT) {
                    SetZoneColor(cameraY + iy);
                    set_font_color(91);
                    move_cursor(ix + 1, iy); printf("X");
                }
                break;
            }

            int ix = (int)simX;
            int iy = (int)simY - cameraY + 1;
            if (iy >= 0 && iy < HEIGHT && k % 3 == 0) {
                // 포물선의 실제 월드 Y좌표 기준으로 배경색과 글자색 설정
                SetZoneColor((int)simY);
                SetTrajectoryColor(simY);

                move_cursor(ix + 1, iy);
                printf("%c", GetTrajectoryChar(simY));
            }
        }
    }

    // 게임이 끝났을 때는 플레이어를 더 이상 그리지 않음
    // 그래야 승리 팝업 위/주변에 플레이어 머리나 몸이 남지 않음
    if (!gameFinished) {
        int count = isMulti ? 2 : 1;
        for (int i = 0; i < count; i++) {
            int drawY = (int)players[i].y - cameraY;
            DrawCharacter((int)players[i].x, drawY, i);
        }
    }

    // [UI 수정] 실시간 타이머 스코어보드 렌더링
    SetZoneColor(cameraY + 1);
    move_cursor(2, 2);
    set_font_color(97);
    char* zname = (currentZone == 0) ? "지하" : (currentZone == 1) ? "지상" : "하늘";
    int height_m = (27 - (int)players[currentPlayer].y) / 2;

    if (!isMulti) {
        printf("[%s] 현재 높이 : %dm  |  ⏱️ 소요 시간 : %.2f초", zname, height_m, (float)playTime[0] / 1000.0f);
    }
    else {
        printf("[%s] 높이 : %dm  |  P1 ⏱️: %.2f초 %s  |  P2 ⏱️: %.2f초 %s",
            zname, height_m,
            (float)playTime[0] / 1000.0f, (currentPlayer == 0 ? "◀" : "  "),
            (float)playTime[1] / 1000.0f, (currentPlayer == 1 ? "◀" : "  "));
    }

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

    // [추가] 축하 Victory 팝업 연출
    if (gameFinished) {
        int cy = 13;

        move_cursor(35, cy);     printf("┌──────────────────────────────────────────────────┐");
        move_cursor(35, cy + 1); printf("│                 축 하 합 니 다 !                 │");
        move_cursor(35, cy + 2); printf("├──────────────────────────────────────────────────┤");

        if (isMulti) {
            move_cursor(35, cy + 3); printf("│    플레이어 %d 승리!                              │", winnerPlayer + 1);
            move_cursor(35, cy + 4); printf("│    소요 등반 시간: %6.2f초                      │", (float)playTime[winnerPlayer] / 1000.0f);
        }
        else {
            move_cursor(35, cy + 3); printf("│    꼭대기 플랫폼 도달 완료!                      │");
            move_cursor(35, cy + 4); printf("│    최종 기록: %6.2f초                           │", (float)playTime[0] / 1000.0f);
        }

        move_cursor(35, cy + 5); printf("│                                                  │");
        move_cursor(35, cy + 6); printf("│        ESC 키를 누르면 메뉴로 이동합니다.        │");
        move_cursor(35, cy + 7); printf("└──────────────────────────────────────────────────┘");
    }

    printf(COLOR_RESET);
    fflush(stdout);
}

// =====================
// 테스트용 공중부양 치트
// =====================

// C키를 누르고 있는 동안 현재 플레이어를 위로 계속 밀어 올리는 함수
void CheckFlyCheat() {
    Player* p = &players[currentPlayer];

    // C키를 누르고 있으면 공중부양 활성화
    if (GetAsyncKeyState('C') & 0x8000) {

        // 차징 중이었다면 차징 해제
        isCharging = false;

        // 공중 상태로 변경
        p->isJumping = true;

        // 위쪽으로 계속 힘을 줌
        // 값이 작을수록 천천히 상승, 클수록 빠르게 상승
        p->vy = -2.0f;

        // 공중부양 중 좌우 이동도 가능하게 처리
        if (GetAsyncKeyState('A') & 0x8000) {
            p->vx = -1.5f;
            moveDir[currentPlayer] = -1;
        }
        else if (GetAsyncKeyState('D') & 0x8000) {
            p->vx = 1.5f;
            moveDir[currentPlayer] = 1;
        }
        else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            p->vx = -1.5f;
            moveDir[currentPlayer] = -1;
        }
        else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            p->vx = 1.5f;
            moveDir[currentPlayer] = 1;
        }
        else {
            // 아무 방향키도 안 누르면 좌우 이동 멈춤
            p->vx = 0.0f;
        }
    }
}

void RunGame(bool multi) {
    // 게임 시작 시 플레이어 위치, 카메라, 타이머, 구역 정보 초기화
    InitGame(multi);

    // 맵 BGM 상태 초기화
    // 이전 게임에서 재생된 구역 정보가 남지 않도록 -1로 초기화
    currentMapBgmZone = -1;

    // 게임 시작 위치에 맞는 첫 번째 맵 BGM 재생
    // 시작 위치는 지하이므로 under.mp3가 재생됨
    ChangeMapBGM(GetZone(players[currentPlayer].y));

    while (1) {
        // 완주 상태가 아닐 때만 입력 및 물리 연산 수행
        if (!gameFinished) {
            // 일반 입력 처리
            Input();

            // 테스트용 치트 입력 처리
            // C키를 누르고 있으면 공중부양
            CheckFlyCheat();

            // 물리, 충돌, 카메라, 구역 변경 처리
            Update();
        }
        else {
            // 게임 종료 상태에서도 시간 계산이 튀지 않도록 프레임 시간만 갱신
            lastFrameTime = GetTickCount64();
        }

        // 화면 출력
        Render();

        // 약 60FPS 유지
        Sleep(16);

        // ESC를 누르면 게임 종료 후 메뉴로 복귀
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }

    // 게임 화면에서 나갈 때 맵 BGM 정지
    StopMapBGM();

    // 화면 정리 후 타이틀 메뉴로 복귀
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
    move_cursor(40, 12); printf("SPACE : 점프 차징 (타임어택)");

    move_cursor(40, 14); printf("[멀티 플레이]");
    move_cursor(40, 15); printf("P1 : A / D (이동) , W (점프)");
    move_cursor(40, 16); printf("P2 : ← / → (이동) , ↑ (점프)");

    move_cursor(40, 18); printf("RULE & TIP");
    move_cursor(40, 19); printf("- 멀티는 턴제이며, 착지 시 턴과 타이머가 전환됩니다.");
    move_cursor(40, 20); printf("- 64번째 최상단 플랫폼에 먼저 도달하는 자가 승리합니다.");
    move_cursor(40, 21); printf("- 머리 위 충돌 걱정 없이 넓어진 맵을 공략해 보세요!");

    move_cursor(40, 25); printf("ESC : 메뉴로 돌아가기");
    fflush(stdout);
    (void)_getch();
}

// [2번 및 3번 요구사항 수정] 크레딧 맵을 메인 테마(하늘+구름+잔디+흙)로 변경 및 조작법 텍스트 제거
void RenderHall() {
    printf("\x1b[H");

    // 1. 하늘 배경 그리기 (1~26행) -> 발이 26행에 오므로 26행까지가 하늘
    for (int i = 0; i < 26; i++) {
        move_cursor(1, i + 1);
        set_bg_color(104);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }

    // 2. 잔디 그리기 (27행)
    move_cursor(1, 27);
    set_bg_color(102); set_font_color(32);
    for (int j = 0; j < WIDTH; j++) {
        if (j % 3 == 0) putchar('v'); else putchar(' ');
    }

    // 3. 흙 바닥 그리기 (28~30행)
    for (int i = 27; i < 30; i++) {
        move_cursor(1, i + 1);
        set_bg_brown(); set_font_color(30);
        for (int j = 0; j < WIDTH; j++) {
            if ((i + j) % 4 == 0) putchar('%'); else if ((i + j) % 7 == 0) putchar('.'); else putchar(' ');
        }
    }

    // 4. 구름 디자인 배치 (메인 화면 양식 적용)
    set_bg_color(104); set_font_color(97);
    move_cursor(23, 7); printf("     _ .--.      ");
    move_cursor(21, 8); printf("   (          )-.   ");
    move_cursor(20, 9); printf(" .'             '. ");
    move_cursor(20, 10); printf("(___.-._.-'.-.___) ");

    move_cursor(83, 12); printf("     _ .--.      ");
    move_cursor(81, 13); printf("   (          )-.   ");
    move_cursor(80, 14); printf(" .'             '. ");
    move_cursor(80, 15); printf("(___.-._.-'.-.___) ");

    // 크레딧 메인 타이틀 출력
    set_bg_color(104); set_font_color(93);
    move_cursor(55, 4); printf("C R E D I T");
    move_cursor(47, 6); set_font_color(97); printf("- 게임 제작에 참여한 사람들 -");

    // NPC 렌더링 (하늘 배경 위에서 깨지지 않게 보정)
    for (int i = 0; i < 3; i++) {
        set_bg_color(104);
        if (i == 0) set_font_color(92);
        else if (i == 1) set_font_color(93);
        else set_font_color(95);

        move_cursor(npcs[i].x, npcs[i].y);
        if (i == 0) { printf("\\O/"); move_cursor(npcs[i].x + 1, npcs[i].y + 1); printf("|"); }
        else if (i == 1) { printf(">O<"); move_cursor(npcs[i].x, npcs[i].y + 1); printf("(|)"); }
        else { printf(" O "); move_cursor(npcs[i].x, npcs[i].y + 1); printf("]I["); }

        move_cursor(npcs[i].x, npcs[i].y + 2); printf("/"); move_cursor(npcs[i].x + 2, npcs[i].y + 2); printf("\\");

        set_font_color(90);
        move_cursor(npcs[i].x - 2, npcs[i].y + 3); printf("=======");

        move_cursor(npcs[i].x - 1, npcs[i].y + 4); set_font_color(97); printf("%s", npcs[i].name);
    }

    // NPC 대화 말풍선 출력 (배경 투명화 보정)
    if (activeBubbleNpc != -1) {
        NPC* n = &npcs[activeBubbleNpc];
        int bx = n->x - 10;
        int by = n->y - 6;

        set_bg_color(104);
        set_font_color(97);
        move_cursor(bx, by);     printf(" ------------------------- ");
        move_cursor(bx, by + 1); printf("  [ "); set_font_color(96); printf("%s", n->name); set_font_color(97); printf(" ]");
        move_cursor(bx, by + 2); printf("  %s", n->desc1);
        move_cursor(bx, by + 3); printf("  %s", n->desc2);
        move_cursor(bx, by + 4); printf(" ----------\\ /------------ ");
    }

    // 플레이어 렌더링 (하늘 배경 위에서 깨지지 않게 보정)
    int px = hallX;
    int py = hallY;
    set_bg_color(104);
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

    // [3번 요구사항 수정] 상단 조작 설명 삭제 및 조건부 대화 유도 알림만 깔끔하게 유지
    if (currentNpc != -1 && activeBubbleNpc == -1) {
        move_cursor(2, 2);
        set_bg_color(104);
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

// [수정] 켜지자마자 나오지 않도록 딜레이 시간을 추가한 로고 출력 함수
void ShowLogo() {
    clear_screen();
    Sleep(1000); // 1초(1000ms) 동안 빈 화면 유지 후 인트로 시작

    for (int i = 0; i < HEIGHT; i++) {
        move_cursor(1, i + 1); set_bg_color(40);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }
    set_font_color(96);
    Sleep(1000);
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

void DrawMenu(int menu) {
    printf("\x1b[H");

    for (int i = 0; i < HEIGHT - 2; i++) {
        move_cursor(1, i + 1);
        set_bg_color(104);
        for (int j = 0; j < WIDTH; j++) putchar(' ');
    }

    move_cursor(1, HEIGHT - 1);
    set_bg_color(102); set_font_color(32);
    for (int j = 0; j < WIDTH; j++) {
        if (j % 3 == 0) putchar('v'); else putchar(' ');
    }

    move_cursor(1, HEIGHT);
    set_bg_brown(); set_font_color(30);
    for (int j = 0; j < WIDTH; j++) {
        if (j % 4 == 0) putchar('%'); else if (j % 7 == 0) putchar('.'); else putchar(' ');
    }

    set_bg_color(104); set_font_color(97);
    move_cursor(23, 7); printf("     _ .--.      ");
    move_cursor(21, 8); printf("   (          )-.   ");
    move_cursor(20, 9); printf(" .'             '. ");
    move_cursor(20, 10); printf("(___.-._.-'.-.___) ");

    move_cursor(83, 12); printf("     _ .--.      ");
    move_cursor(81, 13); printf("   (          )-.   ");
    move_cursor(80, 14); printf(" .'             '. ");
    move_cursor(80, 15); printf("(___.-._.-'.-.___) ");

    set_font_color(97);
    move_cursor(43, 6); printf("A S C I I   J U M P   G A M E");

    int startY = 13;
    char* menus[] = { "싱글 게임", "멀티 게임", "플레이 방법", "크레딧", "나가기" };
    for (int i = 0; i < 5; i++) {
        move_cursor(53, startY + i * 2);
        if (menu == i) {
            set_bg_color(104); set_font_color(93);
            printf("▶ %s", menus[i]);
        }
        else {
            set_bg_color(104); set_font_color(97);
            printf("   %s", menus[i]);
        }
    }

    set_bg_color(104); set_font_color(92);
    move_cursor(93, 26); printf("\\O/");
    move_cursor(94, 27); printf("|");
    move_cursor(93, 28); printf("/"); move_cursor(95, 28); printf("\\");

    printf(COLOR_RESET);
    fflush(stdout);
}

// 타이틀 화면 배경음악을 시작하는 함수
void StartTitleBGM() {
    // 혹시 이전에 열려 있던 titlebgm이 있으면 닫기
    mciSendStringA("close titlebgm", NULL, 0, NULL);

    // title_bgm.mp3 파일을 titlebgm이라는 이름으로 열기
    mciSendStringA("open \"title_bgm.mp3\" alias titlebgm", NULL, 0, NULL);

    // titlebgm을 반복 재생
    // repeat 옵션 때문에 음악이 끝나도 다시 처음부터 재생됨
    mciSendStringA("play titlebgm repeat", NULL, 0, NULL);
}

// 타이틀 화면 배경음악을 정지하는 함수
void StopTitleBGM() {
    // 현재 재생 중인 titlebgm 정지
    mciSendStringA("stop titlebgm", NULL, 0, NULL);

    // 열려 있던 titlebgm 파일 닫기
    mciSendStringA("close titlebgm", NULL, 0, NULL);
}

int main() {
    system("mode con cols=120 lines=32");
    setvbuf(stdout, printBuffer, _IOFBF, sizeof(printBuffer));

    int menu = 0;
    char input;

    // 콘솔 커서 숨기기
    hide_cursor();
    // 게임 시작 로고 출력
    ShowLogo();
    // 로고가 끝난 뒤 타이틀 화면 BGM 시작
    StartTitleBGM();
    // 메인 메뉴 반복 실행
    while (1) {
        DrawMenu(menu);
        input = _getch();

        if (input == 27) break;
        if (input == 'w' || input == 'W') { if (menu > 0) menu--; }
        if (input == 's' || input == 'S') { if (menu < 4) menu++; }
        // 스페이스바를 누르면 현재 선택된 메뉴 실행
        if (input == ' ') {

            // 싱글 게임 선택
            if (menu == 0) {
                // 타이틀 화면을 벗어나므로 타이틀 BGM 정지
                StopTitleBGM();

                // 싱글 게임 실행
                RunGame(false);

                // 게임에서 메뉴로 돌아오면 타이틀 BGM 다시 재생
                StartTitleBGM();
            }

            // 멀티 게임 선택
            else if (menu == 1) {
                // 타이틀 화면을 벗어나므로 타이틀 BGM 정지
                StopTitleBGM();

                // 멀티 게임 실행
                RunGame(true);

                // 게임에서 메뉴로 돌아오면 타이틀 BGM 다시 재생
                StartTitleBGM();
            }

            // 플레이 방법 선택
            else if (menu == 2) {
                // 플레이 방법 화면 출력
                ShowHowToPlay();
            }

            // 크레딧 선택
            else if (menu == 3) {
                // 크레딧 화면 실행
                RunCreditHall();
            }

            // 나가기 선택
            else if (menu == 4) {
                // while문 탈출 후 게임 종료
                break;
            }
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