APPNAME = hgui
SOURCES = $(shell find . -type f -name '*.cc'|xargs)
HEADERS = $(shell find . -type f -name '*.h' |xargs)
CXX = g++ -std=c++20
OPT = -O2 $(INCLUDES) -Wall -ffast-math -Wsign-compare -Wpointer-arith -Wcast-qual -Wcast-align
#OPT = -O0 -g3 $(INCLUDES) -Wall -ffast-math -Wsign-compare -Wpointer-arith -Wcast-qual -Wcast-align

LIB = -lm -lpthread `pkg-config --libs --cflags ncurses`

INCLUDES = -I/usr/include -I.

all:	$(APPNAME)
	@wc $(SOURCES) $(HEADERS)
	@echo OK.

$(APPNAME):
	$(CXX) $(OPT) -o $(APPNAME) $(SOURCES) $(LIB)

clean:
	@echo Cleaning up...
	@rm -f nohup.out core gmon.out *.o *~ $(APPNAME)
	@echo Done.

indent:
	@indent -lc0 -sc -ss -ts8 -d0 -br -brs -lp -i8 -bli0 -npsl -l1000 *.cc *.h
	@echo Indented!

