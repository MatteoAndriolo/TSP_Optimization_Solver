#include"log.h"
#include<stdio.h>
#include<stdlib.h>

void print_info(const char *msg)
{
	printf("\n\n INFO: %s \n\n", msg);
	fflush(NULL);
	exit(1);
}

void print_debug(const char *msg)
{
	printf("\nDEBUG: %s \n", msg);
	fflush(NULL);
}

void print_error(const char *err)
{
	printf("\n\n ERROR: %s \n\n", err);
	fflush(NULL);
	exit(1);
}
