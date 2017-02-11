#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char M[256];

int main(int argc, char *argv[]) 
{
	
	
	while(1)
	{
		printf("Mensaje: ");
		scanf("%s", M);
		printf("%i Caracteres\n", strlen(M));
	}
	return 0;
}
