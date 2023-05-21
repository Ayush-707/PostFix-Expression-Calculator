#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 9003
#define BUFFER_SIZE 1024
#define MAX_INPUT_LENGTH 1024

int is_operator(char c) {
    //printf("operator = '%c'", c);
    if (c == '+' || c == '-' || c == '*' || c == '/') {
        return 1;
    } else return 0;
}


int is_valid_postfix_expression(char *input) {

    if (strcmp(input,"exit") == 0) {
        return 1;
    }
    int operand_count = 0;
    int operator_count = 0;
    int length = strlen(input);

    for (int i = 0; i < length; i++) {
        char c = input[i];
        if (isdigit(c) || isspace(c) || c == '.') {
            // Valid operand character
            if (isdigit(c) || c == '.') {
                // Check if the operand is more than one digit
                while (isdigit(input[i+1]) || input[i+1] == '.') {
                    i++;
                }
                operand_count++;
            }
            
        } else if (is_operator(c)) {
            // Valid operator character
            operator_count++;
            if (operand_count < 2) {
                // Not enough operands for this operator
                return 0;
            }
            //operand_count--;
            if (operator_count >= operand_count) {
                // More operators than operands
                return 0;
            }
        } else {
            // Invalid character
            return 0;
        }
    }

    // Check that there are more operands than operators
    return (operand_count - operator_count >= 1);
}


char* get_validated_input() {
    char*input = malloc(MAX_INPUT_LENGTH*sizeof(char));
    int i;
    do {
        printf("-> Enter a valid postfix expression\n-> Enter 'exit' to disconnect from server: ");
        fgets(input, MAX_INPUT_LENGTH, stdin);

        // Remove newline character from input
        input[strcspn(input, "\n")] = '\0';
        //printf("%s", input);
        i = is_valid_postfix_expression(input);
        //printf("%d", i);

        if (!i) {
            printf("Invalid input. Please try again.\n");
        }
    } while (!i);

    // Convert input string to double and return
    return input;
}


int main() {

    

                //Declare variables
            int sock, num_bytes;
            struct sockaddr_in servaddr;
            char* buffer = malloc(1024*sizeof(char));

            // Create socket
            sock = socket(AF_INET, SOCK_STREAM, 0); // create a socket with the given domain, type and protocol
            if (sock < 0) { // check if socket creation was unsuccessful
                printf("Error creating socket\n"); // print an error message
                return 1; // exit the program with an error code
            }

            // Fill in server address information
            memset(&servaddr, 0, sizeof(servaddr)); // fill the memory with 0s
            servaddr.sin_family = AF_INET; // set the address family to IPv4
            servaddr.sin_port = htons(SERVER_PORT); // set the server port number
            if (inet_pton(AF_INET, SERVER_ADDRESS, &servaddr.sin_addr) <= 0) { // convert the server address from text to binary
                printf("Invalid address or address not supported\n"); // print an error message if the conversion fails
                return 1; // exit the program with an error code
            }
            // Connect to server
            if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { // Try to connect to the server
                printf("Error connecting to server\n"); // If connection fails, print error message
                return 1; // Return 1 to indicate an error has occurred
            } else {
                printf("\n************Connected to the server************\n"); // If connection succeeds, print success message
            }
            
            while ( 1 ) {

                buffer = get_validated_input();
                num_bytes = send(sock, buffer, strlen(buffer), 0); // Send input data to server

                if (strcmp(buffer, "exit") == 0){
                    close(sock);
                    printf("[-]Disconnected\n");
                    exit(1);
                }



                if (num_bytes < 0) {
                    printf("Error sending data to server\n"); // If sending data fails, print error message
                    return 1; // Return 1 to indicate an error has occurred
                } else {
                    // Pass command and data to server
                    printf("\nSending Input Data: '%s' to the Server\n\n", buffer);

                }

                //Receiving data from the server
                char output_string[1024];
                int bytes = recv(sock, output_string, 1024, 0); // Receive data from server
                if (bytes < 0) {
                    printf("Error receiving data from server\n"); // If receiving data fails, print error message
                    //return 1; // Return 1 to indicate an error has occurred
                } else {
                        output_string[bytes] = '\0'; // Add null terminator to the end of the string
                        printf("Received data: %s\n\n", output_string); // Print output data received from server


                }

                
                // Close socket
               // close(sock);


            }
                        return 0;
}
