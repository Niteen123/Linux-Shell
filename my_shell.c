#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <unistd.h>
#include<stdlib.h>
#include<signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 64
#define MAX_TOKEN_LEN 64
int processes_queue[64],counter=0;
char **tokens;
int fore_process=-1;

void directory(char **tokens)
{
    int res=chdir(tokens[1]);
    if(res!=0)
    {
        printf("Shell: Incorrect command");
    }
}
void forker(char **tokens,int n)
{
    int fc=fork();
    if(fc<0)
    {
        fprintf(stderr,"%s\n","Child process not created");
        exit(1);
    }
    else if(fc==0)
    {
        execvp(tokens[0],tokens);
        printf("exec failed");
        exit(1);
    }
    if(fc>0 && n==1)
    {
        if(setpgid(fc,0)!=0)
            printf("setpgid failed");

        processes_queue[counter++]=fc;
    }
    else if(fc>0 && n==0)
    {
        if(setpgid(fc,0)!=0)
            printf("setpgid failed");
        fore_process=fc;
        siginfo_t SignalInfo;
        int wc=waitid(P_PID,fc, &SignalInfo, WEXITED);
    }
}
/*void foreground(char **tokens)
{
    int fc=fork();
    if(fc<0)
    {
        fprintf(stderr,"%s\n","Child process not created");
        exit(1);
    }
    else if(fc==0)
    {
        
        execvp(tokens[0],tokens);
        printf("exec failed");
        exit(1);
    }
    else
    {
        siginfo_t SignalInfo;
        int wc=waitid(P_PID,fc, &SignalInfo, WEXITED);
    }
}*/
void **tokenize(char *input,char **tokens)
{
	
  	char *token = (char *)malloc(MAX_TOKEN_LEN * sizeof(char));
  	int i, tokenIndex = 0, tokenNo = 0;
  	
  	for(i=0;i<strlen(input);i++)
    {
        char readChar=input[i];
        if(readChar==' ' || readChar == '\n' || readChar == '\t')       //checking whether there is space or new line or tab space so that we can create a new token
        {
            token[tokenIndex]='\0';
            if (tokenIndex !=0)                                        // if not done then we would be inserting a new empty token 
            {
                tokens[tokenNo]=(char*)malloc(MAX_TOKEN_LEN*sizeof(char));
                strcpy(tokens[tokenNo++],token);
                tokenIndex=0;
            }
        }
        else
        {
            token[tokenIndex++]=readChar;
        }
    }
    free(token);
    tokens[tokenNo]=NULL;
}

void freeer(char **tokens)
{
    int i;
    for(i=0;tokens[i]!=NULL;i++)
        {
            free(tokens[i]);
        }
        free(tokens);
}

void handler(int sig)
{
    if(fore_process!=-1)
    {
        kill(fore_process,SIGKILL);
    }    
}
void reap()
{
    /*if processid turns out to be greater than 0 means alteast 1 process is zombie so check for all the 
    stored the processes to know which one's are zombies and then clean them*/ 
    int i;
    pid_t processid=waitpid(-1,NULL,WNOHANG);   //	WNOHANG menas return immediately if no child has exited.
        for(i=0;i<counter;i++)
        {
            if(processes_queue[i]==processid)
            {
                processes_queue[i]=-2;
                printf("\nShell: Background process finished"); 
                processid=waitpid(-1,NULL,WNOHANG);
            }
        } 
}

int main()
{
    char  input[MAX_INPUT_SIZE];                      
	int i;
    
    while(1)
    {
        signal(SIGINT,handler);
        fore_process=-1;
        reap();
        bzero(input,sizeof(input));
        printf("\n$");
        scanf("%[^\n]",input); 
        getchar();
        
        tokens = (char **)malloc( MAX_INPUT_SIZE * sizeof(char *));  

		input[strlen(input)] = '\n'; //terminate with new line
        tokenize(input,tokens);
        
        if (tokens[0]==NULL)
        {
            freeer(tokens);
            continue;
        }
        else if (!strcmp(tokens[0],"exit"))
        {
            for(i=0;i<counter;i++)
            {
                if(processes_queue[i]>0)
                {
                    kill(processes_queue[i],SIGKILL);
                }
            }
            freeer(tokens);
            exit(0);     
        }  
        else if (!strcmp(tokens[0],"cd"))
        {
            directory(tokens);     
        }
        else
        {
            i=0;
            while(tokens[i]!=NULL)
            {
                i++;
            }
            i--;
            if (!strcmp(tokens[i],"&"))
            {
                free(tokens[i]);
                tokens[i]=NULL;
                forker(tokens,1);   
            }
            else
            {
                forker(tokens,0);
            }  
        }
        freeer(tokens);
    }
return 0;
}
