%.o: src/%.c
	$(CC) -c $^

solver: src/solver.c graph.o
	$(CC) -o solver $^

test_graph: src/test_graph.c graph.o
	$(CC) -o test_graph $^

test_graph.dot: test_graph
	./test_graph $@

test_graph.svg: test_graph.dot
	dot -Tsvg test_graph.dot > $@
