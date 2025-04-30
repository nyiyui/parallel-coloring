CFLAGS = -g -ggdb -Wall -Wextra -Wpedantic -std=gnu11 -fopenmp

test_all: test_graph test_solver test_solver_color test_solver_color_perf test_solver_subgraph
	time valgrind --leak-check=full ./test_graph /dev/null
	time valgrind --leak-check=full ./test_solver
	time valgrind --leak-check=full ./test_solver_color -n 100 -nnz 100 -f /dev/null
	time valgrind --leak-check=full ./test_solver_color_perf -n 100 -nnz 100 -f /dev/null
	time valgrind --leak-check=full ./test_solver_subgraph -n 100 -nnz 100 -f /dev/null

.PHONY: test_all

%.o: src/%.c
	$(CC) -c $^ $(CFLAGS)

solver.o: graph.o

solver: src/solver.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph: src/test_graph.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver: src/test_solver.c graph.o solver.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver_color: CFLAGS += -DDEBUG
test_solver_color: src/test_solver_color.c graph.o solver.o util.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver_color_perf: src/test_solver_color.c graph.o solver.o util.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver_subgraph: src/test_solver_subgraph.c graph.o solver.o util.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph.dot: test_graph
	./test_graph $@

test_graph.svg: test_graph.dot
	dot -Tsvg test_graph.dot > $@

clean:
	rm -f *.o solver test_graph test_solver test_solver_color test_graph.dot test_graph.svg

.PHONY: clean

upload:
	scp -r *.sbatch Makefile src kshibata6@login-ice.pace.gatech.edu:/home/hice1/kshibata6/cx4803/final-project

.PHONY: upload

download:
	scp kshibata6@login-ice.pace.gatech.edu:/home/hice1/kshibata6/cx4803/final-project/slurm-* .

.PHONY: download
