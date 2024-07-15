#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BUFFER_SIZE 100



int main(void)
{
    close(2);
    dup(1);
        char command[BUFFER_SIZE];
        char   history[BUFFER_SIZE][BUFFER_SIZE];                   /*array have all the commands*/
        int history_count=0;                                        /*counter to know the number of the current command*/


    while (1)
    {

        fprintf(stdout, "my-shell> ");
        memset(command, 0, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE, stdin);
        if(strncmp(command, "exit", 4) == 0)
        {
            break;
        }


        char *command_line[BUFFER_SIZE];                            /*array to save the current command*/
        char *temp = NULL;
        pid_t pid ;                                                 /*to save what the fork return*/
        int background=0;                                           /* if (1) run in background*/
        int i=0;
        int k=2;


        strcpy(history[history_count],command);                         /*save the command in the history array*/
        history_count++;


        if ((command[(strlen(command))-k  ]=='&')||(command[(strlen(command))-k  ]==' '))
        {
            while( (command[(strlen(command))-k]==' '))
            {
                    k++;
            }

            if(strncmp(command, "history", 7) != 0)
            {
                if (command[(strlen(command))-k  ]=='&')
                {
                    background=1;
                    command[(strlen(command))-k  ]=' ' ;
                }
            }
        }


        if(strncmp(command, "history", 7) == 0)
        {
            int j=history_count;
            while (j!=0)
            {
                printf("%d %s",j,history[j-1]);
                j--;
            }
        }


        else
        {
            pid=fork();

            if(pid==-1)
            {
                perror("error");
            }


            else if(pid==0)
            {

                temp = strtok(command, " \n");
                while (temp != NULL)
                {
                    command_line[i]= temp;
                    i++;
                    temp = strtok(NULL, " \n");
                }
                command_line[i]= NULL;
                execvp(command_line[0], command_line);
                perror("error");

                exit(1);   /*because if we got error ,to end the code*/

            }


            else
            {
                if (background==0)
                {
                    waitpid(pid,NULL,WUNTRACED);
                }
            }
            background=0;

        }

    }

    return 0;
}

