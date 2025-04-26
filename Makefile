%.o: %.c
	$(CC) -c $^

solver: src/solver.c graph.o
	$(CC) -o solver $^
