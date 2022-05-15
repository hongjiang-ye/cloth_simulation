INCFLAGS  = -I ./vecmath/include -I ./include
INCFLAGS += -I /usr/include/GL

LINKFLAGS = -L. -lglut -lGL -lGLU -lGL -lX11 -lpthread -ldl
# LINKFLAGS = -L. -lglut -lGL -lGLU 
# LINKFLAGS = -L. -lRK4 -lglut -lGL -lGLU

CFLAGS    = -g -Wall -ansi -no-pie -std=c++11
CC        = g++
SRCS      = $(wildcard *.cpp)
SRCS     += $(wildcard vecmath/src/*.cpp)
OBJS      = $(SRCS:.cpp=.o)
PROG      = a3

all: $(SRCS) $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

depend:
	makedepend $(INCFLAGS) -Y $(SRCS)

clean:
	rm $(OBJS) $(PROG)
