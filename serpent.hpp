#ifndef SNAKE_HPP
#define SNAKE_HPP

#include <deque>
#include <utility>
#include <chrono>

using namespace std;

// 방향 열거형 - 뱀의 이동 방향
enum Direction { UP, DOWN, LEFT, RIGHT };

// Serpent 클래스 - 게임 내 뱀을 나타냄
class Serpent {
public:
    // 생성자 - 초기 위치 설정
    Serpent(int startX, int startY);

    // 상태 갱신 - 이동 주기에 따라 이동 수행
    void refresh();

    // 이동 수행
    void advance();

    // 방향 설정
    void setDirection(Direction newDir);

    // 몸통 정보 반환
    deque<pair<int, int>> getSegments() const;

    // 머리 좌표 반환
    pair<int, int> getHeadPosition() const;

    // 성장 처리
    void extend();

    // 축소 처리
    void shrink();

    // 자가 충돌 여부 확인
    bool detectCollision() const;

    // 머리와 방향 설정
    void assignHead(pair<int, int> newHead, Direction newDir);

    // 해당 좌표를 차지하고 있는지 확인
    bool occupies(int x, int y) const;

    // 현재 방향 반환
    Direction getCurrentDirection() const;

    // 속도 증가
    void boostSpeed();

    // 속도 감소
    void reduceSpeed();

    // 이동 간격 설정
    void defineInterval(float interval);

    // 이동 간격 반환
    float retrieveInterval() const;

private:
    deque<pair<int, int>> segments;  // 몸통 좌표
    Direction currentDir;  // 현재 이동 방향
    bool pendingGrowth;  // 다음 이동 시 성장 여부
    chrono::time_point<chrono::steady_clock> prevMoveTime;  // 마지막 이동 시점
    float intervalSec;  // 이동 간격(초 단위)
    chrono::time_point<chrono::steady_clock> speedAdjustedAt;  // 속도 조절 시점
    bool isBoosted;   // 속도 증가 상태
    bool isSlowed;    // 속도 감소 상태
};

#endif
