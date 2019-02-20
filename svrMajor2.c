//svr v2 (with threads)
/*
    TO-DO
    Beautify
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<unistd.h>
#include<pthread.h>

void *connection_handler(void *fd);

static int totals[2];
static int fds[2] = {-1, -1}; //act like an fdset tbh



int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("usage: ./executable port\n");
        exit(0);
    }
    int sockfd, clientfd;
    int portno;
    int clilen;

    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket fail");
        exit(0);
    }

        portno = atoi(argv[1]); //get port 

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);

//bind
        int on = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("bind fail");
        }

//listen
         listen(sockfd, 2);

        //accept connections

        clilen = sizeof(struct sockaddr_in);

        //PTHREADS
        pthread_t thread;
        int i;

        int sendport;
        int cli_reconnect;
        printf("[server]: Awaiting Connections...\n");
        while((clientfd = accept(sockfd, (struct sockaddr*) &cli_addr, (socklen_t*) &clilen))) //just keep waiting for new clients to connect
        {
            printf("[server]: Connection accepted\n");

            if(pthread_create(&thread, NULL, connection_handler, (void*) &clientfd) < 0) //make a thread, tell it the file descriptor the client is on
            {
                perror("thread fail");
                exit(0);
            }
            puts("[server]: Handler Assigned\n"); //if this isn't puts() other output doesn't work anymore.

            for(i = 0; i < 2; i++) //i is an index for both totals[] and fds[]. If there's something in totals[n], it came from fds[n]. This whole section might need to be moved to the thread.
            {
                if(totals[i] != 0)
                {
                    if(i == 1)
                    {
                        //send port info (totals[i]) to client in fds[i-1]
                    }else
                    {
                        //send port info (totals[i]) to client in fds[i+1]
                    }
                }
            }
        }


}

void *connection_handler(void *fd)
{
    int client;
    for(client = 0; client < 2; client++) //make main() aware that we are an open connection
    {
        if(fds[client] == -1)
        {
            fds[client] = *(int*) fd;
            break;
        }
    } //this LOOKS redundant and unecessary, but it's how clients can reconnect into readable slots, so don't break it
    //or, idk, change it to a 2D array if you MUST
    int total = 0;
    
    int sockfd = *(int*) fd; //store our file descriptor as something easier to type
    int rsize = 69; //return value for recv, use as end marker for buffer

    char buffer[32];
    //sprintf(buffer, "%d", client+1); //tell client which client they are :|
    
    if((client+1) <= 2)
    {
        sprintf(buffer, "%d", client+1); //tell client which client they are :|
        
    }else //tell the client to LEAVE
    {
        sprintf(buffer, "FULL");
    }
    
    send(sockfd, buffer, 32, 0);
    while(rsize != 0)
    {
        rsize = recv(sockfd, buffer, 32, MSG_WAITALL);
        if(rsize == 0) //if the client closed (do it right here so none of that other stuff happens)
        {
            close(sockfd);
            fds[client] = -1;
            totals[client] = 0;
            printf("[client %d]: connection closed\n", client+1);
            break;
        }
        buffer[rsize] = '\0'; //mark the end of the buffer for printf()
        printf("[client %d]: %s ",client+1, buffer);

        total = total + atoi(buffer);
        printf("-- total: %d\n", total);
        sprintf(buffer, "%d", total); //update client on its total
        send(sockfd, buffer, 32, 0);
        
        int status = 0;
        if(fds[0] != -1 && fds[1] != -1)
        {
            status = 1;
        }
        if(total > 49151)
        {
            total = 0;
        }

        if(total >= 1024 && total <= 49151 && status == 1) //what if we only tell main() what the clients' totals are when they're in range, and just let the threads keep track of them until then?
        {
            totals[client] = total;
            sprintf(buffer, "%s %d", "PORT", total);
            printf("[server]: Sending [client %d] Port to [client ",client+1);
            int i;
            for(i = 0; i < 2; i++) //update the OTHER client, tell it to connect to first client
            {
                if(totals[i] == 0)
                {
                    if(i == 1)
                    {
                        printf("%d] (code: %s)\n", i+1, buffer);
                        send(fds[i], buffer, 32, 0);
                        sprintf(buffer, "SERV");
                        send(fds[i-1], buffer, 32, 0);
                    }else
                    {
                        printf("%d] (code: %s)\n", i+1, buffer);
                        send(fds[i], buffer, 32, 0);
                        sprintf(buffer, "SERV");
                        send(fds[i+1], buffer, 32, 0);
                    }
                }
            }
                        totals[0] = 0;
                        totals[1] = 0;
                        total = 0;
                        printf("[server]: Totals Reset\n");
        }else if(total >= 1024 && total <= 49151 && status == 0)
        {
            printf("[server]: Only one client connected. Total resetting...\n");
            sprintf(buffer, "NOPE");
            send(sockfd, buffer, 32, 0);
            total = 0;
        };
    }
}