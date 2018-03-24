
GCC=g++
GFLAGS=-g -lpthread -lrt -Wall -pedantic
OUT=a.out

all: main.cpp
	$(GCC) $(GFLAGS) main.cpp -o $(OUT)
	./$(OUT)

gprof: main.cpp
	$(GCC) $(CFLAGS) -pg main.cpp -o $(OUT)
	gprof ./$(OUT)

helgrind: main.cpp
	$(GCC) $(CFLAGS) main.cpp -o $(OUT)
	valgrind --tool=helgrind ./$(OUT)

cachegrind: main.cpp
	$(GCC) $(CFLAGS) main.cpp -o $(OUT)
	valgrind --tool=cachegrind ./$(OUT)

sanitize: main.cpp
	$(GCC) $(CFLAGS) -O1 -fsanitize=thread -fPIC -pie main.cpp -o $(OUT)
	./$(OUT)
	
clean:
	rm -f *.out *.out.*
