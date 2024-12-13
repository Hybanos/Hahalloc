tests: lib
	gcc -O0 tests.c -L. -lhahalloc -o out -lm
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./out

bench: lib
	gcc -O0 speed.c -L. -lhahalloc -o out -lm
	LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH ./out
	python3 py/graphs.py

lib:
	gcc -O4 -ffast-math -march=native -fpic -shared -o libhahalloc.so hahalloc.c

clean:
	rm *.o