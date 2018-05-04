all:
	mpic++ kmeans.cpp -o kmeans -lmpi -L/usr/mpi/gcc/openmpi-2.1.2a1/lib
clean:
	rm -rf kmeans *.o
