EXECS=example1
MPICC?=mpicc

all: ${EXECS}
example1: example1.c
	${MPICC} -o example1 example1.c

clean:
	rm -f ${EXECS}
