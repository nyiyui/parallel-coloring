CFLAGS = -g -ggdb -Wall -Wextra -Wpedantic -std=c11

%.o: src/%.c
	$(CC) -c $^ $(CFLAGS)

solver: src/solver.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph: src/test_graph.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver: src/test_solver.c graph.o solver.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph.dot: test_graph
	./test_graph $@

test_graph.svg: test_graph.dot
	dot -Tsvg test_graph.dot > $@
