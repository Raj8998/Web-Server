#include"headerInclude.h"

float THINK_TIME = 0.1F;
int DEFAULT_PER_USER_REQUESTS_COUNT = 100;

// user info struct
struct userInfo {
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  double total_rtt;
};

void requestToServer(struct userInfo *user);
struct sockaddr_in serverAddress;
struct hostent *host;
pthread_mutex_t mutex;
int time_up = 0;
FILE *log_file;

void *threadRunnerFunction(void *argv){
    struct userInfo *user = (struct userInfo *)argv;
    // printf("User: %d\n", user->id);
    requestToServer(user);
}

void requestToServer(struct userInfo *user){
    // Setup server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(user->portno);
    bcopy((char *)host->h_addr, (char *)&serverAddress.sin_addr.s_addr,
        host->h_length);


    while(1){
        pthread_mutex_lock(&mutex);
        if(time_up == 1){
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        struct timeval start, end;
        bzero(&start, sizeof(struct timeval));
        bzero(&end, sizeof(struct timeval));
        /* start timer */
        gettimeofday(&start, NULL);

        // Create socket
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(clientSocket < 0){
            fprintf(stderr, "Error opening socket. Please try again after some time.");
            exit(1);
        }

        // Connect to server and get status
        int connectionStatus = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

        if(connectionStatus < 0){
            fprintf(stderr, "Connection error occurred. Please try again after some time.\n");
            exit(1);
        } else {
            char recievedData[4096];
            // printf("Requesting data for user: %d\n", user->id);
            char request[100] = "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\n\r\n";
            // printf("Request:\n%s\n", request);

            // Send user input message to server
            write(clientSocket, request, strnlen(request, 4096));

            // Wait for response to be recieved from server
            read(clientSocket, recievedData, 2048);

            bzero(recievedData, 4096);
            close(clientSocket);
        }
        /* end timer */
        gettimeofday(&end, NULL);
        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        double rtt = seconds + microseconds*1e-6;
        
        user->total_rtt = user->total_rtt + rtt;
        user->total_count = user->total_count + 1;
        usleep(user->think_time*1000000);
    }
}

struct userInfo *createUser(int id, int port, float thinkTime){
    struct userInfo *user = malloc(sizeof(struct userInfo));
    user->id = id;
    user->portno = port;
    user->think_time = thinkTime;
    user->total_count = 0;
    user->total_rtt = 0;
    return user;
}