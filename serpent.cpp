#include "serpent.hpp"
#include <ncurses.h>
#include <iostream>

// 생성자 - 초기 위치에 몸통 구성
Serpent::Serpent(int startX, int startY)
    : currentDir(RIGHT), pendingGrowth(false),
      prevMoveTime(chrono::steady_clock::now()),
      intervalSec(0.2f), isBoosted(false), isSlowed(false) {
    segments.push_back({ startX, startY });
    segments.push_back({ startX - 1, startY });
    segments.push_back({ startX - 2, startY });
}

// 상태 갱신 - 일정 시간 간격마다 이동
void Serpent::refresh() {
    auto now = chrono::steady_clock::now();
    chrono::duration<float> elapsed = now - prevMoveTime;

    // 속도 변경 상태가 지속되면 일정 시간 후 복구
    if (isBoosted || isSlowed) {
        chrono::duration<float> durationSinceSpeedChange = now - speedAdjustedAt;
        if (durationSinceSpeedChange.count() >= 5.0f) {
            intervalSec = 0.2f;
            isBoosted = false;
            isSlowed = false;
        }
    }

    // 이동 타이밍 도달 시 이동
    if (elapsed.count() >= intervalSec) {
        advance();
        prevMoveTime = now;
    }
}

// 이동 수행 - 현재 방향 기준으로 한 칸 이동
void Serpent::advance() {
    auto head = segments.front();

    switch (currentDir) {
        case UP:    head.second--; break;
        case DOWN:  head.second++; break;
        case LEFT:  head.first--;  break;
        case RIGHT: head.first++;  break;
    }

    segments.push_front(head);

    if (!pendingGrowth) {
        segments.pop_back();
    } else {
        pendingGrowth = false;
    }
}

// 방향 변경 요청
void Serpent::setDirection(Direction newDir) {
    if (currentDir == newDir) return;
    if ((currentDir == UP && newDir == DOWN) ||
        (currentDir == DOWN && newDir == UP) ||
        (currentDir == LEFT && newDir == RIGHT) ||
        (currentDir == RIGHT && newDir == LEFT)) {
        endwin();
        exit(0);
    }
    currentDir = newDir;
}

// 몸통 정보 반환
deque<pair<int, int>> Serpent::getSegments() const {
    return segments;
}

// 머리 좌표 반환
pair<int, int> Serpent::getHeadPosition() const {
    return segments.front();
}

// 성장 처리 - 다음 이동 시 꼬리 유지
void Serpent::extend() {
    pendingGrowth = true;
}

// 축소 처리 - 꼬리 하나 제거
void Serpent::shrink() {
    if (segments.size() > 1) {
        segments.pop_back();
    }
}

// 자가 충돌 여부 확인
bool Serpent::detectCollision() const {
    auto& head = segments.front();
    for (auto it = ++segments.begin(); it != segments.end(); ++it) {
        if (*it == head) return true;
    }
    return false;
}

// 머리 좌표 및 방향 강제 지정
void Serpent::assignHead(pair<int, int> newHead, Direction newDir) {
    segments.push_front(newHead);
    currentDir = newDir;
    if (!pendingGrowth) {
        segments.pop_back();
    } else {
        pendingGrowth = false;
    }
}

// 해당 좌표를 차지하는지 확인
bool Serpent::occupies(int x, int y) const {
    for (const auto& segment : segments) {
        if (segment.first == x && segment.second == y) return true;
    }
    return false;
}

// 현재 방향 반환
Direction Serpent::getCurrentDirection() const {
    return currentDir;
}

// 이동 간격 반환
float Serpent::retrieveInterval() const {
    return intervalSec;
}

// 이동 간격 설정
void Serpent::defineInterval(float interval) {
    intervalSec = interval;
}

// 속도 증가 처리
void Serpent::boostSpeed() {
    intervalSec = 0.1f;
    speedAdjustedAt = chrono::steady_clock::now();
    isBoosted = true;
    isSlowed = false;
}

// 속도 감소 처리
void Serpent::reduceSpeed() {
    intervalSec = 0.4f;
    speedAdjustedAt = chrono::steady_clock::now();
    isBoosted = false;
    isSlowed = true;
}
