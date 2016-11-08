CPP      = g++ -D__DEBUG__
CC       = gcc -D__DEBUG__
OBJ      = LiCppTest2.o
LINKOBJ  = LiCppTest2.o
LIBS     = -static-libgcc -g3 -lpthread
INCS     = -I"../LiCpp"
CXXINCS  = -I"../LiCpp"
BIN      = LiCppTest2
CXXFLAGS = $(CXXINCS) -g3 -std=c++11
CFLAGS   = $(INCS) -g3 -std=c++11
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

LiCppTest2.o: LiCppTest2.cpp
	$(CPP) -c LiCppTest2.cpp -o LiCppTest2.o $(CXXFLAGS)
