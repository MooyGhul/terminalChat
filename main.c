# include <stdio.h>
# include <string.h>
# include <pthread.h>
# include <stdbool.h>
# include <string.h>
# include <stdlib.h>

# include "process.h"

int main(int argc, char** args){
    if(argc!=4){
        printf("\n*** Please Provide all the required args ***\n");
        exit(1);
    }

    printf("********************************\n");
    printf("You have connected with %s\n", args[2]);
    printf("Enter '!' if you want to exit.\n");
    printf("********************************\n\n");

    char* myPortNumber = args[1];
    char* remoteMachineName = args[2];
    char* remotePortNumber = args[3];
    
    buildChat(myPortNumber,remoteMachineName,remotePortNumber);

    printf("********************************\n");
    printf("TALKING DONE! LEFT SUCCESSFULLY\n");
    printf("********************************\n");
}