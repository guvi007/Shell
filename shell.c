#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<limits.h>
#include<signal.h>
#include <sys/types.h>

/*
	Splits the array with Type = {">", "<", " "}(Redirect from).
	Returns the final size of the splited array
*/
int splitByType(char *line, char *array[], char *type)
{
	char *splitPipe = strtok(line,type);
	int i = 0;

	while( splitPipe != NULL )
	{
		array[i++] = splitPipe;
		splitPipe = strtok(NULL, type);
	}
	return i;
}


int findAttribute(int low, char *command, char *string)
{
	int lengthString = strlen(command);
	int n = strlen(string);
	int i = low;
	for(; i<lengthString; ++i)
	{
		if (command[i] == ' ' || command[i] == '>' || command[i] == '<')
		{
			for(int j=low; j<i; j++)
			{
				string[n+j-low] = command[j];
			}
			break;
		}
		else if(i == lengthString - 1) {
			for(int j=low; j<i+1; j++)
			{
				string[n+j-low] = command[j];
			}
			i += 1;
			break;
		}
	}
	return i;
}

/*
	Determines the command that has to be executed
*/
void determineExec(char command[])
{
	int lengthString = strlen(command);
	char executable[1000] = "";
	char attribute[1000] = "";

	int i = findAttribute(0, command, executable);

	int invalidHyphen = 0;
	while(i<lengthString)
	{
		if(i+1 >= lengthString) {
			printf("%s\n", "Invalid Command");
			return;
		}

		if(command[i] == '-'){
			if(invalidHyphen == 1) {
				printf("%s\n", "Invalid Command");
				return;
			}
			i = findAttribute(i+1, command, attribute);
		}
		else if(command[i] == '>') {
			invalidHyphen = 1;
		}
		else if(command[i] == '<') {
			invalidHyphen = 1;
		}
		else if(command[i] == ' ') {
			i+=1;
		}
	}

	printf("%s\n", attribute);
	printf("%s\n", executable);

}


/*
	Splits the array with "|".
*/
void splitByPipe(char *line)
{
	char *array[1000];
	int elements = splitByType(line, array, "|");

	if(elements == 1)
	{
		determineExec(array[0]);
	}
	else {
		for (int j = 0; j < elements; ++j)
		{
			printf("%s\n", array[j]);
		}
	}
}

/*
	Function to take input from stdin
*/
void Input()
{
	// taking input
	printf("%s", "$ ");
	char line[1000] = "";
  char c = getchar();
	int i = 0;
	line[i] = c;
  while (c != '\n' && c != EOF) {
			i += 1;
			c = getchar();
			line[i] = c;
	}
	line[i] = '\0';

	// close shell on "exit" command
	if(strcmp("exit", line) == 0) {
		exit(0);
	}
	splitByPipe(line);
}

void main() {
	while(1) {
		Input();
	}
}
