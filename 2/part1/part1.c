#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 

int main(){ 
    char out[] = "Hello World"; // final output
    setbuf(stdout, NULL);
    int curr=0,n=11,random_num=0; // necessary variables
    while(curr < n){
    	int pid_num = fork();
        if(pid_num > 0){ // parent process
            printf("%c %d\n", out[curr], pid_num); // print designated character
            random_num = rand()%4 + 1; // generate random number (1-4)
            sleep(random_num); // sleep for random (1-4) number of seconds
            curr++; //to spawn next process by curr process
        }
        else // child process
        	curr = n; //to end process
    }
    return 0; 
}
