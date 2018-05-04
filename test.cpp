#include <stdio.h>
int main(int argc, char ** argv){
	FILE *file = fopen(argv[1],"r");
	fseek(file,0,SEEK_END);
}
