/*
 ============================================================================
 Name        : FTP_SERVER.c
 Author      : Amaragy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT_NUMBER 65496
#define SERVER_NUMBER "127.0.0.1"


void print_fatal_msg(char *msg);
char* get_file_name (char* full_url);

int main(void) {
    int server_socket, client_socket;
    char buffer[1024]; // to read socket data
    struct stat file_state;
    struct sockaddr_in server_address, client_address;
    int file_handler;
    // define the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        print_fatal_msg("server tcp socket creation has failed");
    }

    int optionValue = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(int)) == -1)
        print_fatal_msg("setting socket option SO_REUSEADDR");



    // define the address
    server_address.sin_addr.s_addr = 0;
    server_address.sin_port = htons(PORT_NUMBER);
    server_address.sin_family = AF_INET;
    memset(&(server_address.sin_zero), '\0', 8);

    // bind socket to address
    if (bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address))) {
        print_fatal_msg("binding server socket to address has failed");
    }


    // listen to socket
    if (listen(server_socket,1)) {
        print_fatal_msg("unable to listen on port number ");
    }

    // print incoming messages
    while(1) {
        unsigned int address_size = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &address_size);
        if(client_socket == -1) {
            print_fatal_msg("accepting connection");
        }

        memset(&buffer, '\0', 1024);
        strcpy(buffer, "Hello for new World!\n");
        send(client_socket, buffer, 1024, 0);
        printf("Welcome to FTP server: %s",buffer);
        printf("server: got connection from %s port %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        memset(&buffer, '\0', 1024);
        int recv_length = recv(client_socket, &buffer, 1024, 0);
        printf("recv_length RECEIVED: %d\n", recv_length);
        char command[100];
        while(recv_length > 0) {
            printf("STR: %s\n", buffer);
            int file_size = 0;
            sscanf(buffer, "%s", command);
            if (strcmp(command, "get") == 0) {
                sscanf(buffer, "%s%s", command, command);
                file_handler = open(command, O_RDONLY);
                if (file_handler == -1) {
                    printf("could not open a file to get %s\n", buffer);
                    file_size = -1;
                    send(client_socket, &file_size, sizeof(int), 0);
                } else {
                    stat(command, &file_state);
                    file_size = file_state.st_size;
                    send(client_socket, &file_size, sizeof(int), 0);
                    sendfile(client_socket,file_handler,NULL,file_size);
                    close(file_handler);
                }

            } else if (strcmp(command, "put") == 0) {
                sscanf(buffer, "%s%s", command, command);
                printf("=====command===== >> %s\n", command);
                int received_file_size;
                recv(client_socket, &received_file_size, sizeof(received_file_size), 0);
                printf("the received size is %d\n", received_file_size);
                char *file_bytes = (char*)malloc(received_file_size);
                recv(client_socket, file_bytes, received_file_size, 0);
//                printf("=============== >> %s\n", file_bytes);
                char *file_name = get_file_name(command);
                printf("=========file_name====== >> %s\n", file_name);
                file_handler = open(file_name, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if (file_handler == -1) {
                    print_fatal_msg("could not open a file to put\n");
                }

                ssize_t written_size = write(file_handler, file_bytes, received_file_size);
                printf("=========written_size====== >> %d\n", written_size);
                free(file_bytes);
                close(file_handler);
            } else if (strcmp(buffer, "pwd") == 0) {
                system("pwd > /tmp/temps.txt");
                stat("/tmp/temps.txt",&file_state);
                file_size = file_state.st_size;
                send(client_socket, &file_size, sizeof(int), 0);
                file_handler = open("/tmp/temps.txt", O_RDONLY);
                sendfile(client_socket,file_handler,NULL,file_size);
                close(file_handler);
            } else if (strcmp(command, "ls") == 0) {
                system("ls > /tmp/temps.txt");                    // to execute system command and append results to a file
                stat("/tmp/temps.txt",&file_state);             // get the file state for sending via client_socket
                file_size = file_state.st_size;
                send(client_socket, &file_size, sizeof(int), 0);
                file_handler = open("/tmp/temps.txt", O_RDONLY); // now to open the file for read only
                sendfile(client_socket,file_handler,NULL,file_size);
                close(file_handler);
            } else if (strcmp(command, "bye") == 0) {
            	printf("Disconnected from %s port %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
            } else {
                printf("Ubdefined\n");
            }

            memset(&(buffer), '\0', 1024);
            recv_length = recv(client_socket, &buffer, 1024, 0);
            printf("recv_length RECEIVED: %d\n", recv_length);
        }
    }
    return 0;
}

void print_fatal_msg(char *msg) {
    printf("FATAL ERROR: %s\n", msg);
    exit(1);
}

char* get_file_name (char* full_url) {
  char *result; //result here
  char *ch; //define this
  ch = strtok(full_url, "/"); //first split
  while (ch != NULL) {
      result = ch;
      ch = strtok(NULL, "/");//next split
  }

  return result;
}
