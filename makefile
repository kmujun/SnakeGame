# 컴파일러와 컴파일 옵션 설정
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

# ncurses 라이브러리를 설정함 
LDFLAGS = -lncurses

# 타겟 실행 파일 이름
TARGET = snake

# 소스 파일 목록
SRCS = main.cpp game.cpp serpent.cpp

# 오브젝트 파일 목록
OBJS = $(SRCS:.cpp=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# 개별 .cpp -> .o 컴파일 규칙
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 클린 명령어
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
