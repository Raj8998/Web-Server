#include"headerInclude.h"
#define SERVER_LISTENING_CAPACITY 30000

// The Thread Runner Function
void *threadRunnerFunction(void *argv);

// The Lock checking function
int checkAndWaitForConnection(int threadID);

// The process request function
char *processRequest(char data[4096]);

// The utility function to create response object
char *createResponseForCode(int statusCode, char *httpVersion, char data[4096]);

// The utility function to convert response object to actual response string.
char* getReponseString(struct HTTP_Response response);

// The utility function for file read.
char *readFileData(char PATH[]);

// The utility function to add an active socket to the queue. Must run atomic
int addToQueue(int socketID);

// The utility function to get an active socket from the queue and remove it from queue. Must run atomic
int getSocketIDFromQueue();

// The utility function to generate date string
char *generateDate();

int socketQueue[SERVER_LISTENING_CAPACITY];

int count = 0;
int ConnectionCount = 0;

pthread_mutex_t mutex;
pthread_cond_t empty, fill;

// The Thread Runner Function
void *threadRunnerFunction(void *argv){
    int threadID = *((int *)argv);
    threadID = threadID+1;
    while(1){

        int sockID = checkAndWaitForConnection(threadID);

        printf("New Socket ID Found: %d in Thread: %d\n", sockID, threadID);

        char recievedData[4096];
        char *dataToBeSent;
        bzero(recievedData, 4096);

        int n = read(sockID, (void *)recievedData, 4096);

        dataToBeSent = processRequest(recievedData);

        write(sockID, dataToBeSent, strnlen(dataToBeSent, 4096));
        free(dataToBeSent);
        close(sockID);
    }
    printf("Outside thread While loop.\n");
}

// The Lock checking function
int checkAndWaitForConnection(int threadID){
    pthread_mutex_lock(&mutex);
    while(count == 0){
        pthread_cond_wait(&fill, &mutex);
    }

    int sockID = getSocketIDFromQueue();
    count--;
    ConnectionCount++;
    printf("Servicing %d request.\n", ConnectionCount);
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    return sockID;
}

// The process request function
char *processRequest(char data[4096]){
	int init_size = strlen(data);
	char delim[] = "\n";
    int lineNum = 0;
    struct HTTP_Request request;

	char *ptr = strtok(data, delim);
	while(ptr != NULL && lineNum < 3)
	{
		char delim2[] = " ";
        char *headers = strtok(ptr, delim2);
        while(headers != NULL){
            
            if (lineNum == 0) strcpy(request.method, headers);

            if (lineNum == 1) strcpy(request.url, headers);

            lineNum +=1;
            headers = strtok(NULL, delim2);
        }
		ptr = strtok(NULL, delim);
	}

    if(strcmp(request.method, "GET") == 0){
        struct stat stats;
        char PATH[4096];
        strcpy(PATH, "/var/www");
        char indexfile[] = "/index.html";
        strncat(PATH, request.url, strnlen(request.url, 4096));
        if(stat(PATH, &stats) == 0){
            if (S_ISDIR(stats.st_mode)){
                strncat(PATH, indexfile, strlen(indexfile));
            }
            char *fileData = readFileData(PATH);
            strcpy(data, fileData);
            free(fileData);
            return createResponseForCode(200, request.HTTP_version, data);
        } else{
            strcpy(data, "Path Not Found.");
            return createResponseForCode(404, request.HTTP_version, data);
        }        
    } else {
        strcpy(data, "Unsupported Method Request!");
        return createResponseForCode(500, request.HTTP_version, data);
    }
}

// The utility function to create response object
char *createResponseForCode(int statusCode, char *httpVersion, char data[4096]){
    struct HTTP_Response response;
    switch (statusCode)
    {
    case 200:
        strcpy(response.HTTP_version, httpVersion);
        sprintf(response.content_length, "%d", (int)strnlen(data, 4096));
        strcpy(response.content_type,"text/html");
        strcpy(response.body, data);
        strcpy(response.status_code, "200");
        strcpy(response.status_text, "OK");
        break;
    case 404:
        strcpy(response.HTTP_version, httpVersion);
        sprintf(response.content_length, "%d", (int)strnlen(data, 4096));
        strcpy(response.content_type,"text/html");
        strcpy(response.body, data);
        strcpy(response.status_code, "404");
        strcpy(response.status_text, "Not Found");
        break;
    case 500:
        strcpy(response.HTTP_version, httpVersion);
        sprintf(response.content_length, "%d", (int)strnlen(data, 4096));
        strcpy(response.content_type,"text/html");
        strcpy(response.body, data);
        strcpy(response.status_code, "500");
        strcpy(response.status_text, "Unsupported Method");
        break;
    default:
        break;
    }

    return getReponseString(response);
}

// The utility function to convert response object to actual response string.
char* getReponseString(struct HTTP_Response response){
    char *myData = (char *)malloc(4096*sizeof(char));
    bzero(myData, 4096);

    strcpy(myData, "HTTP/1.1");
    strcat(myData, " ");

    strcat(myData, response.status_code);
    strcat(myData, " ");

    strcat(myData, response.status_text);
    strcat(myData, " ");
    strcat(myData, "\r\n");

    strcat(myData, "Content-Type: ");
    strcat(myData, response.content_type);
    strcat(myData, "\r\n");

    
    strcat(myData, "Date: ");
    // date creation
    char *date= generateDate();
    strncat(myData, date, 1000);
    free(date);

    strcat(myData, "\r\n");
    
    strcat(myData, "Content-Length: ");
    strcat(myData, response.content_length);
    strcat(myData, "\r\n\n");

    strcat(myData, response.body);

    return myData;
}

// The utility function for file read.
char *readFileData(char PATH[]){
    FILE* ptr;
    char *data = (char *)malloc(4096*sizeof(char));
    ptr = fopen(PATH, "r");
    char str[50];
 
    if (NULL == ptr) {
        printf("file can't be opened \n");
    }

    while (fgets(str, 50, ptr) != NULL) {
        strncat(data, str, 50);
    }
    while (!feof(ptr)) {
        
    }
 
    fclose(ptr);
    return data;
}

// The utility function to initialize active socket queue with -1
void initalizeSocketQueue(){
    for(int i=0;i<SERVER_LISTENING_CAPACITY; i++){
        socketQueue[i] = -1;
    }
}

// The utility function to add an active socket to the queue. Must run atomic
int addToQueue(int socketID){
    for(int i=0;i<SERVER_LISTENING_CAPACITY; i++){
        if(socketQueue[i] == -1){
            socketQueue[i]=socketID;
            return 0;
        }
    }
    return 1;
}

// The utility function to get an active socket from the queue and remove it from queue. Must run atomic
int getSocketIDFromQueue(){
    int sockID = -1;
    for(int i=0; i<SERVER_LISTENING_CAPACITY; i++){
        if(socketQueue[i] != -1){
            sockID = socketQueue[i];
            socketQueue[i] = -1;
            break;
        }
    }
    return sockID;
}

// The utility function to generate date string
char *generateDate(){
    char *buf = (char *)malloc(sizeof(char)*1000);
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    int n = strftime(buf, 1000, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return buf;
}