#include "game.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>

Windmill windmill;

// 스테이지 생성자 
StageController::StageController(int width, int height)
    : serpent(width / 2, height / 2), width(width), height(height),
      growScore(0), poisonScore(0), gateScore(0), maxLength(0),
      stageLevel(1), missionLen(4), missionGrow(1), missionPoison(1), missionGate(1), missionMaxLen(5),
      missionLenDone(false), missionGrowDone(false), missionPoisonDone(false), missionGateDone(false), missionMaxDone(false),
      gatesActive(false), windmillFrozen(false), gateTimestamp(time(0)), gameStartTime(time(0)), tickSpeed(200) {

    initscr(); // 커서 초기화 
    cbreak();
    noecho();
    curs_set(0); // 커서 없앰
    keypad(stdscr, TRUE); // 키패드 활성화 
    timeout(tickSpeed); // 초기 속도 설정 

    mainWin = newwin(height, width, 0, 0); // 게임창 
    scoreBoard = newwin(7, 30, 4, width + 2); // 스코어보드 창
    missionBoard = newwin(9, 30, 11, width + 2); // 미션 창 
    timeBoard = newwin(3, 30, 1, width + 2); // 시간 창 

    srand(static_cast<unsigned int>(time(0)));

    setupMap(); // 맵초기화 
    setupStage(); // 스테이지 초기화 
    distributeItems(); // 아이템 배치 
}

// 스테이지 소멸자 
StageController::~StageController() {
    delwin(mainWin);
    delwin(scoreBoard);
    delwin(missionBoard);
    delwin(timeBoard);
    endwin();
}
// 스테이지 메인 
void StageController::execute() {
    while (true) {
        render(); //그리기
        handleInput(); // 입력 처리
        tick(); // 상태 업데이트 
    }
}

// 게임 종료 함수 
void StageController::terminateGame() {
    delwin(mainWin);
    delwin(scoreBoard);
    delwin(missionBoard);
    delwin(timeBoard);
    endwin();
    exit(0);
}


// 사용자 입력 
void StageController::handleInput() {
    int ch = getch();
    switch (ch) {
        case KEY_UP:    serpent.setDirection(UP); break;
        case KEY_DOWN:  serpent.setDirection(DOWN); break;
        case KEY_LEFT:  serpent.setDirection(LEFT); break;
        case KEY_RIGHT: serpent.setDirection(RIGHT); break;
    }
}


// 게이트 통과 
void StageController::useGate(const std::pair<int, int>& exitGate) {
    auto exitX = exitGate.first;
    auto exitY = exitGate.second;
    std::pair<int, int> newHead = { -1, -1 };
    Direction newDirection;
    Direction entryDirection = serpent.getCurrentDirection();
    std::vector<Direction> directions;


    // 진입 방향 우선순위 설정 
    switch (entryDirection) {
        case UP:    directions = { UP, RIGHT, LEFT, DOWN }; break;
        case DOWN:  directions = { DOWN, RIGHT, LEFT, UP }; break;
        case LEFT:  directions = { LEFT, UP, DOWN, RIGHT }; break;
        case RIGHT: directions = { RIGHT, DOWN, UP, LEFT }; break;
    }


    // 새로운 머리 위치랑 방향 설정 
    for (const auto& direction : directions) {
        switch (direction) {
            case UP:
                if (exitY > 0 && map[exitY - 1][exitX] == 0 && !serpent.occupies(exitX, exitY - 1)) {
                    newHead = { exitX, exitY - 1 };
                    newDirection = UP;
                }
                break;
            case DOWN:
                if (exitY < height - 1 && map[exitY + 1][exitX] == 0 && !serpent.occupies(exitX, exitY + 1)) {
                    newHead = { exitX, exitY + 1 };
                    newDirection = DOWN;
                }
                break;
            case LEFT:
                if (exitX > 0 && map[exitY][exitX - 1] == 0 && !serpent.occupies(exitX - 1, exitY)) {
                    newHead = { exitX - 1, exitY };
                    newDirection = LEFT;
                }
                break;
            case RIGHT:
                if (exitX < width - 1 && map[exitY][exitX + 1] == 0 && !serpent.occupies(exitX + 1, exitY)) {
                    newHead = { exitX + 1, exitY };
                    newDirection = RIGHT;
                }
                break;
        }
        if (newHead != std::pair<int, int>{-1, -1}) break;
    }


    // 유요한 위치 없으면 종료 
    if (newHead == std::pair<int, int>{-1, -1}) {
        terminateGame();
        return;
    }

    serpent.assignHead(newHead, newDirection);


    // 바람개비 회전 멈충(게이트가 내에 있는 경우) 
    if (gateInsideWindmill(exitGate)) {
        windmillFrozen = true;
        pauseStartTime = time(0);
    }
}


// 상태 업데이트 
void StageController::tick() {
    static int updateCounter = 0;
    serpent.refresh();
    timeout(static_cast<int>(serpent.retrieveInterval() * 1000));


    // 시간 초과인 경우 게임 오버 
    time_t currentTime = time(0);
    double elapsedSeconds = difftime(currentTime, gameStartTime);
    if (elapsedSeconds > 120) terminateGame();



    // 길이 없데이트 
    if (serpent.getSegments().size() > maxLength) {
        maxLength = serpent.getSegments().size();
    }


    // 아이템 중 오래된 거 제거 
    cleanUpItems();

    auto head = serpent.getHeadPosition();


    // 벽 or 장애물 or 스스로 충돌 시 게임 오버 
    if (map[head.second][head.first] == 1 || map[head.second][head.first] == 2 || serpent.detectCollision()) {
        terminateGame();
    }



    // 바람개비 날이랑 충돌 했는지에 대한 검사 
    for (int i = 1; i <= windmill.length; ++i) {
        if ((map[windmill.center.second + i][windmill.center.first] == 1 && serpent.occupies(windmill.center.first, windmill.center.second + i)) ||
            (map[windmill.center.second - i][windmill.center.first] == 1 && serpent.occupies(windmill.center.first, windmill.center.second - i)) ||
            (map[windmill.center.second][windmill.center.first + i] == 1 && serpent.occupies(windmill.center.first + i, windmill.center.second)) ||
            (map[windmill.center.second][windmill.center.first - i] == 1 && serpent.occupies(windmill.center.first - i, windmill.center.second)) ||
            (map[windmill.center.second + i][windmill.center.first + i] == 1 && serpent.occupies(windmill.center.first + i, windmill.center.second + i)) ||
            (map[windmill.center.second - i][windmill.center.first - i] == 1 && serpent.occupies(windmill.center.first - i, windmill.center.second - i)) ||
            (map[windmill.center.second + i][windmill.center.first - i] == 1 && serpent.occupies(windmill.center.first - i, windmill.center.second + i)) ||
            (map[windmill.center.second - i][windmill.center.first + i] == 1 && serpent.occupies(windmill.center.first + i, windmill.center.second - i))) {
            terminateGame();
        }
    }

    // 아이템 경우 처리 

    // 성장 아이템 먹은 경우 
    if (map[head.second][head.first] == 5) {
        serpent.extend();
        map[head.second][head.first] = 0;
        growScore++;
    }


    // 독아이템 먹은 경우 
    else if (map[head.second][head.first] == 6) {
        serpent.shrink();
        map[head.second][head.first] = 0;
        poisonScore++;
        if (serpent.getSegments().size() < 3) terminateGame();
    }


    //속도 증가 아이템 
    else if (map[head.second][head.first] == 9) {
        serpent.boostSpeed();
        map[head.second][head.first] = 0;
    }

    //속도 감소 아이템 
    else if (map[head.second][head.first] == 10) {
        serpent.reduceSpeed();
        map[head.second][head.first] = 0;
    }


    //게이트 통과 
    if (head == gateA) {
        useGate(gateB);
        gateScore++;
    }
    else if (head == gateB) {
        useGate(gateA);
        gateScore++;
    }


    // 미션 상태 업데이트 
    checkMissions();


    //완료 했을 경우 스테이지 다음 진행 
    if (missionLenDone && missionGrowDone && missionPoisonDone && missionGateDone) {
        proceedNextStage();
    }

    distributeItems();



    // 10회마다 바람개비 회전 
    if (stageLevel == 4) {
        if (++updateCounter >= 10) {
            spinWindmill();
            updateCounter = 0;
        }
    }

    timeout(static_cast<int>(serpent.retrieveInterval() * 1000));
}



// 미션 상태 업데이트 
void StageController::checkMissions() {
    missionLenDone = (serpent.getSegments().size() >= static_cast<size_t>(missionLen));
    missionGrowDone = (growScore >= missionGrow);
    missionPoisonDone = (poisonScore >= missionPoison);
    missionGateDone = (gateScore >= missionGate);
    missionMaxDone = (maxLength >= missionMaxLen);
}


// 다음 스테이지로 진행 
void StageController::proceedNextStage() {
    stageLevel++;
    if (stageLevel > 4) {
        terminateGame();
    } else {
        resetStage();
        serpent.defineInterval(serpent.retrieveInterval() * 0.5f);
    }
}


// 초기 맵 설정 
void StageController::setupMap() {
    map = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (int i = 0; i < width; ++i) {
        map[0][i] = 1;
        map[height - 1][i] = 1;
    }
    for (int i = 0; i < height; ++i) {
        map[i][0] = 1;
        map[i][width - 1] = 1;
    }
    map[0][0] = 2;
    map[0][width - 1] = 2;
    map[height - 1][0] = 2;
    map[height - 1][width - 1] = 2;
}


//스테이지 초기화 

void StageController::setupStage() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map[y][x] != 1 && map[y][x] != 2) {
                map[y][x] = 0;
            }
        }
    }

    if (stageLevel == 2) {
        int startX = 15;
        int startY = height - 15;
        for (int i = startY; i < height - 5; ++i) {
            map[i][startX] = 1;
        }
        for (int i = 5; i < startX; ++i) {
            map[startY][i] = 1;
        }
    }
    else if (stageLevel == 3) {
        for (int i = 5; i < width - 5; ++i) {
            map[height / 3][i] = 1;
            map[2 * height / 3][i] = 1;
        }
    }
    else if (stageLevel == 4) {
        setupWindmill();
        serpent = Serpent(width - 5, 1);
        serpent.setDirection(DOWN);
    }
    /*
    missionLen += 1;
    missionGrow += 1;
    missionPoison += 1;
    missionGate += 1;
    */
}


// 화면 그리기 
void StageController::render() {
    werase(mainWin);
    werase(scoreBoard);
    werase(missionBoard);
    werase(timeBoard);

    mvprintw(0, width + 5, "Stage %d", stageLevel);

    time_t currentTime = time(0);
    double elapsedSeconds = difftime(currentTime, gameStartTime);
    int minutes = static_cast<int>(elapsedSeconds) / 60;
    int seconds = static_cast<int>(elapsedSeconds) % 60;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (map[y][x]) {
                case 1: mvwaddch(mainWin, y, x, '#'); break;
                case 2: mvwaddch(mainWin, y, x, '*'); break;
                case 5: mvwaddch(mainWin, y, x, '+'); break;
                case 6: mvwaddch(mainWin, y, x, '-'); break;
                case 7: mvwaddch(mainWin, y, x, 'G'); break;
                case 9: mvwaddch(mainWin, y, x, '>'); break;
                case 10: mvwaddch(mainWin, y, x, '<'); break;
            }
        }
    }

    for (auto& segment : serpent.getSegments()) {
        mvwaddch(mainWin, segment.second, segment.first, 'O');
    }

    box(scoreBoard, 0, 0);
    mvwprintw(scoreBoard, 1, 1, "Score Board");
    mvwprintw(scoreBoard, 2, 1, "B: %lu / %d", serpent.getSegments().size(), maxLength);
    mvwprintw(scoreBoard, 3, 1, "+: %d", growScore);
    mvwprintw(scoreBoard, 4, 1, "-: %d", poisonScore);
    mvwprintw(scoreBoard, 5, 1, "G: %d", gateScore);

    // 미션 테두리 그리기 
    box(timeBoard, 0, 0);
    mvwprintw(timeBoard, 1, 1, "Time: %02d:%02d", minutes, seconds);


    // 그리는 부분 
    box(missionBoard, 0, 0);
    mvwprintw(missionBoard, 1, 1, "Mission");
    mvwprintw(missionBoard, 2, 1, "Pass the stage in 2 minutes");
    mvwprintw(missionBoard, 3, 1, "B: %d (%c)", missionLen, missionLenDone ? 'v' : ' ');
    mvwprintw(missionBoard, 4, 1, "Max B: %d (%c)", missionMaxLen, missionMaxDone ? 'v' : ' ');
    mvwprintw(missionBoard, 5, 1, "+: %d (%c)", missionGrow, missionGrowDone ? 'v' : ' ');
    mvwprintw(missionBoard, 6, 1, "-: %d (%c)", missionPoison, missionPoisonDone ? 'v' : ' ');
    mvwprintw(missionBoard, 7, 1, "G: %d (%c)", missionGate, missionGateDone ? 'v' : ' ');

    wrefresh(mainWin);
    wrefresh(scoreBoard);
    wrefresh(missionBoard);
    wrefresh(timeBoard);
}



// 아이템 배치
void StageController::distributeItems() {
    time_t now = time(0);

    // 성장 아이템
    if (growItems.size() < 3) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while (map[y][x] != 0 || serpent.occupies(x, y));
        map[y][x] = 5;
        growItems.push_back({x, y});
        growItemTimestamps.push_back(now);
    }

    // 독 아이템
    if (poisonItems.size() < 3) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while (map[y][x] != 0 || serpent.occupies(x, y));
        map[y][x] = 6;
        poisonItems.push_back({x, y});
        poisonItemTimestamps.push_back(now);
    }

    // 속도 증가 아이템
    if (boostItems.size() < 1) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while (map[y][x] != 0 || serpent.occupies(x, y));
        map[y][x] = 9;
        boostItems.push_back({x, y});
        boostTimestamps.push_back(now);
    }

    // 속도 감소 아이템
    if (slowItems.size() < 1) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while (map[y][x] != 0 || serpent.occupies(x, y));
        map[y][x] = 10;
        slowItems.push_back({x, y});
        slowTimestamps.push_back(now);
    }

    // 게이트 생성 조건 확인 
    if (serpent.getSegments().size() >= 4 && !gatesActive) {
        do {
            gateA = {rand() % width, rand() % height};
            gateB = {rand() % width, rand() % height};
        } while (map[gateA.second][gateA.first] != 1 || map[gateB.second][gateB.first] != 1 || gateA == gateB);

        map[gateA.second][gateA.first] = 7;
        map[gateB.second][gateB.first] = 7;

        gateTimestamp = now;
        gatesActive = true;
    }

    // 게이트 재생성 
    else if (gatesActive && difftime(now, gateTimestamp) >= 20) {
        if (!serpent.occupies(gateA.first, gateA.second) && !serpent.occupies(gateB.first, gateB.second)) {
            if (gateA.first != -1) map[gateA.second][gateA.first] = 1;
            if (gateB.first != -1) map[gateB.second][gateB.first] = 1;

            do {
                gateA = {rand() % width, rand() % height};
                gateB = {rand() % width, rand() % height};
            } while (map[gateA.second][gateA.first] != 1 || map[gateB.second][gateB.first] != 1 || gateA == gateB);

            map[gateA.second][gateA.first] = 7;
            map[gateB.second][gateB.first] = 7;
            gateTimestamp = now;
        }
    }
}

// 오래된 아이템 제거
void StageController::cleanUpItems() {
    time_t now = time(0);

    //성장 아이템 
    for (size_t i = 0; i < growItems.size(); ++i) {
        if (difftime(now, growItemTimestamps[i]) >= 10) {
            map[growItems[i].second][growItems[i].first] = 0;
            growItems.erase(growItems.begin() + i);
            growItemTimestamps.erase(growItemTimestamps.begin() + i);
            --i;
        }
    }


    // 독 아이템 
    for (size_t i = 0; i < poisonItems.size(); ++i) {
        if (difftime(now, poisonItemTimestamps[i]) >= 10) {
            map[poisonItems[i].second][poisonItems[i].first] = 0;
            poisonItems.erase(poisonItems.begin() + i);
            poisonItemTimestamps.erase(poisonItemTimestamps.begin() + i);
            --i;
        }
    }

     //속도 증가 아이템 
    for (size_t i = 0; i < boostItems.size(); ++i) {
        if (difftime(now, boostTimestamps[i]) >= 10) {
            map[boostItems[i].second][boostItems[i].first] = 0;
            boostItems.erase(boostItems.begin() + i);
            boostTimestamps.erase(boostTimestamps.begin() + i);
            --i;
        }
    }

    //속도 감소 아이템 

    for (size_t i = 0; i < slowItems.size(); ++i) {
        if (difftime(now, slowTimestamps[i]) >= 10) {
            map[slowItems[i].second][slowItems[i].first] = 0;
            slowItems.erase(slowItems.begin() + i);
            slowTimestamps.erase(slowTimestamps.begin() + i);
            --i;
        }
    }
}


// 스테이지 상태 초기화 
void StageController::resetStage() {
    if (stageLevel == 4) {
        serpent = Serpent(width - 5, 1);  // 뱀 오른쪽 상단에서 시작하게 
        serpent.setDirection(DOWN);
    } else {
        serpent = Serpent(width / 2, height / 2);  // 기본 위치
    }

    growScore = 0;
    poisonScore = 0;
    gateScore = 0;
    maxLength = 0;

    missionLenDone = false;
    missionGrowDone = false;
    missionPoisonDone = false;
    missionGateDone = false;
    missionMaxDone = false;

    gateA = { -1, -1 };
    gateB = { -1, -1 };
    gatesActive = false;
    windmillFrozen = false;

    growItems.clear();
    poisonItems.clear();
    growItemTimestamps.clear();
    poisonItemTimestamps.clear();
    boostItems.clear();
    slowItems.clear();
    boostTimestamps.clear();
    slowTimestamps.clear();

    gameStartTime = time(0); // 스테이지 시간 초기화 

    setupMap();
    setupStage();
    distributeItems();
}

// 바람개비 초기화
void StageController::setupWindmill() {
    windmill.center = {width / 2, height / 2};
    windmill.length = 5;
    windmill.state = 0;

    for (int i = 1; i <= windmill.length; ++i) {
        map[windmill.center.second + i][windmill.center.first] = 1;
        map[windmill.center.second - i][windmill.center.first] = 1;
    }
}

// 바람개비 회전
void StageController::spinWindmill() {
    static bool paused = false;
    static time_t pauseStart;

    if (paused) {
        if (difftime(time(0), pauseStart) >= 1) paused = false;
        else return;
    }

    windmill.state = (windmill.state + 1) % 8;

    for (int i = 1; i <= windmill.length; ++i) {
        map[windmill.center.second + i][windmill.center.first] = 0;
        map[windmill.center.second - i][windmill.center.first] = 0;
        map[windmill.center.second][windmill.center.first + i] = 0;
        map[windmill.center.second][windmill.center.first - i] = 0;
        map[windmill.center.second + i][windmill.center.first + i] = 0;
        map[windmill.center.second - i][windmill.center.first - i] = 0;
        map[windmill.center.second + i][windmill.center.first - i] = 0;
        map[windmill.center.second - i][windmill.center.first + i] = 0;
    }


    // 각도 0 - > 45 - > 90 -> 135 -> 180 -> 225 -> 270 -> 315 
    switch (windmill.state) {
        case 0:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second + i][windmill.center.first] = 1;
                map[windmill.center.second - i][windmill.center.first] = 1;
            }
            break;
        case 1:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second + i][windmill.center.first + i] = 1;
                map[windmill.center.second - i][windmill.center.first - i] = 1;
            }
            break;
        case 2:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second][windmill.center.first + i] = 1;
                map[windmill.center.second][windmill.center.first - i] = 1;
            }
            break;
        case 3:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second + i][windmill.center.first - i] = 1;
                map[windmill.center.second - i][windmill.center.first + i] = 1;
            }
            break;
        case 4:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second - i][windmill.center.first] = 1;
                map[windmill.center.second + i][windmill.center.first] = 1;
            }
            break;
        case 5:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second - i][windmill.center.first - i] = 1;
                map[windmill.center.second + i][windmill.center.first + i] = 1;
            }
            break;
        case 6:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second][windmill.center.first - i] = 1;
                map[windmill.center.second][windmill.center.first + i] = 1;
            }
            break;
        case 7:
            for (int i = 1; i <= windmill.length; ++i) {
                map[windmill.center.second - i][windmill.center.first + i] = 1;
                map[windmill.center.second + i][windmill.center.first - i] = 1;
            }
            break;
    }
}

// 바람개비와 게이트가 겹치는지 
bool StageController::gateInsideWindmill(const pair<int, int>& gate) {
    int x = gate.first;
    int y = gate.second;
    int cx = windmill.center.first;
    int cy = windmill.center.second;
    int len = windmill.length;

    return (x == cx && abs(y - cy) <= len) ||
           (y == cy && abs(x - cx) <= len) ||
           (abs(x - cx) <= len && abs(y - cy) <= len);
}
