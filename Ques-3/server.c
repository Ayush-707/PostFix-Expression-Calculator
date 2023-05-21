#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#define PORT 9003
#define MAX_CLIENTS 10
#define STACK_MAX 10000



// Declaration of functions
//char* handle_input ( char *data);
double compute_postfix_expression(char *buffer);

time_t start, current;




// Definition of a struct to hold arguments for a function

typedef struct {
    int top;
    float items[STACK_MAX];
} Stack;






void push(Stack *s, double item) {
    if (s->top < STACK_MAX) {
        s->items[s->top++] = (double)item;
    } else {
        fprintf(stderr, "Error: stack overflow\n");
        exit(EXIT_FAILURE);
    }
}

double pop(Stack *s) {
    if (s->top > 0) {
        return s->items[--s->top];
    } else {
        fprintf(stderr, "Error: stack underflow\n");
        exit(EXIT_FAILURE);
    }
}

int is_operator(char c) {
    //printf("operator = '%c'", c);
    if (c == '+' || c == '-' || c == '*' || c == '/') {
        return 1;
    } else return 0;
}

double perform_operation(double a, double b, char operator) {
    double c;
    switch (operator) {
        case '+': 
            c = a+b;
            return c;
        case '-': 
            c = a-b;
            return c;
        case '*': 
            c = a*b;
            return c;
        case '/': 
            c = a/b;
            return c;
        default: fprintf(stderr, "Error: invalid operator\n"); exit(EXIT_FAILURE);
    }
}

double compute_postfix_expression(char *buffer) {
    Stack stack;
    stack.top = 0;

    int i = 0;
    int j = 0;
    
    while (j <= strlen(buffer) - 1) {

        char *offset;
        //printf("Char = '%c'\n",temp);

        if (isdigit(buffer[i])) {
            double value = strtod(buffer + i, &offset);
            i = offset - buffer;
            push(&stack, value);
        } else if (is_operator(buffer[i])) {

            double b = pop(&stack);
            double a = pop(&stack);
            double result = perform_operation(a, b, buffer[i]);
            push(&stack, result);
        }
        i++;
        j++;
    }

    return pop(&stack);
}


// This function creates a socket and returns its file descriptor

int main() {

    time(&start);
    
    int sockfd, ret;

    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    addr_size = sizeof(newAddr);

    char buffer[1024];
    pid_t childpid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-]Error in connection\n");
    }

    printf("[+]Server socket created\n");

            memset(&serverAddr, 0, addr_size); // fill the memory with 0s
            serverAddr.sin_family = AF_INET; // set the address family to IPv4
            serverAddr.sin_port = htons(PORT); // set the server port number
            
            ret = bind(sockfd, (struct sockaddr*)&serverAddr, addr_size);
            
            if (ret < 0) {
                printf("Error in binding\n");
                exit(1);
            }

            printf("[+]Bind to port: %d\n", PORT);

            if ( listen(sockfd, 10) == 0) {
                printf("[+]Listening to clients....\n");
            } else {
                printf("[-]Error in listening\n");
                
            }


            while (1) {
                newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);

                if (newSocket < 0) {
                    exit(1);
                }

                printf("[+]Connection Accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

                if ((childpid =fork()) == 0) {
                    close(sockfd);

                    while(1) {
                        
                        recv(newSocket, buffer, 1024, 0);

                        if(strcmp(buffer, "exit") == 0) {
                            printf("[-]Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                            break;
                        } else {
                            printf("[->]Client Request: %s\n", buffer);
                            double ans = compute_postfix_expression(buffer);
                            char str[20], response[5000];
                            snprintf(str, sizeof(str), "%.0f",ans);
                            send(newSocket, str, strlen(str), 0);
                            time(&current);
                            double elapsed = difftime(current,start);
                            sprintf(response, "[Client ID: '%d'], [Query: '%s'], [Answer: '%s'], [Time Elapsed: '%.0f']",ntohs(newAddr.sin_port),buffer,str,elapsed);
                            bzero(buffer, sizeof(buffer));
                            bzero(str, sizeof(str));

                            FILE* fp = fopen("server_records.txt", "a");
                            fprintf(fp,"%s\n\n",response);
                            fclose(fp);


                        }


                    }
                }
            }



    close(newSocket);
    return 0;
}
