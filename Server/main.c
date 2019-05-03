
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>


#include <arpa/inet.h>
#include <netdb.h>

#include <sys/select.h>

#define PORT 8080

#define USER_NAME_LENGTH 10
#define BACKLOG 10




//Game Structure and functions

struct game{ // Link list of game states

    int p1tag; // Unused
    int p2tag; //Unused
    int p1Sock;
    int p2Sock;
    int full; // Unused
    int turn;
    int count;
    int board[3][3];
    struct game *next;
};

struct client{ // client link list structure
    char name[USER_NAME_LENGTH];
    int sock;
    struct client *next;
};


const char* getname(int socket, struct client *list){ // Returns the name of the given inputted socket from the client list

    char name[USER_NAME_LENGTH];
    struct client *tmp;
    tmp = list;

    while((tmp != NULL) && (tmp->sock != socket)){
        tmp = tmp->next;
    }
    if (tmp == NULL){ // could not find matching socket
        printf("Error, getName: could not get name");
        exit(-1);
    }else{
        strncpy(name,tmp->name,USER_NAME_LENGTH);
    }

    return tmp->name;
}

int addplayer(int socket,struct game **g, struct client *list){ // Adds player to the given game list, transmits board and turn when game is filled

    int emptyboard[3][3] = {0};
    int randturn = (rand() % 2)+1;


    struct game *newgame;
    struct game *tmp;

    while((newgame = calloc(1,sizeof(struct game))) == NULL){
        printf("Error: Memory Allocation error while add player to game\n");
        exit(-1);
    }
    //Intialises values
    newgame->p1Sock =0;
    newgame->p2Sock =0;
    newgame->next = NULL;
    newgame->turn = 1;
    newgame->count = 0;

    char turnStr[1024];
    char name[11] = "";


    tmp = *g;
    if (*g == NULL){ // Check if no games

        newgame->p1Sock = socket;
        *g = newgame;
        return 0;

    }else if (tmp->p2Sock == 0){ // Check if first game is full
        tmp->p2Sock = socket;

        //Send who your matched against
        char matchStr[1024];
        sprintf(matchStr,"\nMatch Found! %s VS %s !\n",getname(tmp->p1Sock ,list),getname(tmp->p2Sock ,list));
        send(tmp->p1Sock,matchStr, sizeof(matchStr), 0);
        send(tmp->p2Sock,matchStr, sizeof(matchStr), 0);

        //Send Game
        sendGame(tmp);

        //Send who's turn it is
        if (tmp->turn == 1){
           strcat(name, getname(tmp->p1Sock ,list));
        }else{
            strcat(name, getname(tmp->p2Sock ,list));
        }
        sprintf(turnStr,"%s goes first!",name);
        send(tmp->p1Sock,turnStr, sizeof(turnStr), 0);
        send(tmp->p2Sock,turnStr, sizeof(turnStr), 0);

        free(newgame); // free newgame as it is unused
        return 0;

    }else{ // Search for non full game
        struct game *tmp;
        tmp = *g;
        while(tmp->next != NULL){ //Go to the end of the list
         tmp = tmp->next;
        }
            if (tmp->p2Sock != 0){//checks if last game is full

                //Is full, add a new game
                newgame->p1Sock = socket;
                tmp->next = newgame;

                return 0;
            }else{ // the last game in the list is not full

                tmp->p2Sock = socket;

                //Send who your matched against
                char matchStr[1024];

                sprintf(matchStr,"\nMatch Found! %s VS %s !\n",getname(tmp->p1Sock ,list),getname(tmp->p2Sock ,list));
                send(tmp->p1Sock,matchStr, sizeof(matchStr), 0);
                send(tmp->p2Sock,matchStr, sizeof(matchStr), 0);

                //Send Game
                sendGame(tmp);

                //Send who's turn it is
                if (tmp->turn == 1){
                   strcat(name, getname(tmp->p1Sock ,list));
                }else{
                    strcat(name, getname(tmp->p2Sock ,list));
                }

                sprintf(turnStr,"%s goes first!",name);
                send(tmp->p1Sock,turnStr, sizeof(turnStr), 0);
                send(tmp->p2Sock,turnStr, sizeof(turnStr), 0);

                free(newgame); // free newgame as it is unused
                return 0;
            }

    }
}



int resetgame(struct game *g, int socket){ // Unused, incorporated into makeMove Function

    //Search for game with matching socket

    //Create new game and add those sockets to that game

    //Replace game in link list

    //Return success

}



char idTochar(int i,int r, int c){// converts player index to char
    char naught = 'o';
    char cross = 'x';
    char space = ' ';
    if (i == 1){
        return naught;
    }else if( i == 2){
        return cross;
    }else{
        //int i = (r+1) + (c+1)*2;
        int i = (c+1) + (r*3);
        char str[5];
        char a;
        sprintf(str,"%d",i);
        a = str[0];
        return a;
    }
}


void sendGame(struct game *tmp){ //Creates ascii version of the game board and sends it to the clients

        //Transmit Board to each player
        char command[1024] = "/game"; // sent before board to indicate following transmission

        char board[1024] = "";
        //strcat(board,board1);
        char board1[50] = "     |     |     \n";
        strcat(board,board1);
        char board2[50];
        sprintf(board2,"  %c  |  %c  |  %c \n", idTochar(tmp->board[0][0],0,0),idTochar(tmp->board[0][1],0,1),idTochar(tmp->board[0][2],0,2));
        strcat(board,board2);
        //char board2[50] = "  %c  |  %c  |  %c \n";
        char board3[50] = "_____|_____|_____\n";
        strcat(board,board3);
        char board4[50] = "     |     |     \n";
        strcat(board,board4);
        //char board5[50] = "  %c  |  %c  |  %c \n";
        char board5[50];
        sprintf(board5,"  %c  |  %c  |  %c \n", idTochar(tmp->board[1][0],1,0),idTochar(tmp->board[1][1],1,1),idTochar(tmp->board[1][2],1,2));
        strcat(board,board5);
        char board6[50] = "_____|_____|_____\n";
        strcat(board,board6);
        char board7[50] = "     |     |     \n";
        strcat(board,board7);
        //char board8[50] = "  %c  |  %c  |  %c \n";
        char board8[50];
        sprintf(board8,"  %c  |  %c  |  %c \n", idTochar(tmp->board[2][0],2,0),idTochar(tmp->board[2][1],2,1),idTochar(tmp->board[2][2],2,2));
        strcat(board,board8);
        char board9[50] = "     |     |     \n";
        strcat(board,board9);
        send(tmp->p1Sock,board,sizeof(board),0);
        send(tmp->p2Sock,board,sizeof(board),0);

}

int checkWin(struct game *g){

    int j = 0;
    for(int i = 0;i<3;i++){ // go through each row/colum
        //Check rows
        if((g->board[i][j] == g->board[i][j+1]) && (g->board[i][j+1] == g->board[i][j+2])){ // check if line is the same
            if(g->board[i][j] !=0){ // check if its not an empty space
                return (g->board[i][j]); // return winning tag number
            }
        }
        //Check columns
        if((g->board[j][i] == g->board[j+1][i]) && (g->board[j+1][i] == g->board[j+2][i])){ // check if line is the same
            if(g->board[j][i] !=0){ // check if its not an empty space
                return (g->board[i][j]); // return winning tag number
            }
        }
    }

    //Check Diagonals

    if((g->board[0][0] == g->board[1][1]) && (g->board[1][1] == g->board[2][2])){
        if(g->board[0][0] != 0){
            return(g->board[0][0]);
        }
    }

    if((g->board[0][2] == g->board[1][1]) && (g->board[1][1] == g->board[2][0])){
        if(g->board[0][2] != 0){
            return(g->board[0][2]);
        }
    }

    return 0;
}



int placeMove(struct game g,int r, int c, int tag){ // returns 0 if invalid move
    if(g.board[r][c] != 0){
        return 0; // invalid move
    }else{
        g.board[r][c] = tag;
        return 1; // valid move
    }
}

void add(struct client **list){ // Unused Function
    if (*list == NULL){
        printf("list is null");
    }
    exit(-1);

}

int addclient(int socket, char *name, struct client **list){


    struct client *newClient;
    //Check if malloc successfull
    if ((newClient = malloc(sizeof(struct client))) == NULL){
        printf("Error: Memory Allocation error in add client\n");
        exit(-1);
    }

    //Copy username
    strncpy(newClient->name,name,USER_NAME_LENGTH);

    // Copy link list data
    newClient->sock = socket;
    newClient->next = NULL;

    struct client *tmp;
    //Check if your at end of the list
    if (*list == NULL){
        *list = newClient;

    }else{ // Find the end of the list
        tmp = *list;

        // Walk to end of the list
        while(tmp->next != NULL){
            tmp = tmp->next;

        }

        tmp->next = newClient;
    }

    return 0;
}

void removeclient(int socket, struct client **list){
    struct client *tmp;
    struct client *prev;
    tmp = *list;

    while ((tmp->next != NULL) && (tmp->sock != socket)){
        prev = tmp;
        tmp = tmp->next;

    }

    if (tmp == NULL){
        printf("Error removing client");
    }else if(prev == NULL){ // Check if at beginning of link list
        //Assign to the next entry in the list
        *list = tmp->next;
    }else{
        //Connect previous link to link after current link
        prev->next = tmp->next;
    }

}
void *getaddr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }else{
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
}

int updateName(char *name, int socket, struct client *list){ // This function is obsolete and has been replaced with checkName

    struct client *tmp;
    struct client *tmp2;
    tmp = list;

    while((tmp != NULL) && (tmp->sock != socket)){
        tmp = tmp->next;
    }
    if (tmp == NULL){ // could not find matching socket
        printf("Error Updating Name");
        exit(-1);
    }
    tmp2 = list; // Reset List
    //Check if name is available
    while ((tmp2 != NULL) && (strcmp(tmp->name, name) != 0)){
        tmp2 = tmp2->next;
    }
    if (tmp2 == NULL){// No name match
        strncpy(tmp->name,name,USER_NAME_LENGTH);
        printf("\nSocket %d name changed to %s\n", socket, name);
        return(0);
    }else{
        return(-1);
    }
}





int checkName(char *name, int socket, struct client *list){

    int status = 0;

    struct client *tmp;
    struct client *tmp2;
    tmp = list;

    while((tmp != NULL) && (tmp->sock != socket)){ // Search for matching socket
        tmp = tmp->next;
    }
    if (tmp == NULL){ // could not find matching socket
        printf("Error Updating Name");
        exit(-1);
    }

    tmp2 = list; // Reset List


    //Check if name is available
    while (tmp2 != NULL){
        int test = strcmp(name,tmp2->name);
        if ((socket != tmp2->sock) && (test == 0)){ // check if sockets dont match and name is the same
            status = 1;
            return(1);
        }
        tmp2 = tmp2->next;
    }
    printf("\nSocket %d name changed to %s\n", socket, name);
    strncpy(tmp->name,name,USER_NAME_LENGTH);
    return(0);

}


int makeMove(struct game *g,int r, int c, int socket, struct client *list){ // returns 0 if invalid move
    struct game *tmp;

    int tag = 0;

    int next; // next turn

    tmp = g;
    while(tmp != NULL){ // search game list for match sock
        if (tmp->p1Sock == socket){ // check sock1
            tag = 1;
            break;
        }else if (tmp->p2Sock == socket){ // check sock2
            tag = 2;
            break;
        }else{ // keep searching
            tmp = tmp->next;
        }
    }

    if (tmp == NULL){
        char retStr[1024]; // Return string
        sprintf(retStr,"You are currently not in a game, type /find to find a game.");
        send(socket,retStr, sizeof(retStr), 0);
        printf("\nmakeMove: error finding matching game for socket %d\n",socket);
        return 3; // User not found

    }
    if (tag != tmp->turn){
        return 2; // Not the senders turn
    }

    if(tmp->board[r][c] != 0){
        return 0; // invalid move
    }else{
        tmp->board[r][c] = tag;

        if (tmp->turn == 2){ // Change who's turn it is
            tmp->turn = 1;
            next = tmp->p1Sock;
        }else{
            tmp->turn = 2;
            next = tmp->p2Sock;
        }

        //Display Game State to players

       sendGame(tmp);

       //Tell who's turn it is now

        //Check if a win occured
        int win = checkWin(tmp);
        char winStr[1024];
        char name[11] = "";

        if (win > 0){ // Check if a win occured
            strcat(name, getname(socket,list));
            sprintf(winStr,"Game Over! %s Wins!\n\nNext Game starting Now!\n",name);
            send(tmp->p1Sock,winStr, sizeof(winStr), 0);
            send(tmp->p2Sock,winStr, sizeof(winStr), 0);

            //Wipe the board for next game
            int i,j;
            for(i = 0; i < 3;i++){ // Clear the board
                for(j = 0; j < 3; j++){
                    tmp->board[i][j] = 0;
                }
            }
            //Send wiped board
            tmp->count = 0;
            sendGame(tmp);


        }else if (tmp->count >= 8){// Draw has occured
            char drawStr[1024];
            sprintf(drawStr,"Game Over! It was a draw!");
            send(tmp->p1Sock,drawStr, sizeof(drawStr), 0);
            send(tmp->p2Sock,drawStr, sizeof(drawStr), 0);

            //Wipe the board for next game
            int i,j;
            for(i = 0; i < 3;i++){ // Clear the board
                for(j = 0; j < 3; j++){
                    tmp->board[i][j] = 0;
                }
            }
            //Reset turn counter
            tmp->count = 0;
            //Send new board
            sendGame(tmp);


        }else{ // non winning or drawing move
            tmp->count++;
        }

            //Send who's turn it is
            char turnStr[1024];
            sprintf(turnStr,"%s goes now!",getname(next,list));
            send(tmp->p1Sock,turnStr, sizeof(turnStr), 0);
            send(tmp->p2Sock,turnStr, sizeof(turnStr), 0);




        return 1; // valid move
    }
}

int endgame(struct game **g, int socket,struct client *list,int find){

    //Search for game with matching socket and store previous game
    struct game *tmp;
    struct game *prev;
    prev = NULL;
   // struct game *next;

    int tag = 0;

    tmp = *g;
    while(tmp != NULL){ // search game list for match sock
        if (tmp->p1Sock == socket){ // check sock1
            tag = 1;
            break;
        }else if (tmp->p2Sock == socket){ // check sock2
            tag = 2;
            break;
        }else{ // keep searching
            prev = tmp;
            tmp = tmp->next;
        }

    }

    if (tmp == NULL){ // failed to find matching game
            if (find != 1){ // Check if endgame call is coming from /find command
                printf("\nEndGame: Failed to find game to end.\n");
                char resp[1024] = "You are not in a game currently!";
                send(socket,resp, sizeof(resp), 0);
            }
        return 3; // no matching game found
    }

    //Message both sockets that the game is ended
    char resnStr[1024];
    char name[11] = "";
    strcat(name, getname(socket,list));
    sprintf(resnStr,"%s has resigned from the game! type /find to find a new game.",name);
    send(tmp->p1Sock,resnStr, sizeof(resnStr), 0);
    send(tmp->p2Sock,resnStr, sizeof(resnStr), 0);

    // Delete Game


    if (prev != NULL){
       prev->next = tmp->next;
    }else if (tmp->next != NULL){
        *g = tmp->next;
        free(tmp);
    }else{
        free(*g);
        *g = NULL;
    }

    return(0);

}


int main(void){

    //client list
    struct client *clients = NULL; // link list of clients

    char buffer[1024] = {0};

    socklen_t addr_size;
    char addr_str[INET6_ADDRSTRLEN];
    void *addr;

    struct sockaddr_in *ipv4; // server ipv4 addr
    struct sockaddr_in6 *ipv6; // server ipv6 addr

    int s;
    int new_socket;
    int status;
	struct addrinfo hints;
	struct addrinfo *res; // will point to the results
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me    if (tmp == NULL){ // could not find matching socket

	if ((status = getaddrinfo(NULL, "8080", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

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


	s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    inet_ntop(res->ai_family,addr,addr_str,sizeof addr_str);
    printf("Server Address: %s\n", addr_str);

    if (bind(s,res->ai_addr,res->ai_addrlen) == -1){
        fprintf(stderr,"Port binding Error. Exiting!\n");
        exit(-1);
    }

    //Listen
    if (listen(s, BACKLOG) == -1){
        fprintf(stderr, "Error listening to port. Exitin!");
        exit(-1);
    }


    ////FDS variables
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int newfd;
    struct sockaddr_storage remoteaddr; // Connecting client Adrress
    socklen_t addrlen;
    int nbytes;


    int update;

    //Game Data

    struct game *games = NULL;
    int gc = 0; // game counter


    FD_SET(s,&master);

    fdmax = s;

    //Select Loop
    while(1){

        read_fds = master;

        if (select(fdmax+1, &read_fds,NULL,NULL,NULL) == -1){
            printf("Selection Error");
            exit(-1);
        }
        for(int i = 0; i<= fdmax; i++){
            if(FD_ISSET(i,&read_fds)){
                if (i == s){
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(s,(struct sockaddr *) &remoteaddr,&addrlen);

                    if (newfd == -1){
                        printf("Error accepting in select");
                    }else{
                        FD_SET(newfd,&master);
                        inet_ntop(remoteaddr.ss_family, getaddr((struct sockaddr*)&remoteaddr), addr_str, sizeof(addr_str));
                        printf("Select, New Connection from: %s \n", addr_str);
                        // Add the newfd to fdmax
                        if(newfd > fdmax){
                            fdmax = newfd;

                        }

                        addclient(newfd,"default",&clients);

                    }

                }else{
                    // handle data from a client
                    if ((nbytes = recv(i, buffer, sizeof buffer, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("Selectserver: Socket %d disconnected.\n", i);
                        } else {
                            perror("receive");
                        }
                        //Close game if there in a game
                        endgame(&games,i,clients,0);
                        //Remove from select set
                        close(i);
                        FD_CLR(i, &master); // remove from master set
                    } else {


                        char *newline = strchr( buffer, '\n' );
                        if ( newline )
                        *newline = 0;

                        char test[20];
                        strcpy(test,buffer);
                        char *pch;
                        int len1;

                        pch = strtok(test," ");
                        len1 = strlen(pch);

                        //Command Checks, for name, game, resn (resign), help and find game

                        if  (strcmp("/name",pch) == 0){

                                    pch = strtok(NULL," ");
                                    len1 = strlen(pch);

                                    //Check if name taken


                                    // If the name fails to update terminate connection
                                    if((update = checkName(pch,i,clients)) != 0) {

                                        //Send reason for disconnect
                                        char resp[1024] = "You have been kicked for choosing an already taken name!"; // Kicking if a conflicting name occurs was a requirement of the assignment
                                        send(i,resp, sizeof(resp), 0);

                                        //Remove from select set and close socket
                                        close(i);
                                        FD_CLR(i,&master);
                                        //Close game if there in a game
                                        endgame(&games,i,clients,0);
                                    }

                        }else if(strcmp("/game",pch) == 0){ //example: /game 0 1
                            int n = -1;
                            int r,c ;
                            char tmp[1024];

                            sscanf(buffer,"%s %d", tmp ,&n);
                            n = n-1;
                            r = n / 3;
                            c = n % 3;

                            if(r <= -1 || c <= -1 ){
                                printf("Error Reading game command");
                                char resp[1024] = "Invalid Game command! Error Reading";
                                send(i,resp, sizeof(resp), 0);
                                break;

                            }
                            if (r <0 || c <0 || c>2 || r>2){
                                printf("Invalid input range for command /game");
                                char resp[1024] = "Invalid Game command!";
                                send(i,resp, sizeof(resp), 0);
                                break;

                            }
                            int move = makeMove(games,r,c,i,clients);
                            if (move == 2){
                                printf("Game: Not your turn!");
                                char resp[1024] = "Game Server: Not your Turn!";
                                send(i,resp, sizeof(resp), 0);
                                // Send message to sender telling them that
                            }else if(move == 0){
                                printf("Game: That spot is taken!");
                                // Send message saying that spot is taken already
                                char resp[1024] = "Game Server: That spot is taken! Choose Another.";
                                send(i,resp, sizeof(resp), 0);
                            }else{
                            }

                        }else if(strcmp("/resn",pch) == 0){
                            endgame(&games,i,clients,0);

                        }else if(strcmp("/find",pch) == 0){ // resigns from game and finds a new one
                            endgame(&games,i,clients,1); // resigns if currently in a game
                            addplayer(i,&games,clients);
                        }else if(strcmp("/help",pch) == 0){
                            char resp[1024] = "Command List\n";
                            strcat(resp,"/find  finds a game to play on\n");
                            strcat(resp,"/resn  resigns you from your current game\n");
                            strcat(resp,"/game  this command followed by a number between 1 and 9 will make your game move \n");
                            strcat(resp,"/name  changes your username to what you input after /name, Choosing an existing name will kick you from the server! \n"); // assignment requirement
                            strcat(resp,"/quit  exits you from the server \n");
                            strcat(resp,"\n Enter messages you want to send to other users by typing without a command\n");
                            send(i,resp, sizeof(resp), 0);

                        }else{
                            // Append sender to message
                            char name[10];
                            strcpy(name,getname(i,clients));
                            strcat(name,": ");
                            strcat(name,buffer);

                        for(int j = 0; j <= fdmax; j++) { // Go through all sockets

                            if (FD_ISSET(j, &master)) { //Check if there open sockets
                                //Send combined message to all users that are
                                if (j != s && j != i) {
                                    if (send(j, name, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }

                        }
                    }
                }
            }
        }
    }
}
