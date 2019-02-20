//client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <errno.h>

static char* rem_ip;

void *server_talker(void* fd);

int main(int argc, char* argv[])
{
    int sockfd, portno, n; //socketfd, port #, holder
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[32];

    if(argc < 4)
    {
        printf("usage: ./executable hostname port remoteip\n");
        exit(0);
    }

    portno = atoi(argv[2]); //port
    rem_ip = argv[3];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket error");
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if(server == NULL)
    {
        printf("Host not found!\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server-> h_length);
    serv_addr.sin_port = htons(portno);
//make threads
    pthread_t thread;

    if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        exit(0);
    }
    else
    {
        if(pthread_create(&thread, NULL, server_talker, (void*) &sockfd) < 0) //make a thread, tell it the file descriptor the server is on
        {
            perror("thread fail");
            exit(0);
        }


    }
    char retval[32];
    pthread_join(thread,(void**) &retval); //pretty much does nothing
    printf("[this client]: cool bye\n");
    exit(0);
}

void *server_talker(void* fd)
{
    int rsize = 69;
    int total;
    int sockfd = *(int*) fd;
    char buffer[32];
    char port_from_buffer[32];

    int new_port;
    int sockfd2;

    int clino;

    struct sockaddr_in serv_addr2, cli_addr;  

    rsize = recv(sockfd, buffer, 32, MSG_WAITALL); //get the client number :| (or a LEAVE message)

    if(strncmp(buffer, "FULL", 4) != 0)
    {
        clino = atoi(buffer);
        printf("[client %d]: connected\n", clino);
        
    }else
    {
        printf("[server]; Server Full\n");
        rsize = 0; //skip the whole loop
    }

    
    while(rsize != 0)
    {
        printf("[client %d]: Enter client data: ", clino);
        scanf("%s", buffer);
        if(strcmp(buffer, "0") == 0)
        {
            //sprintf(buffer, "exit");
            break;
            //this should send rsize of 0 to server, server handles that fine.
        }

        send(sockfd, buffer, 32, 0);
        rsize = recv(sockfd, buffer, 32, MSG_WAITALL);
        //buffer[rsize] = '\0';
        if(strncmp(buffer, "PORT", 4) == 0)
        {

            
            strcpy(port_from_buffer, &buffer[5]); //buffer[5] is where the portno starts
                                          //printf("new port = %s\n", new_port);
            new_port = atoi(port_from_buffer);
            
            rsize = recv(sockfd, buffer, 32, MSG_WAITALL); //get our total real quick
            total = atoi(buffer);
            printf("[server]: Total = %d\n", total);
            printf("[client %d]: disconnecting from server...\n", clino);
            printf("[client %d]: reconnecting on port %d\n", clino, new_port);            
            close(sockfd); //client disconnects from server, 
            sockfd2 = socket(AF_INET, SOCK_STREAM, 0); //and reconnects to the other client, treating it like a server.
            serv_addr2.sin_family = AF_INET;
//            serv_addr2.sin_addr.s_addr = INADDR_ANY;
            serv_addr2.sin_addr.s_addr = inet_addr(rem_ip);
            serv_addr2.sin_port = htons(new_port); //Use the port number we got from the other client, through the server

            if(connect(sockfd2, (struct sockaddr*) &serv_addr2, sizeof(serv_addr2)) < 0)
            {
                perror("connect");
                exit(0);
            }
            sprintf(buffer, "%d", total);
            send(sockfd2, buffer, 32, 0);
            close(sockfd2);
            break;
        }

        printf("[server]: Total = %d\n", atoi(buffer));
        total = atoi(buffer);
        if(total > 49151)
        {
            printf("[server]: Total max reached, rolling over...\n");
        }
        if(total >= 1024 && total <= 49151)
        {
            recv(sockfd, buffer, 32, MSG_WAITALL);
            if(strncmp(buffer, "NOPE", 4)==0)
            {
                //do nothing. This doesn't need to be here. Could do something fun if we wanted.

            }
        }
        //total = 0;


        //if(total >= 1024 && total <= 49151) //If we are in the port range, become a server
        if(strncmp(buffer, "SERV", 4) == 0)        
        {

            sockfd2 = socket( AF_INET, SOCK_STREAM, 0); //Use a new socket so we don't lose the connection with the og server
            if(sockfd < 0)
            {
                perror("socket fail");
                exit(0);
            }

            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_addr.s_addr = INADDR_ANY;
            serv_addr2.sin_port = htons(total); //set our port to be 

            if(bind(sockfd2, (struct sockaddr*) &serv_addr2, sizeof(serv_addr2)) < 0)
            {
                printf("[client %d]: tried to bind to port %d\n",clino, total);
                perror("bind fail");
                break;
            }
        listen(sockfd2, 1);
        printf("[client %d]: listening...\n", clino);
        int clilen = sizeof(struct sockaddr_in);
        int clientfd;
        if((clientfd = accept(sockfd2, (struct sockaddr*) &cli_addr, (socklen_t*) &clilen)) <= 0)
        {
            perror("accept");
        }
        //printf("plswork = %d\n", clientfd);
        printf("[client %d]: other client connected\n", clino);

        rsize = recv(clientfd, buffer, 32, MSG_WAITALL);
        if(rsize <= 0)
        {
            perror("rsize");
            exit(0);
        }

        int total2 = atoi(buffer);
        printf("[other client]: Total = %d\n", total2);

        

//             break;
        }
    }   
    pthread_exit((void*) buffer);
}


//EAT = (1-p) * (MAT) + p * (page fault service time)

//PFST is the fault overhead, time needed for page fault exception time. Time needed to swap the pages in and out between main memory and secondary memory, and the restart overhead.

