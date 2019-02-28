#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void Input() {
	char line[50];
	printf("$ ");
	scanf("%s", line);
	printf("%s\n", line);
	if(strcmp("exit", line) == 0) {
		exit(0);
	}
}

void main() {
	while(1) {
		Input();
	}
}
