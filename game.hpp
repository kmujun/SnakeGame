#ifndef GAME_HPP
#define GAME_HPP

#include "serpent.hpp"
#include <ncurses.h>
#include <vector>
#include <utility>
#include <iostream>
#include <ctime>

using namespace std;

// Windmill 구조체 - 회전 오브젝트 표현
struct Windmill {
    pair<int, int> center;
    int length;
    int state;
};

extern Windmill windmill;  // 외부에서 공유되는 바람개비 정보

class StageController {
public:
    StageController(int width, int height);  // 생성자
    ~StageController();                      // 소멸자
    void execute();                          // 메인 루프 실행
    void terminateGame();                    // 게임 종료 처리

private:
    void render();                           // 화면 출력
    void tick();                             // 상태 갱신
    void handleInput();                      // 키 입력 처리
    void distributeItems();                  // 아이템 배치
    void setupMap();                         // 맵 초기화
    void setupStage();                       // 스테이지 세팅
    void resetStage();                       // 스테이지 리셋
    void cleanUpItems();                     // 오래된 아이템 제거
    void useGate(const pair<int, int>& exit); // 게이트 통과 처리
    void checkMissions();                    // 미션 진행 상황 갱신
    void proceedNextStage();                 // 다음 스테이지로 이동
    void setupWindmill();                    // 바람개비 초기화
    void spinWindmill();                     // 바람개비 회전 처리
    bool gateInsideWindmill(const pair<int, int>& gate);  // 게이트가 바람개비 안에 있는지 확인

    // 아이템 관련
    vector<pair<int, int>> growItems;
    vector<pair<int, int>> poisonItems;
    vector<time_t> growItemTimestamps;
    vector<time_t> poisonItemTimestamps;
    vector<pair<int, int>> boostItems;
    vector<pair<int, int>> slowItems;
    vector<time_t> boostTimestamps;
    vector<time_t> slowTimestamps;

    Serpent serpent;                         // 뱀 객체
    int width, height;                       // 맵 크기
    vector<vector<int>> map;                 // 맵 정보
    pair<int, int> gateA, gateB;             // 게이트 좌표
    int growScore, poisonScore, gateScore;
    int maxLength;
    int stageLevel;
    int missionLen, missionGrow, missionPoison, missionGate;
    int missionMaxLen;
    bool missionLenDone, missionGrowDone, missionPoisonDone, missionGateDone;
    bool missionMaxDone;
    bool gatesActive;
    bool windmillFrozen;
    time_t pauseStartTime;

    time_t gateTimestamp;
    time_t gameStartTime;
    int tickSpeed;

    WINDOW* mainWin;
    WINDOW* scoreBoard;
    WINDOW* missionBoard;
    WINDOW* timeBoard;
};

#endif