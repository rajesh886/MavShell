/*
  Name - Rajesh Rayamajhi
  
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports five arguments

#define NUM_HISTORY 15          //Maximum number of history that shell support is 15.

static void handle_signal (int sig){} //signal handler function that passes SIGINT signal

static void handler(int sig){}   //signal handler function that passes SIGTSTP signal

int main()
{
  int pidlist[15] = { };          //this array will store the pids of the last 15 processes.

  int m=0;                        //variable m runs to print the history commands stored in historylist.

  int semi_col_count=0;          /*variable semi_col_count keeps track of the number of semi columns in the input given by user so we can know
                                  th number of times we need to fork.*/

  int last_pid_value=0;           //this stores the last pid value that was forked.

  int pid_index,j,hist_index=0;   /*variable pid_index keeps track of the number of pids,
                                  variable j runs to print the pids of the last 15 processes in a loop
                                  variable hist_index keeps track of the number of history */


  char historylist[NUM_HISTORY] [40]= {}; //this array will store the history list of the last 15 commands entered by the user.


  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  //this part of code passes SIGINT signal when user presses ctr-c. It swallows the SIGINT signal.
  struct sigaction act;
  memset (&act, '\0', sizeof(act));
  act.sa_handler = &handle_signal;
  sigaction(SIGINT , &act, NULL);

  //this part of code swallows SIGTSTP signal when user presses ctr-z.
  struct sigaction act1;
  memset(&act1, '\0', sizeof(act1));
  act1.sa_handler = &handler;
  sigaction(SIGTSTP, &act1, NULL);



  while( 1 )
  {
    char sequence[10][20]={};     /* this array keeps the commands seperated by ; for example
                                      ls ; pwd ls will be stored in sequence[0] and pwd will be
                                      stored in sequence[1]. */
    int q=0;


    //Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input


    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );


    //history code
    strcpy(historylist[hist_index],cmd_str);       //it copies the command entered by the user to historylist array including history itself.
    hist_index++;
    if(hist_index==15)                            //hist_index initializes to zero to keep track of the last 15 commands entered by the user.
    {
      hist_index=0;
    }
    char first_element = cmd_str[0];           //keeping the first letter of command to compare with '!' character to run the history command.
    char second_element[3];                   //making an array of string to store the number entered by the user.

    //copying the number entered by the user to the second_element array.
    second_element[0]=cmd_str[1];
    second_element[1] = cmd_str[2];
    second_element[2]='\0';

    if(first_element-'!'==0 )
    {
      int index=atoi(second_element);   //converting the string number to the integer value so that integer value is the hist_index

        cmd_str=historylist[index];    /*this will run the history command by taking the integer value entered by
                                                 the user and using that value as the index of the historylist array. This will replace the
                                           cmd_str by the command stored in the historylist array. */

        if(strlen(cmd_str)==0){          /*this will check if the command exists or not in the history list.
                                              if the command doesn't exist it will return '!' as the first element in the cmd_str so by
                                              comparing again with '!', we can find out if the command exists or not in the history.*/
          printf("command not in history.\n");
          continue;
        }


    }

    for(q=0;q<(strlen(cmd_str)-1);q++)   //this counts the number of semi column if it is present in the given command.
    {
      if(cmd_str[q]-';'==0)
      {
        semi_col_count++;
      }
    }


    //code that runs the commands in sequence separated by ;
    int n,p=0;          //varaible n runs through the loop to check if there is a semi column in the given input of user
                        //variable p is used as index to store the commands in the sequence array.

    for(n=0;n<(strlen(cmd_str)-1);n++)
    {
      if(cmd_str[n]-';'==0)
      {
        semi_col_count++;
        char *tok_string=strtok(cmd_str,";");   //parsing the input seperated by semi columns
        while(tok_string!=NULL)
        {

          if(tok_string[0]==32)               /*if the space is present before the command, it parses the input.ascii value of space is 32.*/
          {
            int len=strlen(tok_string);
            strncpy(tok_string,&tok_string[1],len); //this copies the command after the space to the end of the null character.
          }
          strcpy(sequence[p],tok_string);   //copying tokenized commands into the sequence array.
          p++;
          tok_string = strtok(NULL, ";");
        }

      }
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
      token_count++;
    }


    if(token[0]==NULL)   //typing a blank line stores null value in token[0] and continue statement takes back to the while loop again.
    {
      continue;
    }

    else if((strcmp(token[0], "quit") == 0) || (strcmp(token[0], "exit") == 0))  //quit or exit is stored in token[0] after parsing the input.
    {
      exit(0);
    }

    else if((strcmp(token[0],"cd")==0))
    {
      chdir(token[1]);
    }

    else if((strcmp(token[0],"listpids")==0))
    {
      for(j=0;j<15;j++)
      {
      if(pidlist[j]!=0){                  //if the array pidlist doesn't contain pid of the process, it doesn't print because initial value of array is zero.
        printf("%d: %d\n",j,pidlist[j]);
      }
      }
    }

    else if((strcmp(token[0],"history")==0))
    {
        for(m=0; m<15; m++)
        {
          if(strlen(historylist[m])!=0 )    //if the historylist array doesn't contain the history command, it doesn't print because initial value of array is zero.
          {
            printf("%d: %s",m,historylist[m]);
          }
        }
    }

    else if((strcmp(token[0],"bg")==0))
    {
      kill(last_pid_value,SIGCONT);      //this passes SIGCONT signal to the last pid process to run in background.
    }

    else if((semi_col_count>0))         //If the semi column is present in the given command,it runs this statement.
    {
      int param_index=0;              //index to run through the sequence array.
      for(param_index=0;param_index<=semi_col_count;param_index++)   /*loop and semi column count is used to determine the number of fork. if there is
                                                                    one semicolumn in the given sequence commans we have to fork two times to run two seperate
                                                                    commands. so param_index is run from index 0 to the number of semicolumn to fork multiple times.*/
      {
      //reusing the same code from the above to parse the command input.
      char *token[10];

      int token_count = 0;

      // Pointer to point to the token
      // parsed by strsep
      char *arg_ptr;

      char *working_str  = strdup( sequence[param_index] );

      // we are going to move the working_str pointer so
      // keep track of its original value so we can deallocate
      // the correct amount at the end
      char *working_root = working_str;

      // Tokenize the input stringswith whitespace used as the delimiter
      while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
                (token_count<MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
        {
          token[token_count] = NULL;
        }
        token_count++;
      }

      //reusing the code from below to fork() and create a child process.
      pid_t pid = fork();
      int status;
      if(pid == 0)
      {
          execvp(token[0],token);
          exit(0);

      }
      pidlist[pid_index]=pid;                        //it is saving the pids of the process that is forked in an array pidlist
      last_pid_value=pid;                    //it keeps track of the last pid that was forked.
      pid_index++;
      if(pid_index==15)
      {
        pid_index=0;
      }
      waitpid(pid, &status, 0);
    }

  }

    else
    {
    pid_t child_pid = fork();
    int status;
    if(child_pid == 0)
    {
       if((execvp(token[0],token))==-1)             //if the command is not found, the return value of execvp is -1.
      {
        char *str=strtok(cmd_str,"\n");             //removing the '\n' or newline character that fgets have in cmd_str.
        printf("%s: Command not found.\n",str);
        exit(0);
      }
      else
      {
        execvp(token[0],token);
        exit(0);
      }
    }
      pidlist[pid_index]=child_pid;                        //it is saving the pids of the process that is forked in an array pidlist
      last_pid_value=child_pid;                    //it keeps track of the last pid that was forked.
      pid_index++;                                  //it increases the index of pidlist so new pid can be stored in different array index.
      if(pid_index==15)
      {
        pid_index=0;
      }
    waitpid(child_pid, &status, 0);
  }
  semi_col_count=0;
  free( working_root );
 }
  return 0;
}
