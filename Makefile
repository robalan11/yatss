CC = gcc
CXX = g++
CPPFLAGS = -Wall -Wextra -ansi -pedantic -Wno-unused-parameter
LDFLAGS = -Bdynamic
LDLIBS = -lwinmm
OBJECTS = filter.o instrument.o sounds.o utilities.o main.o

.PHONY: all clean

all: yatss

yatss: $(OBJECTS)
	@echo LD $@
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o yatss

%.o: %.cpp
	@echo CXX $@
	$(CC) $(CPPFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJECTS) yatss.exe
