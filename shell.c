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

int getPath(char* string, char actualPath[1000])
{
	char *path[1000];
	char* currentPath = getcwd(NULL,0);
	int elements = splitByType(string,path,"/");

	if(elements > 1)
	{
		char nowPath[1000];
		memset(nowPath,'\0', 1000*sizeof(char));
		for(int h = 0; h < elements-1; h++)
		{
			strcat(nowPath,path[h]);
			strcat(nowPath,"/");
		}
		printf("%s\n",nowPath);
		int checkDirectory=chdir(nowPath);
		printf("%d\n",checkDirectory);
		if(checkDirectory<0)
		{
			printf("Invalid Path");
			return -1;
		}
		actualPath = getcwd(NULL,0);
		strcat(actualPath,"/");
		strcat(actualPath, path[elements-1]);
	}
	else {
		actualPath = getcwd(NULL,0);
		strcat(actualPath,"/");
		strcat(actualPath, path[elements-1]);
	}

	chdir(currentPath);
	printf("%s\n", actualPath);
	return 0;
}

void execute(char *executable,char *input,char *output,char *error,char *attribute,
						int numArguments,char	arguments[100][1000], int isappend)
{
		pid_t pid;
		int status;

		if(strcmp(executable,"cd") == 0)
		{
				int ch = chdir(arguments[0]);
				if(ch < 0)
				{
					printf("Wrong path specified.Check the Path again\n");
				}
				return;
		}


		if(strlen(input) > 0)
		{
			char inputPath[1000];
			memset(inputPath,'\0', 1000*sizeof(char));
			int val = getPath(input,inputPath);
			printf("%d\n", val);
		}

		if(strlen(output) > 0)
		{
			char outputPath[1000];
			memset(outputPath,'\0', 1000*sizeof(char));
			int val = getPath(output,outputPath);
			printf("%d\n", val);
			printf("%s\n", outputPath);
		}

		if(strlen(error) > 0)
		{
			char errorPath[1000];
			memset(errorPath,'\0', 1000*sizeof(char));
			int val = getPath(error,errorPath);
		}

		pid = fork();

		if(pid < 0)
		{
			printf("%s\n", "Cannot fork child process!!");
		}
		else if(pid == 0)
		{
			char *args[1000];
			char command[1000] = "/bin/";
			strcat(command, executable);
			int i = 0;
			args[i] = command;
			++i;

			if(strlen(attribute) > 0){
					char arg1[1000] = "-";
					strcat(arg1,attribute);
					args[i] = arg1;
					++i;
			}

			if(numArguments > 0){
				for(int j=0; j<numArguments; j++){
					args[i] = arguments[j];
					++i;
				}
			}

			int check = execvp(args[0],args);

			if(check < 0) {
				printf("%s\n", "Exec command failed.");
			}
			exit(1);
		}
		else {
			while(wait(&status) != pid);
		}
}

void proceed(char *executable,char *input,char *output,char *error,char *attribute,
						int numArguments,char	arguments[100][1000], int isappend)
{
	printf("output = %s\n", output);
	printf("input = %s\n", input);
	printf("error = %s\n", error);
	printf("#attributes = %s\n", attribute);
	printf("executable = %s\n", executable);
	for(int i=0; i<numArguments; ++i) {
		printf("arguments = %s\n", arguments[i]);
	}
	execute(executable,input,output,error,attribute,numArguments,arguments, isappend);
}

/*
	Function to jump over spaces.
	Returns the index of last space until non-space character is found.
*/
int removeSpacing(char *string, int i, int lengthString)
{
	int j = i+1;
	for(; j< lengthString; ++j) {
		if(string[j] != ' ' && string[j] != '\t') {
			break;
		}
	}
	return (j-1);
}

int findAttribute(int low, char *command, char *string)
{
	int lengthString = strlen(command);
	int n = strlen(string);
	int i = low;
	for(; i<lengthString; ++i)
	{
		if (command[i] == ' ' || command[i] == '>' || command[i] == '<' || command[i] == '\t')
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
	Determines the command that has to be executed.
*/
void determineExec(char command[])
{
	int lengthString = strlen(command);
	char executable[1000], attribute[1000];
	char output[1000], error[1000], input[1000];
	char arguments[100][1000];
	int isappend = 0, numArguments = 0, inputind = -1, outputind = -1, errorind = -1;
	memset(executable,'\0', 1000*sizeof(char));
	memset(attribute,'\0', 1000*sizeof(char));
	memset(input,'\0', 1000*sizeof(char));
	memset(output,'\0', 1000*sizeof(char));
	memset(error,'\0', 1000*sizeof(char));
	for(int i=0;i<100;++i) {
		memset(arguments[i],'\0', 1000*sizeof(char));
	}


	for(int g=1;g<lengthString;g++)
	{
		if(command[g] == '>' && command[g-1] == '1' && (g-2 < 0 || command[g-2] == ' '))
		{
				memset(output,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,output);
				outputind = g;
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
		}
		else if(command[g] == '>' && command[g-1] == '2' && (g-2 < 0 || command[g-2] == ' '))
		{
				memset(error,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,error);
				errorind = g;
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
		}
		else if(command[g]=='>' && command[g-1] == '0' && (g-2 < 0 || command[g-2] == ' '))
		{
				memset(input,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,input);
				inputind = g;
				for(int y=g-1; y<fend; ++y)
				{
					command[y] = ' ';
				}
		}
		/*
		else if(command[g]=='>' && command[g+1] == '>')
		{
			if(g-2 < 0 || command[g-2] == ' ')
			{
				memset(input,'\0', 1000*sizeof(char));
				int fstart=removeSpacing(command,g,lengthString);
				int fend=findAttribute(fstart+1,command,input);
				inputind = g;
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
		*/
		else if(command[g] == '<' && (command[g-1] == '2' || command[g-1] == '1' || command[g-1] == '0'))
		{
			printf("%s\n", "Invalid Command wrong redirection");
			return;
		}
		else if(command[g] == '>' && (command[g-2] == '2' || command[g-2] == '1' || command[g-2] == '0') && (g-3)>=0 && command[g-1] == ' ' && command[g-3] == ' ')
		{
			printf("%s\n", "Invalid Command wrong redirection");
			return;
		}
	}

	int i = -1;
	i = removeSpacing(command, i, lengthString);
	i = findAttribute(i+1, command, executable);

	if(strcmp("exit", executable) == 0) {
		exit(0);
	}

	while(i<lengthString)
	{

		if(command[i] == '-'){
			if(i+1 >= lengthString || command[i+1] == '-')
			{
				printf("%s\n", "Invalid Command");
				return;
			}
			i = findAttribute(i+1, command, attribute);
		}
		else if(command[i] == '>')
		{
			i = removeSpacing(command, i, lengthString);

			if(i+1 >= lengthString || (command[i-1] == '>' && command[i+1] == '>')) {
				printf("%s\n", "Invalid Command");
				return;
			}
			if(command[i+1] == '>')
			{
				isappend = 1;
				i+=1;
			}
			if(i < outputind) {
				char temp[1000] = "";
				memset(temp,'\0', 1000*sizeof(char));
				i = findAttribute(i+1, command, temp);
			}
			else {
				memset(output,'\0', 1000*sizeof(char));
				outputind = i;
				i = findAttribute(i+1, command, output);
			}
		}
		else if(command[i] == '<')
		{
			i = removeSpacing(command, i, lengthString);

			if(i+1 >= lengthString || command[i+1] == '<') {
				printf("%s\n", "Invalid Command");
				return;
			}
			if(i < inputind) {
				char temp[1000] = "";
				memset(temp,'\0', 1000*sizeof(char));
				i = findAttribute(i+1, command, temp);
			}
			else {
				memset(input,'\0', 1000*sizeof(char));
				inputind = i;
				i = findAttribute(i+1, command, input);
			}
		}
		else if(command[i] == ' ' || command[i] == '\t')
		{
			i+=1;
		}
		else {
			i = findAttribute(i, command, arguments[numArguments]);
			numArguments++;
		}
	}

	proceed(executable, input, output, error, attribute, numArguments, arguments, isappend);

	// printf("output = %s\n", output);
	// printf("input = %s\n", input);
  // printf("error = %s\n", error);
	// printf("#attributes = %s\n", attribute);
	// printf("executable = %s\n", executable);
	// for(int i=0; i<numArguments; ++i) {
	// 	printf("arguments = %s\n", arguments[i]);
	// }
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
			// determineExec(array[j]);
		}
	}
}

/*
	Function to take input from stdin
*/
void Input()
{
	// taking input
	printf("\e[36;1m~%s>$ \e[0m", getcwd(NULL,0));
	// char line[1000] = "";
  // char c = getchar();
	// int i = 0;
	// line[i] = c;
  // while (c != '\n' && c != EOF) {
	// 		i += 1;
	// 		c = getchar();
	// 		if(c == '\t') {
	// 			exit(0);
	// 			printf("%s\n", "hi");
	// 			line[i] = ' ';
	// 			line[i+1] = ' ';
	// 			line[i+2] = ' ';
	// 			line[i+3] = ' ';
	// 			i += 3;
	// 		}
	// 		else if(c == '\b'){
	// 			printf("%s\n", "backspace");
	// 			--i;
	// 			if(i<-1) i = 0;
	// 		}
	// 		else {
	// 			printf("%s\n", "normal");
	// 			line[i] = c;
	// 		}
	// 		printf("%s\n", line);
	// }
	// line[i] = '\0';
	char* command=malloc(sizeof(char)*1000);
	size_t n = 1000;
	getline(&command, &n, stdin);
	command[strlen(command)-1]='\0';
	if(strcmp(command,"\n")==0)
		return;
	splitByPipe(command);
}

/*
	Function to handle ^C signal
*/
void sig(int sigi)
{
	int a = 10;
	char b = (char) a;
	signal(SIGINT, sig);
	printf("%c",b);
	Input();
}

void main() {
	setvbuf(stdin, NULL, _IONBF, 0);
	signal(SIGINT, sig);
	while(1) {
		Input();
	}
}
