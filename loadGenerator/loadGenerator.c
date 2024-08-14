#include"loadGenThreadRunner.h"

int main(int argc, char const* argv[]){

    if(argc != 6){
        fprintf(stderr, "Usage: %s <server-name-or-ip> <host-port> <user-count> <think-time> <test-duration>\n", argv[0]);
        exit(1);
    }
    

    // Verify server address
    host = gethostbyname(argv[1]);
    if(host == NULL){
        host = gethostbyaddr((char *)&argv[1], sizeof(struct in_addr), AF_INET);
        if(host == NULL){
            fprintf(stderr, "Invalid server IP address or name.");
            exit(1);
        }
    }

    /* open log file */
    log_file = fopen("load_gen.log", "w");
    fprintf(log_file, ".................Start.................\n");
    fprintf(log_file, "Starting the load generator.\n\n");
    pthread_t threadArray[atoi(argv[3])];
    struct userInfo *users[atoi(argv[3])];
    printf("Creating load generators.\n");
    for(int i=0; i<atoi(argv[3]); i++){
        pthread_t threadId;
        users[i] = createUser(i+1, atoi(argv[2]), atof(argv[4]));
        if(pthread_create(&threadId, NULL, threadRunnerFunction, (void *)users[i]) != 0){
                perror("pthread_create");
        }
        threadArray[i] = threadId;
        // printf("Load generator: User %d Created\n", (i+1));
    }

    sleep(atoi(argv[5]));

    pthread_mutex_lock(&mutex);
    time_up = 1;
    pthread_mutex_unlock(&mutex);

    int totalRequests = 0;
    float total_rtt = 0;
    for(int i=0; i<atoi(argv[3]); i++){
        pthread_join(threadArray[i], NULL);
        totalRequests = totalRequests + users[i]->total_count;
        total_rtt = total_rtt + users[i]->total_rtt;
        fprintf(log_file, "Total Requests Sent by user: %d are %d, total RTT= %lf\n",users[i]->id, users[i]->total_count, users[i]->total_rtt);
    }

    fprintf(log_file,"\nTotal Requests sent in span of %d seconds are %d and cumulative RTT is %lf seconds\n", atoi(argv[5]), totalRequests, total_rtt);
    fprintf(log_file, "Number of Users: %d \n", atoi(argv[3]));
    fprintf(log_file, "Average Throughput: %.2f \n", totalRequests/(atoi(argv[5]))*1.0);
    fprintf(log_file, "Average Response Time: %.6f \n", total_rtt/(totalRequests)*1.0);
    fprintf(log_file, ".................End.................\n\n");

    /* close log file */
    fclose(log_file);

    printf("Total Requests sent in span of %d seconds are %d and cumulative RTT is %lf seconds\n", atoi(argv[5]), totalRequests, total_rtt);
    printf("Average Throughput: %.2f \n", totalRequests/(atoi(argv[5]))*1.0);
    printf("Average Response Time: %.6f \n", total_rtt/(totalRequests)*1.0);
}