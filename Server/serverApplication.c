#include"threadRunner.h"

#define MAX_WORKER_THREADS 5
int serverSocket;
pthread_t threadIds[MAX_WORKER_THREADS];

void signalHandler(int sig){
    signal(SIGINT, signalHandler);
    printf("\nShutting Down Server...\n");
    exit(0);
}

// void signalHandler(int sig){
//     // signal(SIGINT, signalHandler);
//      close(serverSocket);
//      printf("\nRepeaing off worker threads.\n");
//      for(int i=0 ;i<MAX_WORKER_THREADS; i++){
//          pthread_kill(threadIds[i], SIGKILL);
//      }
//      for(int i=0 ;i<MAX_WORKER_THREADS; i++){
//          pthread_join(threadIds[i], NULL);
//      }
//     printf("\nShutting Down Server...\n");
//     exit(0);
// }

int main(int argc, char const* argv[]){
    signal(SIGINT, signalHandler);
    if(argc != 2){
        printf("Usage: %s <port>", argv[0]);
        exit(1);
    }

    signal(SIGINT, signalHandler);

    // Create server-listener socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        printf("Error opening socket. Please try again after some time.");
        exit(1);
    }

    // Set server address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[1]));  // Setup server port
    serverAddress.sin_addr.s_addr = INADDR_ANY;     // Any incoming message will be considered for this server

    // Bind server to specific port
    int binStatus = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(binStatus < 0){
        printf("Error binding the server to port %d. Please try again after some time.\n", atoi(argv[1]));
        exit(1);
    }

    // Spin up listener
    if(listen(serverSocket, SERVER_LISTENING_CAPACITY) < 0){
        printf("Error starting listener. Please try again after some time.\n");
        exit(1);
    }

    initalizeSocketQueue();
    int id;
    for(int i=0; i<MAX_WORKER_THREADS; i++){
        pthread_t threadId;
        id=i;
        if(pthread_create(&threadId, NULL, threadRunnerFunction, &id) != 0){
                perror("pthread_create");
        }
        sleep(1);
        threadIds[i] = threadId;
        printf("Thread: %d Created\n", id);
    }

    printf("Server Initialization Complete...\n");

    struct sockaddr_in clientAddress;
    socklen_t clientSocketLength = sizeof(clientAddress);
    while(1){

        // if (sigsetjmp(env, 1) == 42) {
        //     // printf("\n");
		// 	continue;
        // }
		// jump_active = 1;

        // if capacity is full do not accept any requests and wait for conditional signal.
        while(count == SERVER_LISTENING_CAPACITY){
            pthread_cond_wait(&empty, &mutex);
        }
        
        int clientAcceptedSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientSocketLength);
        if(clientAcceptedSocket < 0){
            printf("Error accepting client. Server might be at full capacity.\n");
            continue;
        }
        pthread_mutex_lock(&mutex);

        printf("Got Connection for FD: %d... \n", clientAcceptedSocket);

        addToQueue(clientAcceptedSocket);
        count++;
        // conditional fill signal so worker threads can take it up.
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(&mutex);
        signal(SIGINT, signalHandler);
    }

}