/* This is a csc60mshell.c
 * This program serves as a skeleton for starting for lab 4.
 * Student is required to use this program to build a mini shell 
 * using the specification as documented in lab 4.
 * Date: 4/28/2016
 * Author(s): Ali H. Iqbal
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXLINE 80
#define MAXARGS 20

void process_input(int argc, char **argv) {

   if(argc >2)
   {
	int i, inPut = 0, outPut = 0, redir = 0;

	//counts redirctions
	
	for( i = 0; i<argc;i++)
	{

	   if(strcmp(argv[i],">")==0)
	   {
		redir++;
	   	outPut = i;
	   }

	   else if(strcmp(argv[i],"<")==0)
	   {
		redir++;
		inPut = i;
	   }
	}

	if(redir>1)
	{
	   printf("\tToo many redirections\n");
	   _exit(3);
	}

	//redirects output
	if(outPut !=0)
	{
	   int fileId = open(argv[outPut+1], O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
	   if(fileId<0)
	   {
		printf("\tError creating file\n");
		_exit(3);
	   }
	   
	   dup2(fileId,1);//file id to stdout.
	   close(fileId);
	   argv[outPut] = NULL;
	}
	
	//redirects input file
	if(inPut !=0)
	{
	   int fileId = open(argv[inPut+1],O_RDONLY,0);
	   
	   if(fileId<0){
		printf("\tError creating file\n");
		_exit(3); 
	   }
	
	   dup2(fileId,0);
	   close(fileId);
	   argv[inPut] = NULL;
	}
   }
   
   //execute command
   if(execvp(argv[0],argv)==-1)
   {
      perror("Shell Program Error");
      _exit(-1);
   }
   else
   	execvp(argv[0],argv);
   
   return;
}


int parseline(char *cmdline, char **argv)
{
  int count = 0;
  char *separator = " \n\t";
  argv[count] = strtok(cmdline, separator);
  while ((argv[count] != NULL) && (count+1 < MAXARGS)) {
   argv[++count] = strtok((char *) 0, separator);
  }
  return count;
}

//jobs
struct job
{
   int process_id;
   char command[80];
   int job_number;
};

struct job job_array[20];
int numJobs = 0;

void childDone(int signal)
{ 
   int status;
   pid_t pid;
   pid = waitpid(-1,&status,WNOHANG);

   int x,flag=-1;
   for(x = 0;x<numJobs;x++)
   {
	if(flag==0)
	{
	   job_array[x-1]=job_array[x];
        }

	if(job_array[x].process_id==pid)
	{
	   flag=0;
	   printf("\n\t[%d]Done %s\n",job_array[x].job_number,
			job_array[x].command);
	   printf("\tChild returned with status:%d\n",status);
	   fflush(stdout);
	   numJobs--;
	}
   }
   if(numJobs == 0)strncpy(job_array[0].command,"",1);
};

/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */
int main(void)
{
   char cmdline[MAXLINE];
   char *argv[MAXARGS];
   int argc;
   int status;
   pid_t pid;
int i;

   int isBackgroundJob;

   struct sigaction act;
   struct sigaction oact;
   sigaction(SIGTERM,NULL,&oact);//save (old action)

   act.sa_handler = SIG_IGN;	 //new action
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   sigaction(SIGINT, &act, 0);   //set (new action)

   //child process done or exited
   if(signal(SIGCHLD, childDone)==SIG_ERR)
	_exit(0);

   for(i=0; i<10;i++) 
   {
	
        printf("csc60mshell> ");
	fgets(cmdline, MAXLINE, stdin);

	//parese input
    	argc = parseline(cmdline, argv); 
	
	//restart while loop if user simply pressed enter
  	if(argc == 0)
	{
	   continue;
   	}
	else
	{
	   //check if it is a background proc
	   if(strcmp(argv[argc-1],"&")==0)
	   {
		isBackgroundJob = 0;
		argv[ argc-1]=NULL;//removes &
		argc--;
		
		
		int n;
		for(n = 0; n<argc;n++){
		   strcat(job_array[numJobs].command,argv[n]);
		   strcat(job_array[numJobs].command," ");
		}

		job_array[numJobs].job_number = numJobs+1;
	   }
	   else 
		isBackgroundJob = 1;

	   //built in command
	   if(strcmp(argv[0],"cd")== 0)
	   {
		// changes dir 
		if(argc == 1)
		   chdir(getenv("HOME"));
		//directory to the input path
		else if(argc == 2)
		   chdir(argv[1]);
		else
		{
		   printf("\tError: Too many arguments.\n");
		   continue;
		}
	
		//update env
		char tempbuf[256];
		getcwd(tempbuf, 256);
		setenv("PWD", tempbuf, 1);
	   }
	   else if(strcmp(argv[0],"pwd") == 0)
	   {
		if(argc == 1)
		   printf("\t%s\n", getenv("PWD"));
		else
		   printf("\tError too many arguments.\n");
	   }
	   else if(strcmp(argv[0],"exit") == 0)
	   {
		if(numJobs==0)exit(0);
		else 
		   printf("Close all background jobs before exiting\n");
		 
	   }
	   else if(strcmp(argv[0],"jobs")==0)
	   {
		if(numJobs == 0)
		   printf("No jobs currently running.\n");

		else
		{
		   int i;
		   for(i = 0; i<numJobs;i++)
		      printf("[%d]Running   %s\n",
				job_array[i].job_number,
					job_array[i].command);
		} 
	  }
	   // non built in commands
	  else
	  {
		//validation
		if((strcmp(argv[0],">")==0))
		{
		   printf("\tError no command.\n");
		   continue;
		}
		else if(strcmp(argv[0],"<")==0)
		{
		   printf("\tError no command.\n");	
		   continue;
		}
		else if (strcmp(argv[(argc-1)],"<") ==0) 
	   	{
		   printf("\tError no input file specified.\n");
		   continue;
		}
		else if(strcmp(argv[(argc-1)],">")==0)
		{
		   printf("\tError no output file specified.\n");
		   continue;
		}
		
		//if no errors
		pid  = fork();
		if(pid == -1)
		   perror("\tShell Program: Fork Error\n");
		else if(pid == 0)//child
		{
		   if(isBackgroundJob==0) setpgid(0,0);
		   else sigaction(SIGINT,&oact,NULL);
		   process_input(argc, argv);
		   _exit(EXIT_SUCCESS);
		}
		else//parent waits 
		{
		   if(isBackgroundJob==0)
		   {
			job_array[numJobs].process_id = pid;
		   	numJobs++;
		   }
	    	   else
	           {
		      if(waitpid(-1,&status,0) == -1)
		         perror("\tShell Program Error\n");
		      else if(WIFSIGNALED(status))
		      {
		         printf("\tChild returned status: %d\n", 
				WTERMSIG(status));
		      }
		      else if(WIFSTOPPED(status))
		      {
			 printf("\tChild returned status:%d\n",
				WIFSTOPPED(status));
		      }
		      else if(WIFEXITED(status))
		      {
		         printf("\tChild returned status:%d\n",
				WEXITSTATUS(status));
		      }
		   }
						
		}//4loop
	   }//non-built commands
   	   continue;
	}
}
}

   //Main 4loop
