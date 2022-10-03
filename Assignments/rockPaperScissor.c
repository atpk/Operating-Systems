#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

char name[10];
char buffer[80];
int p[2];

#define PAPER 1
#define SCISSOR 2
#define ROCK 3

int readfrompipe (int p) { // read from pipe
   int x;
   read(p,buffer,sizeof(buffer));
   sscanf(buffer,"%d",&x);
   return x;
}

void writetopipe(int x) { // write to pipe
    sprintf(buffer,"%d",x);
    write(p[1], buffer, sizeof(buffer));
}

void childSigHandler ( int sig )
{
   if (sig == SIGUSR1) {
      // create random number from 1 to 3
      int no = 1 + rand() % 3;
      writetopipe(no);
   } else if (sig == SIGUSR2) {
      printf("Child : Received signal EXIT from parent...\n");
      sleep(1);
      exit(0);
   }
}

void chsig(int a, int b) {
   signal(SIGUSR1, childSigHandler);           /* Register SIGUSR1 handler */
   signal(SIGUSR2, childSigHandler);           /* Register SIGUSR2 handler */
   
   // Get pipes
   p[0] = a;
   p[1] = b;

   srand(time(NULL)+p[0]);
   // Close read end
   close(p[0]);

   // Busy waiting
   while(1) {}

   exit(0);
}

int main () {
   int pidc, pidd; // Childs id
   int pc[2];  // Communication to C
   pipe (pc);
   int pd[2];  // Communication to D
   pipe (pd);
   
   pidc = fork(); // spawn child c
   
   if (pidc) { // parent
      pidd = fork(); // spawn child d
      
      if (pidd) { // parent
         close(pc[1]);
         close(pd[1]);
      }
      else { // Child d
         chsig(pd[0],pd[1]);// wait for signal
         strerror(errno);
         exit(0);
      }
   } else { // Child c
      chsig(pc[0],pc[1]);// wait for signal
      strerror(errno);
      exit(0);
   }

   /* Parent process */
   printf("Parent: Game starts\n");
   int read_c, read_d;
   float cscore=0, dscore=0;
   while (cscore<=10 && dscore<=10) {
      printf("Parent: Sending ready to both child\n");

      kill(pidc,SIGUSR1);
      read_c = readfrompipe(pc[0]);
      printf("C: %d\n",read_c);

      kill(pidd,SIGUSR1);
      read_d = readfrompipe(pd[0]);
      printf("D: %d\n",read_d);

	  // update the scores wrt values recieved
      if (read_d== read_c) {
         cscore+=0.5;
         dscore+=0.5;
      }
      else {
         switch(read_c) {
            case SCISSOR:
            switch (read_d) {
               case PAPER: cscore++; break;
               case ROCK: dscore++; break;
            }
            break;
            case PAPER:
            switch (read_d) {
               case SCISSOR: dscore++; break;
               case ROCK: cscore++; break;
            }
            break;
            case ROCK:
            switch (read_d) {
               case SCISSOR: cscore++;break;
               case PAPER: dscore++; break;
            }
            break;
         }
      }

      printf("Parent: C's score => %f\n", cscore);
      printf("Parent: D's score => %f\n", dscore);
   }
   
   int winner;
   if (cscore>10 && dscore>10) winner = 0;
   else if (cscore>10)  winner = 1;
   else if (dscore>10)  winner = 2;
   
   if(winner==0)   printf("Parent: Game drawn\n");
   if(winner==1)   printf("Parent: C won the game\n");
   if(winner==2)   printf("Parent: D won the game\n");
   
   printf("Parent: sending exit to both child\n");
   kill(pidd,SIGUSR2);
   kill(pidc,SIGUSR2);
   printf("Parent: waiting for children to end\n");
   wait(NULL);
   wait(NULL);
   printf("Parent: Children exited\n");
   exit(0);
}
