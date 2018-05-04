#include <stdio.h>
int main(int argc, char **argv){
	FILE *file = fopen(argv[2],"r");
	fseek(file,0,SEEK_END);
	int size = ftell(file);
	printf("%e",size);
	printf("%d",size);
	printf("%d",size);
}//int
