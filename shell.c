#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<limits.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
char cmdhistory[1000][1000];//stores the commands used
 int ncmd=0;//keeps a count of the number of commands executed
char executable[1000], attribute[1000];
char output[1000], error[1000], input[1000];
char arguments[100][1000];
int isappend = 0, numArguments = 0, inputind = -1, outputind = -1, errorind = -1;

/*
	Splits the array with Type = {">", "<", " "}.
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

char* getPath(char* string)
{
	char *actualPath;
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
		int checkDirectory=chdir(nowPath);
		if(checkDirectory<0)
		{
			printf("Invalid Path");
			return "";
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
	strcat(actualPath,"\0");
	return actualPath;
}

void checkfile(int fileno,char *path)
{
	if( fileno == -1)
	{
			FILE * fPtr;
			fPtr = fopen(path,"w");
			if(fPtr == NULL)
    	{
        /* File not created hence exit */
        printf("Unable to create file.\n");
    	}
			fclose(fPtr);
	}
}

int inputfd(char *input)
{
	if(strlen(input) > 0)
	{

		if(input[0] == '&')
		{
			if(strlen(input)==2 && (input[1] == '1' || input[1] == '2' || input[1] == '0'))
			{
				if(input[1] == '1') {
					close(0);
					dup(1);
				}
				else if(input[1] == '2') {
					close(0);
					dup(2);
				}
				return 1;
			}
			else
			{
				printf("%s\n", "Bad File Descriptor.");
				return 0;
			}
		}

		char *inputPath;
		inputPath = getPath(input);
		if(strcmp(inputPath,"\0") == 0){
			printf("%s\n","Path is Invalid " );
			return 0;
		}
		else{
			int file = open(inputPath, O_RDONLY);
			if(file != -1) {
				close(0);
				dup(file);
			}
			else {
				printf("%s\n", "File Doesn't Exist");
				return 0;
			}
		}
	}
	return 1;
}

int outputfd(char *output, int isappend)
{
	if(strlen(output) > 0)
	{
		if(output[0] == '&')
		{
			if(strlen(output)==2 && (output[1] == '1' || output[1] == '2' || output[1] == '0'))
			{
				if(output[1] == '0') {
					close(1);
					dup(0);
				}
				else if(output[1] == '2') {
					close(1);
					dup(2);
				}
				return 1;
			}
			else
			{
				printf("%s\n", "Bad File Descriptor.");
				return 0;
			}
		}

		char* outputPath;
		outputPath = getPath(output);
		if(strcmp(outputPath,"\0") == 0){
			printf("%s\n","Path is Invalid " );
			return 0;
		}
		else{
			int file;
			if(isappend == 1) {
				file = open(outputPath,O_WRONLY | O_APPEND);
				checkfile(file, outputPath);
				file = open(outputPath,O_WRONLY | O_APPEND);
			}
			else {
				remove(outputPath);
				file = open(outputPath,O_WRONLY);
				checkfile(file, outputPath);
				file = open(outputPath,O_WRONLY);
			}

			close(1);
			dup(file);
		}
	}
	return 1;
}

int errorfd(char *error)
{
	if(strlen(error) > 0)
	{

		if(error[0] == '&')
		{
			if(strlen(error)==2 && (error[1] == '1' || error[1] == '2' || error[1] == '0'))
			{
				if(error[1] == '0') {
					close(2);
					dup(0);
				}
				else if(error[1] == '1') {
					close(2);
					dup(1);
				}
				return 1;
			}
			else
			{
				printf("%s\n", "Bad File Descriptor.");
				return 0;
			}
		}

		char* errorPath;
		errorPath = getPath(error);
		if(strcmp(errorPath,"\0") == 0){
			return 0;
		}
		else{
			int file = open(errorPath,O_WRONLY);
			checkfile(file, errorPath);
			file = open(errorPath,O_WRONLY);
			close(2);
			dup(file);
		}
	}
	return 1;
}

int specialCommand()
{
	if(strcmp(executable,"cd") == 0)
	{
			int ch = chdir(arguments[0]);
			if(ch < 0)
			{
				printf("Wrong path specified.Check the Path again\n");
			}
			return 1;
	}

	if(strcmp(executable, "clear") == 0)
	{
		printf("\e[1;1H\e[2J");
		return 1;
	}

	if(strcmp(executable, "history") == 0) {
		if(ncmd == 0) {
			printf("%s\n", "No Commands Found");
		}
		else {
			for(int i=0; i<ncmd; ++i) {
				printf("%d %s\n",i+1,cmdhistory[i]);
			}
		}
		return 1;
	}
	return 0;
}

void execute()
{
		if(inputind < outputind && inputind < errorind){
			int val = inputfd(input);
			if(val == 0) return;
			if(outputind<errorind){
				val = outputfd(output, isappend);
				if(val == 0) return;
				val = errorfd(error);
				if(val == 0) return;
			}
			else{
				val = errorfd(error);
				if(val == 0) return;
				val = outputfd(output, isappend);
				if(val == 0) return;
			}
		}
		else if(outputind < inputind  && outputind < errorind){
			int val = outputfd(output,isappend);
			if(val == 0) return;
			if(inputind<errorind){
				val = inputfd(input);
				if(val == 0) return;
				val = errorfd(error);
				if(val == 0) return;
			}
			else{
				val = errorfd(error);
				if(val == 0) return;
				val = inputfd(input);
				if(val == 0) return;
			}
		}
		else
		{
			int val = errorfd(error);
			if(val == 0) return;
			if(inputind < outputind){
					val = inputfd(input);
				if(val == 0) return;
				val = outputfd(output, isappend);
				if(val == 0) return;
			}
			else{
				val=outputfd(output,isappend);
				if(val == 0) return;
			  val = inputfd(input);
				if(val == 0) return;
			}
		}

		if(strcmp(executable,"") == 0) exit(0);

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
			for(int j=0; j<numArguments; j++) {
				args[i] = arguments[j];
				++i;
			}
		}

		int check = execvp(args[0],args);



		memset(args[0],'\0', 1000*sizeof(char));
		strcat(args[0], "/usr/bin/");
		strcat(args[0], executable);

		int check2 = execvp(args[0],args);

		if(check < 0 || check2 < 0) {
					printf("%s\n", "Exec command failed.");
		}
		exit(1);

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
	memset(executable,'\0', 1000*sizeof(char));
	memset(attribute,'\0', 1000*sizeof(char));
	memset(input,'\0', 1000*sizeof(char));
	memset(output,'\0', 1000*sizeof(char));
	memset(error,'\0', 1000*sizeof(char));
	for(int i=0;i<100;++i) {
		memset(arguments[i],'\0', 1000*sizeof(char));
	}
	isappend = 0;
	numArguments = 0;
	inputind = -1;
	outputind = -1;
	errorind = -1;

	int lengthString = strlen(command);

	for(int g=1;g<lengthString;g++)
	{
		if(command[g] == '>' && command[g-1] == '1' && (g-2 < 0 || command[g-2] == ' '))
		{
				memset(output,'\0', 1000*sizeof(char));
				int fstart;
				if(command[g+1] == '>') {
					isappend = 1;
					fstart = removeSpacing(command,g+1,lengthString);
				}
				else {
					fstart = removeSpacing(command,g,lengthString);
				}

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
				int fstart;
				if(command[g+1] == '>') {
					isappend = 1;
					fstart = removeSpacing(command,g+1,lengthString);
				}
				else {
					fstart = removeSpacing(command,g,lengthString);
				}
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
				continue;
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
}

void multiplePipe(char **cmd)
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
  {
			// printf("%s\n", *cmd);
      pipe(p);
			int pipe_sz = fcntl(p[1], F_SETPIPE_SZ, 1000000);
      if ((pid = fork()) == -1)
      {
					printf("%s\n", "Cannot fork child process!!");
          exit(0);
      }
      else if (pid == 0)
      {
          dup2(fd_in, 0); //change the input according to the old one
          if (*(cmd + 1) != NULL)
            dup2(p[1], 1);
          close(p[0]);
          determineExec(*cmd);
					execute();
      }
      else
      {
        close(p[1]);
        fd_in = p[0]; //save the input for the next command
				wait(NULL);
        cmd++;
      }
  }
}

/*
	Splits the array with "|".
*/
void splitByPipe(char *line)
{
	char *array[1000];
	for(int i=0;i<100;++i) {
		array[i] = '\0';
	}
	int elements = splitByType(line, array, "|");

	determineExec(array[0]);
	int check = specialCommand();



	if(elements != 1 || check == 0) {
		multiplePipe(array);
	}
}

/*
	Function to take input from stdin
*/
void Input()
{
	// taking input
	printf("\e[36;1m~%s>$ \e[0m", getcwd(NULL,0));
	char* command=malloc(sizeof(char)*1000);
	size_t n = 1000;
	getline(&command, &n, stdin);

	if(strcmp(command,"\n")==0)
		return;
	command[strlen(command)-1]='\0';

	strcpy(cmdhistory[ncmd],command);
	ncmd++;

	if(strcmp("exit", command) == 0) {
		printf("%s","\e[1;1H\e[2J");
		exit(0);
	}
	for(int i=0; i<strlen(command); ++i) {
			if(command[i] == (char)34 || command[i] == (char)39)
				command[i] = ' ';
	}
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

void main()
{
	signal(SIGINT, sig);
	printf("%s","\e[1;1H\e[2J");
	while(1) {
		Input();
	}
}
