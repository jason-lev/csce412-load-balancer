
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -pedantic

TARGET = load-balancer

SOURCES = main.cpp \
          request.cpp \
          requestqueue.cpp \
          webserver.cpp \
          loadbalancer.cpp \
          masterloadbalancer.cpp \
          firewall.cpp \
          utils.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
