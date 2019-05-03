
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <sys/time.h>

#define STDIN 0

#define PORT 8080

#define MAXDATA 255

#define USER_NAME_LENGTH 10
#define IPV4_Length 16
#define IPV6_Length 39

//Functions

int validIpv4Str(char *address){ //Checks if valid ipv4 address
    struct sockaddr_in sa;
    int check = inet_pton(AF_INET,address,&(sa.sin_addr));
    return check;
}

int validIpv6Str(char *address){//Checks if valid ipv6 address
    struct sockaddr_in sa;
    int check = inet_pton(AF_INET6,address,&(sa.sin_addr));
    return check;
}

//Main
int main(int argc, char const *argv[])
{

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char buffer2[1024] ={0};

    if (argc != 3){
        printf("Error!, invalid command line input, expected ip address and username.");
        exit(-1);
    }

    // Parse command line name, limits size to 10 chars
    char name[USER_NAME_LENGTH];
    strncpy(name,argv[2],USER_NAME_LENGTH);

    // Parse command address, limits address to ipv6 length
    char caddr[IPV6_Length]; // command line address
    strncpy(caddr,argv[1],IPV6_Length);

    // Check if address is valid ipv4 or ipv6
    if(validIpv4Str(caddr) != 1 ){
        printf("Error!, Invalid IP address.");
        exit(-1);
    }


    //Setup Socket

    socklen_t addr_size;
    char addr_str[INET6_ADDRSTRLEN];
    void *addr;

    struct sockaddr_in *ipv4; // server ipv4 addr
    struct sockaddr_in6 *ipv6; // server ipv6 addr

    int status, sockfd;
	struct addrinfo hints;
	struct addrinfo *res; // will point to the results
	struct addrinfo *l; // current res link
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets



	//Get the address info
	if ((status = getaddrinfo(caddr, "8080", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	//Traverse res link list
	for(l = res; l!=NULL; l = l->ai_next){
        if ((sockfd = socket(l->ai_family, l->ai_socktype, l->ai_protocol)) == -1) {
			perror("client: socket"); continue;
		}

		if (connect(sockfd, l->ai_addr, l->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect"); continue;
		}
		break; // we got a valid socket, so stop searching
	}


    //Check if connection was successful
    if (l == NULL){
        printf("Error!, Failure to connect to client.\n");
        exit(-1);
    }


    // Check address type and store appropriate result
    if (res->ai_family == AF_INET){ // check ipv4
        ipv4 = (struct sockaddr_in*) res->ai_addr;
        addr = &(ipv4->sin_addr);
    }else if (res->ai_family == AF_INET6){ // check ipv6
        ipv6 = (struct sockaddr_in6*) res->ai_addr;
        addr = &(ipv6->sin6_addr);

    }else{ // check if other ip family, this is an error state
        fprintf(stderr,"Unknown IP family");
        exit(-1);
    }


    //Convert address and display it
    inet_ntop(l->ai_family,addr,addr_str,sizeof addr_str);
    printf("Client Connected to Address: %s\n", addr_str);

    free(l);

    //Display Welcome Messages
    printf("Welcome to the chat server, %s!  type /help for a list of commands. \n",name);


    char sendName[USER_NAME_LENGTH + 6];
    sprintf(sendName,"/name %s",name);

    //Send name to server
    send(sockfd , sendName , USER_NAME_LENGTH + 6 , 0 );

    //Send And Receive Loop



    fd_set master;
    fd_set read_fds;
    int fdmax;



    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    // add socket to master set
    FD_SET(STDIN,&master);
    FD_SET(sockfd,&master);
    fdmax = sockfd;
    int flags;
    int nbytes;

    char command[6] = {0};
    while(1){
        read_fds=master;

        if(select(fdmax+1,&read_fds,NULL,NULL,NULL) == -1){
            printf("Select Error.");
            exit(-1);
        }

        for(int i =0; i<=fdmax;i++){

            if(FD_ISSET(i,&read_fds)){ // File descriptor change
                if (i==STDIN){
                    fgets(buffer2,1024,stdin);
                    strncpy(command,buffer2,5);
                    if (strcmp("/quit",command) == 0){

                        int x = close(sockfd);
                        printf("Exiting!");
                        exit(0);

                    }else if(strlen(buffer2) < 2) {
                            //Continue as buffer is empty
                    }else{
                        send(sockfd , buffer2 , 1024 , 0 );
                    }
                }else{
                    if(nbytes = recv(sockfd,buffer,1024,flags) <=0){
                        if (nbytes==0){
                            printf("Server Disconnected\n");
                        }else{
                            printf("Server Disconnected\n");
                        }
                        close(i);
                        FD_CLR(i,&master);


                    }else{
                        printf("%s \n",buffer );
                    }
                }
            }
        }
    }
}
