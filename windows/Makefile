CC       ?= x86_64-w64-mingw32-gcc
CXX      ?= x86_64-w64-mingw32-g++
CFLAGS   ?= -g -O2 -Wall
CXXFLAGS ?= -g -O2 -Wall
LDFLAGS   = -L.

CPPFLAGS = \
 -DWIN32 -DNDEBUG -D_WINDOWS -DWINDOWS -D_CRT_SECURE_NO_WARNINGS \
 -DBCD_MATH -DDECIMAL_CALL_BY_REFERENCE=1 -DDECIMAL_GLOBAL_ROUNDING=1 \
 -DDECIMAL_GLOBAL_ROUNDING_ACCESS_FUNCTIONS=1 -DDECIMAL_GLOBAL_EXCEPTION_FLAGS=1 \
 -DDECIMAL_GLOBAL_EXCEPTION_FLAGS_ACCESS_FUNCTIONS=1

CORE_OBJS = \
 $(addprefix commands,1 2 3 4 5 6 7) \
 display globals helpers keydown linalg1 linalg2 main math1 math2 \
 phloat sto_rcl tables variables \

WIN32_OBJS = \
 keymap msg2string \
 shell shell_loadimage shell_skin shell_spool skins StdAfx

OBJECTS = \
 $(addprefix core_,$(addsuffix .o,$(CORE_OBJS))) \
 $(addsuffix .o,$(WIN32_OBJS))

EXE = .exe
LINK_CC = $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
LINK_CXX = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

all: free42dec$(EXE)

free42dec$(EXE): $(OBJECTS) $(LIBBID)
	$(LINK_CXX) -lbid-mingw

keymap2cc$(EXE): keymap2cc.o
	$(LINK_CXX)

skin2cpp$(EXE): skin2cc.o
	$(LINK_CXX)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<
%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

-include $(wildcard *.d)
