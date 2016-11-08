CPP      = g++ -D__DEBUG__
CC       = gcc -D__DEBUG__
OBJ      = LiCppTest1.o
LINKOBJ  = LiCppTest1.o
LIBS     = -static-libgcc -g3 -lpthread
INCS     = -I"../LiCpp"
CXXINCS  = -I"../LiCpp"
BIN      = LiCppTest1
CXXFLAGS = $(CXXINCS) -g3 -std=c++11
CFLAGS   = $(INCS) -g3 -std=c++11
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

LiCppTest1.o: LiCppTest1.cpp
	$(CPP) -c LiCppTest1.cpp -o LiCppTest1.o $(CXXFLAGS)
