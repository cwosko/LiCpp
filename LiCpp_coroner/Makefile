CPP      = g++ -D__DEBUG__
CC       = gcc -D__DEBUG__
OBJ      = LiCpp_coroner.o
LINKOBJ  = LiCpp_coroner.o
LIBS     = -static-libgcc -g3 -lpthread
INCS     = -I"../LiCpp"
CXXINCS  = -I"../LiCpp"
BIN      = LiCpp_coroner
CXXFLAGS = $(CXXINCS) -g3 -std=c++11
CFLAGS   = $(INCS) -g3 -std=c++11
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

LiCpp_coroner.o: LiCpp_coroner.cpp
	$(CPP) -c LiCpp_coroner.cpp -o LiCpp_coroner.o $(CXXFLAGS)
