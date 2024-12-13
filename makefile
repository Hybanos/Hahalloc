lib:
	gcc -O4 -ffast-math -march=native -fpic -shared -o libhahalloc.so hahalloc.c

all: tests bench

tests: lib
	gcc -O0 tests.c -L. -lhahalloc -o out -lm
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./out

bench: lib
	gcc -O0 bench.c -L. -lhahalloc -o out -lm

	echo " *\n * THIS CAN TAKE QUITE A LONG TIME\n *"

	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./out
	python3 py/graphs.py

clean:
	rm *.o