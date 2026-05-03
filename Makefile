# Compiler
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic

# Targets
ROBOT_SRCS = $(wildcard robots/*.cpp)
ROBOT_LIBS = $(patsubst robots/%.cpp, lib%.so, $(ROBOT_SRCS))

all: test_robot RobotWarz $(ROBOT_LIBS)

RobotBase.o: RobotBase.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -fPIC -c RobotBase.cpp

test_robot: test_robot.cpp RobotBase.o
	$(CXX) $(CXXFLAGS) test_robot.cpp RobotBase.o -ldl -o test_robot

RobotWarz: RobotWarz.cpp Arena.cpp RobotBase.o
	$(CXX) $(CXXFLAGS) RobotWarz.cpp Arena.cpp RobotBase.o -ldl -o RobotWarz

lib%.so: robots/%.cpp RobotBase.o
	$(CXX) $(CXXFLAGS) -shared -fPIC -I. -o $@ $< RobotBase.o

clean:
	rm -f *.o test_robot RobotWarz *.so
