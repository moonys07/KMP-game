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

// 콘솔 화면 크기
#define WIDTH 120
#define HEIGHT 30

// 점프 차징, 발판 개수 관련 값
#define MAX_GAUGE 1000.0f
#define PLATFORM_COUNT 64

// 점프 물리값. 너무 크게 바꾸면 난이도가 확 달라짐
#define GRAVITY 0.20f
#define MAX_JUMP_VX 2.8f 
#define MAX_JUMP_VY 3.8f 

#define COLOR_RESET "\x1b[0m"

// 플레이어 위치, 속도, 움직임 상태를 저장하는 구조체
typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool isJumping;
    bool isMoving;
    int walkTimer;
} Player;

// 발판의 위치와 크기를 저장하는 구조체
typedef struct {
    int x;
    int y;
    int width;
    int height;
} Platform;

// 크레딧 화면에 나오는 NPC 정보
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

// 지금 어떤 구역 BGM이 나오는지 저장
// -1은 아직 아무 BGM도 안 튼 상태
int currentMapBgmZone = -1;

// 발표 때 scanf 설명하려고 넣은 테스트 모드 값
bool cheatModeEnabled = false; // 치트 모드 ON/OFF
float cheatFlyPower = 2.0f;    // 공중부양 상승 속도

// 타이머랑 게임 클리어 상태
ULONGLONG playTime[2] = { 0, 0 }; // 각 플레이어의 누적 플레이 시간
ULONGLONG lastFrameTime = 0;      // 이전 프레임 시간 저장
bool gameFinished = false;        // 클리어 여부
int winnerPlayer = -1;            // 멀티에서 이긴 플레이어

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

// 실제 게임에서 밟는 발판들. y값이 작아질수록 위쪽으로 올라감
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

    // 마지막으로 올라가기 전 발판
    {88, -531, 14, 2},

    // 마지막 발판. 여기에 닿으면 클리어 처리
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

// y좌표로 현재 구역을 구함
int GetZone(float y) {
    if (y > -135) return 0;
    if (y > -325) return 1;
    return 2;
}

// 구역에 따라 배경색을 바꿔줌
void SetZoneColor(int y) {
    int zone = GetZone((float)y);
    if (zone == 0) set_bg_color(40);
    else if (zone == 1) set_bg_color(104);
    else set_bg_color(106);
}

void DrawPart(int screenX, int screenY, float worldY, int playerIndex, const char* str) {
    if (screenY < 0 || screenY >= HEIGHT || screenX < 1 || screenX > WIDTH) return;

    // 화면 위치에 맞는 배경색을 다시 맞춰줌
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

// 플레이어 상태에 따라 모양을 다르게 그림
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

// 플레이어가 발판이랑 겹치는지 확인
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

// 게임을 처음 시작할 때 값들을 초기화
void InitGame(bool multi) {
    isMulti = multi;
    currentPlayer = 0;
    isCharging = false;
    cameraY = 0;
    currentZone = -1;
    areaNotiTimer = 0;

    // 새 게임이니까 시간 기록도 다시 0으로 맞춤
    playTime[0] = 0;
    playTime[1] = 0;
    gameFinished = false;
    winnerPlayer = -1;

    players[0] = (Player){ 10, 24, 0, 0, false, false, 0 };
    players[1] = (Player){ 60, 24, 0, 0, false, false, 0 };
    clear_screen();
    lastFrameTime = GetTickCount64();
}

// 키 입력을 받아서 이동, 차징, 점프를 처리
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

// 맵 BGM을 끄고 닫음
void StopMapBGM() {
    mciSendStringA("stop mapbgm", NULL, 0, NULL);
    mciSendStringA("close mapbgm", NULL, 0, NULL);
}

// 파일 이름을 받아서 맵 BGM으로 재생
void StartMapBGM(const char* filename) {
    char command[256];
    MCIERROR err;
    char errorText[256];

    // 다른 구역 음악이 나오고 있을 수 있어서 먼저 정지
    StopMapBGM();

    // mp3 파일을 mapbgm이라는 이름으로 열어둠
    sprintf(command, "open \"%s\" type mpegvideo alias mapbgm", filename);
    err = mciSendStringA(command, NULL, 0, NULL);

    // 파일 이름이 틀리거나 파일이 없으면 여기로 들어옴
    if (err != 0) {
        mciGetErrorStringA(err, errorText, sizeof(errorText));
        MessageBoxA(NULL, errorText, "Map BGM open 실패", MB_OK);
        return;
    }

    // 맵 BGM 볼륨 설정
    mciSendStringA("setaudio mapbgm volume to 450", NULL, 0, NULL);

    // 음악 반복 재생
    err = mciSendStringA("play mapbgm repeat", NULL, 0, NULL);

    // BGM 재생에 실패했을 때 오류창 출력
    if (err != 0) {
        mciGetErrorStringA(err, errorText, sizeof(errorText));
        MessageBoxA(NULL, errorText, "Map BGM play 실패", MB_OK);
        return;
    }
}

// 구역이 바뀌면 그 구역에 맞는 음악으로 바꿈
void ChangeMapBGM(int zone) {
    // 같은 구역이면 굳이 다시 틀 필요 없음
    if (currentMapBgmZone == zone) return;

    // 현재 BGM 구역 정보 갱신
    currentMapBgmZone = zone;

    // 구역 번호마다 다른 음악 사용
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

// 매 프레임마다 위치, 충돌, 타이머 같은 게임 상태를 바꿈
void Update() {
    // 프레임 사이 시간을 구해서 현재 플레이어 시간에만 더함
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

                        // 마지막 발판이면 게임 클리어
                        if (i == PLATFORM_COUNT - 1) {
                            gameFinished = true;
                            winnerPlayer = currentPlayer;
                            // 클리어했으므로 맵 BGM 정지
                            StopMapBGM();
                            // 클리어 효과음 재생
                            PlaySound(TEXT("gameclear.wav"), NULL, SND_ASYNC | SND_FILENAME);
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

    // 플레이어 높이에 따라 현재 구역 확인
    int newZone = GetZone(p->y);

    // 구역이 바뀌었으면 알림이랑 BGM도 바꿈
    if (newZone != currentZone) {
        // 현재 구역 정보 갱신
        currentZone = newZone;

        // 구역 이름을 잠깐 보여주기 위한 시간
        areaNotiTimer = 120;

        // 구역 음악 변경
        ChangeMapBGM(newZone);
    }
}

// =====================
// 포물선 색상 함수
// =====================

// 포물선이 배경에 묻히지 않게 구역마다 색을 다르게 줌
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

// 화면 전체를 다시 그리는 함수
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

    // 점프 차징 중일 때 예상 궤적을 보여줌
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
                // 궤적 위치의 배경색과 글자색 맞추기
                SetZoneColor((int)simY);
                SetTrajectoryColor(simY);

                move_cursor(ix + 1, iy);
                printf("*");
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

    // 화면 왼쪽 위에 높이랑 시간을 표시
    SetZoneColor(cameraY + 1);
    move_cursor(2, 2);
    set_font_color(97);
    char* zname = (currentZone == 0) ? "지하" : (currentZone == 1) ? "지상" : "하늘";
    int height_m = (27 - (int)players[currentPlayer].y) / 2;

    if (!isMulti) {
        printf("[%s] 현재 높이 : %dm  |  소요 시간 : %.2f초", zname, height_m, (float)playTime[0] / 1000.0f);
    }
    else {
        printf("[%s] 높이 : %dm  |  P1 : %.2f초 %s  |  P2 : %.2f초 %s",
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

    // 클리어하면 중앙에 결과창 표시
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
// scanf 테스트 모드 설정
// =====================

// 발표에서 scanf 사용을 보여주기 위해 만든 부분
// 치트를 켰을 때만 상승 속도를 한 번 더 입력받음
void SetupCheatModeByScanf() {
    int cheatInput = 0;
    float powerInput = 2.0f;
    int scanResult;

    clear_screen();

    printf("=====================================\n");
    printf("        TEST CHEAT SETTING\n");
    printf("=====================================\n");
    printf("치트 모드를 사용할까요?\n");
    printf("0 = 사용 안 함, 1 = 사용\n\n");
    printf("입력: ");

    // printf 내용이 바로 보이도록 해줌
    fflush(stdout);

    // 먼저 치트 모드를 쓸지 입력받음
    scanResult = scanf("%d", &cheatInput);

    // 엔터가 버퍼에 남아서 다음 입력에 영향 주는 걸 막음
    while (getchar() != '\n');

    // 입력이 잘못됐으면 치트 비활성화
    if (scanResult != 1) {
        cheatModeEnabled = false;
        cheatFlyPower = 2.0f;
        clear_screen();
        return;
    }

    // 1을 입력했을 때만 치트 모드 켜기
    if (cheatInput == 1) {
        cheatModeEnabled = true;

        printf("\n상승 속도를 입력하시오.\n");
        printf("추천값: 2.0\n");
        printf("입력: ");

        fflush(stdout);

        // 치트를 켰을 때만 속도값을 추가 입력
        scanResult = scanf("%f", &powerInput);

        // 엔터가 버퍼에 남아서 다음 입력에 영향 주는 걸 막음
        while (getchar() != '\n');

        // 이상한 값을 넣으면 기본값으로 처리
        if (scanResult != 1 || powerInput <= 0.0f) {
            cheatFlyPower = 2.0f;
        }
        else {
            cheatFlyPower = powerInput;
        }
    }
    else {
        cheatModeEnabled = false;
        cheatFlyPower = 2.0f;
    }

    clear_screen();
}

// C키를 누르는 동안 위로 올라가는 테스트 기능
void CheckFlyCheat() {
    // 치트 모드를 안 켰으면 아무것도 안 함
    if (!cheatModeEnabled) return;

    Player* p = &players[currentPlayer];

    // C키를 누르고 있으면 공중부양 활성화
    if (GetAsyncKeyState('C') & 0x8000) {

        // 차징 중이었다면 차징 해제
        isCharging = false;

        // 공중 상태로 변경
        p->isJumping = true;

        // scanf로 입력한 값만큼 위로 올라감
        p->vy = -cheatFlyPower;

        // 올라가는 중에도 좌우 이동은 가능하게 함
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

// 실제 게임 화면을 실행하는 메인 루프
void RunGame(bool multi) {
    // 플레이어 위치, 카메라, 타이머를 초기화
    InitGame(multi);

    // 게임 시작 전에 테스트 모드 설정
    SetupCheatModeByScanf();

    // scanf 입력 시간은 플레이 시간에 포함하지 않도록 타이머 기준점 재설정
    lastFrameTime = GetTickCount64();

    // 이전 게임에서 남은 BGM 구역 값을 초기화
    currentMapBgmZone = -1;

    // 시작 위치에 맞는 첫 BGM 재생
    ChangeMapBGM(GetZone(players[currentPlayer].y));

    while (1) {
        // 클리어 전에는 입력과 물리 계산을 계속 함
        if (!gameFinished) {
            // 이동, 점프 입력 처리
            Input();

            // C키 테스트 기능 처리
            CheckFlyCheat();

            // 위치, 충돌, 카메라 처리
            Update();
        }
        else {
            // 클리어 후에는 시간만 더 안 늘어나게 처리
            lastFrameTime = GetTickCount64();
        }

        // 화면 다시 그리기
        Render();

        // 너무 빠르게 반복되지 않게 잠깐 쉬기
        Sleep(16);

        // ESC를 누르면 메뉴로 돌아감
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            while (GetAsyncKeyState(VK_ESCAPE) & 0x8000) Sleep(10);
            while (_kbhit()) _getch();
            break;
        }
    }

    // 게임을 나갈 때 맵 음악 끄기
    StopMapBGM();

    // 메뉴로 돌아가기 전에 화면 정리
    clear_screen();
}

// 플레이 방법 화면
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

// 크레딧 화면을 그리는 함수
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

    // 구름 모양 출력
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

    // NPC 근처에 가면 E키 안내만 보여줌
    if (currentNpc != -1 && activeBubbleNpc == -1) {
        move_cursor(2, 2);
        set_bg_color(104);
        set_font_color(96);
        printf("가까이 왔습니다! E키를 눌러보세요.");
    }

    printf(COLOR_RESET);
    fflush(stdout);
}

// 플레이어가 가까이 간 NPC가 있는지 확인
int GetNearNpc() {
    for (int i = 0; i < 3; i++) {
        if (abs(hallX - npcs[i].x) < 5) return i;
    }
    return -1;
}

// 크레딧 화면에서 플레이어를 움직이는 루프
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

// 게임 시작할 때 나오는 로고 화면
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

// 메인 메뉴 화면을 그림
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

// 타이틀 화면 음악 시작
void StartTitleBGM() {
    // 이전에 열려 있던 음악이 있으면 닫기
    mciSendStringA("close titlebgm", NULL, 0, NULL);

    // title_bgm.mp3 파일 열기
    mciSendStringA("open \"title_bgm.mp3\" alias titlebgm", NULL, 0, NULL);

    // 반복 재생
    mciSendStringA("play titlebgm repeat", NULL, 0, NULL);
}

// 타이틀 화면 음악 정지
void StopTitleBGM() {
    mciSendStringA("stop titlebgm", NULL, 0, NULL);
    mciSendStringA("close titlebgm", NULL, 0, NULL);
}

// 프로그램 시작 지점
int main() {
    system("mode con cols=120 lines=32");
    setvbuf(stdout, printBuffer, _IOFBF, sizeof(printBuffer));

    int menu = 0;
    char input;

    // 기본 화면 세팅 후 로고와 메뉴 시작
    hide_cursor();
    ShowLogo();
    StartTitleBGM();
    // 메뉴는 나가기 전까지 계속 반복
    while (1) {
        DrawMenu(menu);
        input = _getch();

        if (input == 27) break;
        if (input == 'w' || input == 'W') { if (menu > 0) menu--; }
        if (input == 's' || input == 'S') { if (menu < 4) menu++; }
        // 스페이스바로 선택한 메뉴 실행
        if (input == ' ') {

            // 싱글 게임
            if (menu == 0) {
                // 게임 화면으로 들어가기 전에 메뉴 음악 정지
                StopTitleBGM();

                // 싱글 실행
                RunGame(false);

                // 다시 메뉴로 돌아오면 음악 재생
                StartTitleBGM();
            }

            // 멀티 게임
            else if (menu == 1) {
                // 게임 화면으로 들어가기 전에 메뉴 음악 정지
                StopTitleBGM();

                // 멀티 실행
                RunGame(true);

                // 다시 메뉴로 돌아오면 음악 재생
                StartTitleBGM();
            }

            // 플레이 방법
            else if (menu == 2) {
                // 설명 화면 출력
                ShowHowToPlay();
            }

            // 크레딧
            else if (menu == 3) {
                // 크레딧 화면 실행
                RunCreditHall();
            }

            // 나가기
            else if (menu == 4) {
                // 반복문을 빠져나가서 종료
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