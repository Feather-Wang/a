all: a.out hextobit

a.out :
	cc wqs_unzip.c huffmantree.c bitstream.c -I. -g
hextobit :
	cc hextobit.c -o hextobit
clean:
	rm a.out hextobit
