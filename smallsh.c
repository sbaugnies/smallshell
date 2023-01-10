// Name: Sylvain Baugnies
// Class: CS344
// Assignment 3: smallsh
// *

//Much of this code is from my previous submission in the Fall of 2021. Updates are based off of module explorations and class discussions via Ed and Discord.

// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <sys/wait.h>



int   statusVar;
int *exitStatus;


/* struct for command information*/
struct commandStruct
{

	char *command;
	char *arguments[512];		// [512] ?
	char *inputFile;
	char *outputFile;
	char *fileDestination;
	char *backgroundStatus;
  //int  argIteration;
  //int  inputArgumentIndex;
  //int  outputArgumentIndex;
};

struct commandStruct* parseCommand(char *currentCommand) {

	struct commandStruct* currentParsedCommand = malloc(sizeof(struct commandStruct));

	//Command input string method below sourced from link 2
	char* firstCommand;
	char* saveptr;
	int inputLength = strlen(currentCommand);
	char* inputCopy = (char*)calloc(inputLength + 1, sizeof(char));
	strncpy(inputCopy, currentCommand, inputLength);


	char *token = strtok_r(inputCopy, " ", &saveptr);
	currentParsedCommand->command = calloc(strlen(token) + 1, sizeof(char));
	currentParsedCommand->arguments[0] = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currentParsedCommand->command, token);
	strcpy(currentParsedCommand->arguments[0], token);


	int	argIteration = 1;
  int inputArgumentIndex = 0;
  int outputArgumentIndex = 0;


	token = strtok_r(NULL, " ", &saveptr);
	while (token != NULL) {
		// Checks if current strtok_r parse is < input file
		if ((strcmp(token, "<")) == 0) {
			token = strtok_r(NULL, " ", &saveptr);		//
      //Allocates and places < token in currentParsedCommand->inputFile to flag as input
			currentParsedCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currentParsedCommand->inputFile, token);
      token = strtok_r(NULL, " ", &saveptr);
      }


		// Checks if current strtok_r parse is > output file
		else if ((strcmp(token, ">")) == 0) {
			token = strtok_r(NULL, " ", &saveptr);		//
      //Allocates and places > token in currentParsedCommand->outputFIle to flag as output
			currentParsedCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currentParsedCommand->outputFile, token);
      token = strtok_r(NULL, " ", &saveptr);
		}

		// Else parses, allocates and copies into arguments string
		else {
			currentParsedCommand->arguments[argIteration] = calloc(strlen(token) + 1, sizeof(char));
			strcpy(currentParsedCommand->arguments[argIteration] , token);
			token = strtok_r(NULL, " ", &saveptr);
			++argIteration;
		}
	}


	return currentParsedCommand;
}

//runCommand function analyzes and runs arguments provided from commandStruct
void runCommand(struct commandStruct* myCommand) {

	const char* statusChecker = "status";
	const char* cdChecker = "cd";
	const char* exitChecker = "exit";
	const char* poundChecker = "#";
	const char* blankChecker = " ";
	int programWhileStatus = 1;       //Used as flag to keep while loop running until exit is called

	char *cdDestination;


	//Checks if input was "exit" command, sets programWhileStatus flag to 0 and exits if so
	if ((strcmp(exitChecker, myCommand->command)) == 0) {
		programWhileStatus = 0;
		exit(0);
	}

	//Checks if input was "status" command, determining status from WEXISTATUS and setting global exitState variable to 1 or 0 depending on returned status
	if ((strcmp(statusChecker, myCommand->command)) == 0) {
    if (WEXITSTATUS(&exitStatus) != 0) {
    statusVar = 1;}
    else {
    statusVar = 0;}
    //int statusVar = WEXITSTATUS(&exitStatus);
		printf("exit value %i\n", statusVar);
    fflush(stdout);
    return;
	}

	//cd code based on link 6
  //Checks if input was "cd" command, returning to home directory if no specific directory provided
	if ((strcmp(cdChecker, myCommand->command)) == 0) {
		cdDestination = myCommand->arguments[1];
		if (cdDestination == NULL) {
			cdDestination = getenv("HOME");
			chdir(cdDestination);
		}
		else {
			if ((chdir(cdDestination)) == -1) {
				printf("%s: no such file or directory", cdDestination);
				fflush(stdout);

			}
			else {
				chdir(cdDestination);
			}
		}
		//code to test cd
		int MAX_BUF = 200;
		char path[MAX_BUF];
		getcwd(path, MAX_BUF);
		//printf("Current working directory: %s\n", path);	//test, comment out after implementation
	}
	
	//Checks if input was any other command besides exit, status or cd
	//Code based on module page "Exploration: Process API - Monitoring Child Processes"
	else {
		//printf("This is where the fork begins");
		pid_t spawnPid = fork();
		int childStatus;
		int childPid;
    int sourceFD;
    int targetFD;
    int result;


		switch (spawnPid) {
		case -1:
			perror("fork() failed!");
			fflush(stdout);
			exit(1);
			break;

		case 0:
      //Checks if an inputFile was parsed into commandStruct, opening/routing the source file as necessary
      if (myCommand->inputFile) {
      //File redirection code taken from "Processes and I/O" Exploration
      
      //Open source file:
      sourceFD = open(myCommand->inputFile, O_RDONLY);
      if (sourceFD == -1) {
        perror("source open()");
        //statusVar = 1;
        exit(1);
      }

      
      //Redirect stdin to source file
      result = dup2(sourceFD, 0);
      if (result == -1) {
        perror("source dup2()");
        exit(2);
      }
      
      fcntl(sourceFD, F_SETFD, FD_CLOEXEC);     //Closes sourceFD file that was opened.
      //break;
      }      


      //Checks if an outputFile was parsed into commandStruct, opening/routing the source file as necessary
      if (myCommand->outputFile) {
      //File redirection code taken from "Processes and I/O" Exploration

      //Open target file
      targetFD = open(myCommand->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);   //Is this 0644 code wrong?
      if (targetFD == -1) {
        perror("target open()");
        //statusVar = 1;
        exit(1);
      }
      

      // Redirect stdout to target file
	    result = dup2(targetFD, 1);
      if (result == -1) { 
		    perror("target dup2()");
		    exit(2);
      }
      
      
      fcntl(targetFD, F_SETFD, FD_CLOEXEC);       //Closes targetFD file that was opened.
      //break;
      }     


			//Runs execvp on the first command argument provided
		  if (execvp(myCommand->arguments[0], myCommand->arguments)) {
      //perror line below is triggered by an invalid command input
     	  perror(myCommand->arguments[0]);
      //statusVar = 1;
      //kill(spawnPid, statusVar);
		    exit(2);
		  }
			//break;
      /////exit(0);

    	default:
			// spawnpid is the pid of the child
			childPid = wait(&childStatus);

      //fflush(stdout);
      //sleep(10);
      
      spawnPid = waitpid(spawnPid, &childStatus, 0);
      //printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);   //delete later
      //fflush(stdout);                      
      // Maybe this would be a good spot to check if currentParsedCommand->inputFile or outputFile exist,
      // then process them?
      //exit(0);
			break;
		}



	}


}




int main() {
	const int maxLineLength = 2048;		// Max length of characters per command
	const int maxArgs = 512;			// Max amount of arguments per command


	char currentCommand[maxLineLength];
	const char * statusChecker = "status";
	const char * cdChecker = "cd";
	const char * exitChecker = "exit";
	const char * poundChecker = "#";
	const char * blankChecker = " ";
	int programWhileStatus = 1;		// Probably change this to handle "exit" command later on
                                //
  exitStatus = 0;


	// Prompt loop for user input
	while (programWhileStatus != 0) {
		printf(": ");
		fflush(stdout);
		fgets(currentCommand, maxLineLength, stdin);
		// Checks if current command is empty or a comment
		if ((strlen(currentCommand) == 1) || ((strcmp(poundChecker, currentCommand)) == 0)) {
			//printf
			continue;
		}
		currentCommand[strcspn(currentCommand, "\n")] = 0;
		//size_t len = strlen(currentCommand);			// Sourced from link 1
		//if (len > 0 && currentCommand[len -1] == '\n')
		//	currentCommand[--len] = '\0';



		// Section below expands $$ variables found in command input
		// Contains code from Ed Discussion post and Link 3 & 4 & 5
		pid_t pid = getpid();					// PID for $$ Variable Expansion
		char* curPid = malloc(6);				//Allocates space for program id
		sprintf(curPid, "%d", pid);				//Sets pid to char* variable
		int length = strlen(currentCommand);	//Sets limit based on length of currentCommand
		char expandedTempCommand[maxLineLength];	//= strndup(currentCommand, maxLineLength);
		char* tempCharCopy;

		int i = 0;
		int currIndex = 0;		//keeps track of non $ iterations

		while (i < length) {
			// If the iterated i character is not $
			if (currentCommand[i] != '$') {

				expandedTempCommand[currIndex] = currentCommand[i];
				i++;
				currIndex++;
			}
			// If the iterated i character is $$
			if (currentCommand[i] == '$' && currentCommand[i + 1] == '$') {
				strcat(expandedTempCommand, curPid);
				i += 2;	
				currIndex += strlen(curPid);	
				
			
			}

		}
		

		//Command input string method below sourced from link 2
		char* firstCommand;
		char* saveptr;
		int inputLength = strlen(expandedTempCommand);
		char* inputCopy = (char*)calloc(inputLength + 1, sizeof(char));
		strncpy(inputCopy, currentCommand, inputLength);

		firstCommand = strtok_r(inputCopy, " ", &saveptr);

		//Checks if the first argument that was parsed in commandStruct is blank/space or a # comment; continues to next prompt if so
    if ((strcmp(blankChecker, currentCommand) == 0) || (currentCommand[0] == '#') || (strcmp(poundChecker, currentCommand) == 0)) {
      		//Free up everything:
		      //memset clears out expandedTempCommand & currentCommand
		      memset(expandedTempCommand, 0, maxLineLength);
		      memset(currentCommand, 0, maxLineLength);
          ////memset(firstCommand, 0, maxLineLength);
		      //free() arguments inside struct
		      free(inputCopy);
          ////free(expandedTempCommand);
		      int iClear = 0;
		      ////free(currentCommand);
          //free(firstCommand);
          //printf("all cleared\n");
		      fflush(stdout);;
          continue;}
    //Calls parseCommand with the expandedTempCommand variable (expanded with any $$ that may have been expanded into PIDs)
		struct commandStruct* myCommand = parseCommand(expandedTempCommand);
		
    
    //Calls up runCommand
		runCommand(myCommand);
    

	  //Free up everything:
		//memset clears out expandedTempCommand & currentCommand
		memset(expandedTempCommand, 0, maxLineLength);
		memset(currentCommand, 0, maxLineLength);

    //The following free() calls are no longer required.
		//free() arguments inside struct
		//free(myCommand->command);
		//int iClear = 0;
		/*while (myCommand->arguments[iClear]) {
    printf("This is myCommand->arguments[iClear]: %s\n", myCommand->arguments[iClear]);
    fflush(stdout);
			free(myCommand->arguments[iClear]);
			iClear++;
		}
		//free(myCommand->inputFile);     
		//free(myCommand->outputFile);
		////free(myCommand->fileDestination);
		free(myCommand);
    //printf("all cleared\n");
    free(firstCommand);
		*/
    fflush(stdout);


	}

	return EXIT_SUCCESS;
}







//Sources:
//
//
//
// Link 1: https://cboard.cprogramming.com/c-programming/178471-unwanted-line-after-printing-string-console-also-writing-file.html
//
// Link 2: https://stackoverflow.com/questions/1556616/how-to-extract-words-from-a-sentence-efficiently-in-c/1556650
//
//
// Link 3: https://stackoverflow.com/a/3213868

// Link 4: https://edstem.org/us/courses/14269/discussion/773207?comment=1794568
//
// Link 5: https://stackoverflow.com/a/53230284
//
// Link 6: https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
