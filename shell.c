#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<limits.h>
#include<signal.h>
#include <sys/types.h>

int removeSpacing(char *string, int i, int lengthString)
{
	int j = i+1;
	for(; j< lengthString; ++j) {
		if(string[j] != ' ') {
			break;
		}
	}
	return (j-1);
}

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
	char input[1000] = "";
	char output[1000] = "";
	char error[1000] = "";
	char arguments[100][1000];
	int isappend = 0, invalidHyphen = 0, numArguments = 0;
	for(int i=0;i<100;++i) {
		memset(arguments[i],'\0', 1000*sizeof(char));
	}


	for(int g=1;g<lengthString;g++)
	{
		if(command[g] == '>' && command[g-1] == '1')
		{
			if(g-2 < 0 || command[g-2] == ' ')
			{
				memset(output,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,output);
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
			}
			else{
				printf("%s\n", "Invalid Command wrong redirection");
				return;
			}
		}
		else if(command[g] == '>' && command[g-1] == '2')
		{
			if(g-2 < 0 || command[g-2] == ' ')
			{
				memset(error,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,error);
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
			}
			else{
				printf("%s\n", "Invalid Command wrong redirection");
				return;
			}
		}
		else if(command[g]=='>' && command[g-1] == '0')
		{
			if(g-2 < 0 || command[g-2] == ' ')
			{
				memset(input,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,input);
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
			}
			else{
				printf("%s\n", "Invalid Command wrong redirection");
				return;
			}
		}
		else if(command[g] == '<' && (command[g-1] == '2' || command[g-1] == '1' || command[g-1] == '0'))
		{
			printf("%s\n", "Invalid Command wrong redirection");
			return;
		}
	}

	int i = -1;
	i = removeSpacing(command, i, lengthString);
	i = findAttribute(i+1, command, executable);

	while(i<lengthString)
	{

		if(command[i] == '-'){
			if(i+1 >= lengthString || invalidHyphen == 1 || command[i+1] == '-')
			{
				printf("%s\n", "Invalid Command");
				return;
			}
			i = findAttribute(i+1, command, attribute);
		}
		else if(command[i] == '>')
		{
			invalidHyphen = 1;
			i = removeSpacing(command, i, lengthString);

			if(i+1 >= lengthString) {
				printf("%s\n", "Invalid Command");
				return;
			}
			if(command[i+1] == '>')
			{
				isappend = 1;
				i+=1;
			}
			else {
				memset(output,'\0', 1000*sizeof(char));
				i = findAttribute(i+1, command, output);
			}
		}
		else if(command[i] == '<')
		{
			invalidHyphen = 1;
			i = removeSpacing(command, i, lengthString);

			if(i+1 >= lengthString || command[i+1] == '<') {
				printf("%s\n", "Invalid Command");
				return;
			}
			else {
				memset(input,'\0', 1000*sizeof(char));
				i = findAttribute(i+1, command, input);
			}
		}
		else if(command[i] == ' ')
		{
			i+=1;
		}
		else {
			i = findAttribute(i, command, arguments[numArguments]);
			numArguments++;
		}
	}

	printf("output = %s\n", output);
	printf("input = %s\n", input);
  printf("error = %s\n", error);
	printf("#attributes = %s\n", attribute);
	printf("executable = %s\n", executable);
	for(int i=0; i<numArguments; ++i) {
		printf("arguments = %s\n", arguments[i]);
	}
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
