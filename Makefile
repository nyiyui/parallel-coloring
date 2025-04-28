CFLAGS = -g -ggdb -Wall -Wextra -Wpedantic -std=c11

%.o: src/%.c
	$(CC) -c $^ $(CFLAGS)

solver: src/solver.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph: src/test_graph.c graph.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver: src/test_solver.c graph.o solver.o
	$(CC) -o $@ $^ $(CFLAGS)

test_solver_color: src/test_solver_color.c graph.o solver.o
	$(CC) -o $@ $^ $(CFLAGS)

test_graph.dot: test_graph
	./test_graph $@

test_graph.svg: test_graph.dot
	dot -Tsvg test_graph.dot > $@

clean:
	rm -f *.o solver test_graph test_solver test_solver_color test_graph.dot test_graph.svg

.PHONY: clean
