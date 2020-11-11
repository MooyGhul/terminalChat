# include <stdio.h>
# include <string.h>
# include <pthread.h>
# include <stdbool.h>
# include <string.h>
# include <stdlib.h>
# include <netdb.h>
# include <unistd.h>
# include <time.h>

# include "list.h"
# include "process.h"

static List* outgoingList;
static List* incomingList;


static char* myPortNumer;
static char* remoteMachineName;
static char* remotePortNumber;

# define MAX_LEN 2048

static char* sendContent = NULL;
static char* inputContent = NULL;
static char* outputContent = NULL;
static char* receiveContent = NULL;

void* inputThread(void* unused);
void* outputThread(void* unused);
void* sendThread(void* unused);
void* receiveThread(void* unused);

static pthread_cond_t s_syncOkToPrintCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToPrintMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t s_syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToSendMutex = PTHREAD_MUTEX_INITIALIZER;



static char* exitCond = "!";

static void pItemFreeFn(void* node){
    
}

int buildChat(char* myPortNUM,char* remoteMachineNAME,char* remotePortNUM){

    myPortNumer = myPortNUM;
    remoteMachineName = remoteMachineNAME;
    remotePortNumber = remotePortNUM;

    outgoingList = List_create();
    if(outgoingList == NULL){
        printf("FAILED TO CREATE LIST!");
        exit(1);
    }


    incomingList = List_create();
    if(incomingList == NULL){
        printf("FAILED TO CREATE LIST!");
        exit(1);
    }

    pthread_t threadInputData;
    pthread_t threadReceiveData;
    pthread_t threadSendData;
    pthread_t threadOutputData;

    int rc1 = pthread_create(&threadInputData, NULL, inputThread, NULL);
    int rc2 = pthread_create(&threadOutputData, NULL, outputThread, NULL);

    int rc3 = pthread_create(&threadReceiveData, NULL, receiveThread, NULL);
    int rc4 = pthread_create(&threadSendData, NULL, sendThread, NULL);

    

    if(rc1 || rc2 || rc3 || rc4){
       printf("ERROR; pthread_create() FAILED\n");
       exit(-1);
    }

    rc1 = pthread_join(threadInputData,NULL);
    rc2 = pthread_join(threadOutputData,NULL);
    rc3 = pthread_join(threadReceiveData,NULL);
    rc4 = pthread_join(threadSendData,NULL);
    
    if(rc1 || rc2 || rc3 || rc4){
       printf("ERROR; pthread_join() FAILED\n");
       exit(-1);
    }


    List_free(incomingList, pItemFreeFn);
    List_free(outgoingList, pItemFreeFn);

    return 0;
}

int equalsExit(char* a, char* b){

    for(int i = 0; i<1; i++){
        if(*(a+i)!=*(b+i)){
            return 0;
        }
    }
    return 1;
}

void* outputThread(void* unused){
    while(1){
        pthread_mutex_lock(&s_syncOkToPrintMutex);
        {
        if(List_count(incomingList)==0){
            pthread_cond_wait(&s_syncOkToPrintCondVar,&s_syncOkToPrintMutex);
            
            outputContent = (char*)List_trim(incomingList);

            if((*(outputContent+1)=='\0') && equalsExit(outputContent,exitCond)){
                printf("%s has left the chart, input 'exit!' to leave chart\n",remoteMachineName);
                free(outputContent);
                break;
            }
            printf("  %s: %s\n",remoteMachineName,outputContent);
            free(outputContent);
        }
        pthread_mutex_unlock(&s_syncOkToPrintMutex);
        }
    }
    return NULL;
}

void* inputThread(void* unused){
    while(1){
        inputContent = malloc(MAX_LEN);
        char* fg = fgets(inputContent, MAX_LEN, stdin);


        if(fg == NULL){
            printf("FAILED TO READ FROM YOUR KEYBOARD!");
            exit(1);
        }

        for(int i = 0; i<MAX_LEN; i++){
            if(*(inputContent+i)=='\n'){
                *(inputContent+i)='\0';
                break;
            }
        }

        pthread_mutex_lock(&s_syncOkToSendMutex);
        { 

        List_prepend(outgoingList,inputContent);

        pthread_cond_signal(&s_syncOkToSendCondVar);

        }
        pthread_mutex_unlock(&s_syncOkToSendMutex);

        if((*(inputContent+1)=='\0') && equalsExit(inputContent,exitCond)){
            
            break;
        }
    }

    
    return NULL; 
}

void* sendThread(void* unused){

    struct addrinfo hints;
    struct addrinfo *res;
    int status;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;	        // IPv4
    hints.ai_socktype = SOCK_DGRAM;	    // UDP
    hints.ai_flags = AI_PASSIVE;	    // fill in my IP for me

    if  ((status = getaddrinfo(
                    remoteMachineName,  // e.g. "www.example.com" or IP 
                    remotePortNumber,   // e.g. "http" or port number
                    &hints, 
                    &res                // get IP address
    )) != 0) 
    {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockfd = socket(res->ai_family, res->ai_family, res->ai_protocol);
    const struct sockaddr* to = res->ai_addr;
    socklen_t tolen = res->ai_addrlen;

    freeaddrinfo(res);

    while(1){

        pthread_mutex_lock(&s_syncOkToSendMutex);
        {   

            if(List_count(outgoingList)==0){
                pthread_cond_wait(&s_syncOkToSendCondVar,&s_syncOkToSendMutex);

            }
            while(List_count(outgoingList)!=0){
                sendContent = (char*)List_trim(outgoingList);
                int limitChars = MAX_LEN;

                sendto(
                    sockfd, 
                    sendContent, 
                    limitChars, 
                    0,
                    to,  
                    tolen          
                );


            }
            if((*(sendContent+1)=='\0') && equalsExit(sendContent,exitCond)){
                printf("You are leaving the chart. Please wait for the other user to be fully disconnected.\n");
                free(sendContent);
                break;
            }
        }
        pthread_mutex_unlock(&s_syncOkToSendMutex);

    }
    
    return NULL;
}

void* receiveThread(void* unused){

    // ****** Address Structure ****** 
    struct sockaddr_in sin;                     // in means internet
                                                // local variable on stack
    memset(&sin, 0, sizeof(sin));

    // SETUP STRUCTURE, ALL happened userspace.
    sin.sin_family = AF_INET;                   // AF address
    sin.sin_addr.s_addr = htonl(INADDR_ANY);    // work on/ open any local port
    sin.sin_port = htons(atoi(myPortNumer));    // host to network short (16 bit)

    int socketDescriptor =  socket              // told linux to open a socket,here is the socket
                            (             
                                PF_INET,        // PF for protocol
                                SOCK_DGRAM,      // DGRAM: UDP
                                0
                            );   

    bind(socketDescriptor,                      // opens socket
        (struct sockaddr*) &sin,               // IP address
        sizeof(sin)
    );

    // ****** Receive Data ******
    while(1){
        struct sockaddr_in sinRemote; 
        unsigned int sin_len = sizeof(sinRemote);
        receiveContent = malloc(MAX_LEN);

        
        int bytesRx = recvfrom(                 // blocking call (waits until receive sth)
            socketDescriptor,
            receiveContent,                     // client data written here (buffer)
            MAX_LEN,
            0,
            (struct sockaddr*) &sinRemote,      // who get infomation from
                                                // field with the sender's IP addr as ptr
            &sin_len                            // sin is output parameter
                                                // sinLen is iin/out parameter
        );


        // NULL terminated the string received
        int terminateIdx = (bytesRx < MAX_LEN) ? bytesRx : MAX_LEN - 1;
        receiveContent[terminateIdx] = 0;


        pthread_mutex_lock(&s_syncOkToPrintMutex);
        {   

            List_prepend(incomingList, receiveContent);

            pthread_cond_signal(&s_syncOkToPrintCondVar);

        }
        pthread_mutex_unlock(&s_syncOkToPrintMutex);


        if((*(receiveContent+1)=='\0') && equalsExit(receiveContent,exitCond)){
            
            break;
        }
    }
    return NULL;
}