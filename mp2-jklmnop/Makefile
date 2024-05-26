# Select version of g++
ifneq (, $(shell which g++-9))
CXX = g++-9
else
ifneq (, $(shell which g++-11))
CXX = g++-11
else
ifneq (, $(shell which g++))
CXX = g++
else
$(error No g++ compiler found.)
endif
endif
endif


CXX += -gdwarf-4 $(CS340)
CFLAGS = -W -Wall -Wno-pointer-sign
CFLAGS_CATCH = -fpermissive -w -std=c++11
CC = ${CXX} -x c ${CFLAGS}


all: png-dimensions

# .o rules:
png-dimensions.o: png-dimensions.c
	${CC} $^ -c -o $@

# exe rules:
png-dimensions: png-dimensions.o png-dimensions-main.c
	${CXX} $^ -o $@


# tests
test: png-dimensions.o tests/test-dimensions.cpp tests/test.o
	$(CXX) $(CFLAGS_CATCH) $^ -o $@

tests/test.o: tests/test.cpp
	$(CXX) $(CFLAGS_CATCH) $^ -c -o $@



clean:
	rm -f png-dimensions png-dimensions.o tests/test.o test
