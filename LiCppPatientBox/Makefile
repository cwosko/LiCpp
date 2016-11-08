CPP      = g++ -D__DEBUG__
CC       = gcc -D__DEBUG__
OBJ      = LiCppPatientBox.o
LINKOBJ  = LiCppPatientBox.o
LIBS     = -static-libgcc -g3 -lpthread
INCS     = -I"../LiCpp"
CXXINCS  = -I"../LiCpp"
BIN      = LiCppPatientBox
CXXFLAGS = $(CXXINCS) -g3 -std=c++11
CFLAGS   = $(INCS) -g3 -std=c++11
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

LiCppPatientBox.o: LiCppPatientBox.cpp
	$(CPP) -c LiCppPatientBox.cpp -o LiCppPatientBox.o $(CXXFLAGS)
