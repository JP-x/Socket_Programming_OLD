/*

* Fall 2013

*

* First Name: Jonathan

* Last Name: Padilla

* Username:  jpadi004

* email address: jpadi004@ucr.edu

*

*

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string> 
#include <iostream>
#include <sys/wait.h>

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <pwd.h>
#include <grp.h>

#define MAXDATASIZE 150
#define BACKLOG 10

using namespace std;

void setup(int & sockfd, struct sockaddr_in &cur_addr, int & port)
{
    //create socket and return descriptor. fills in the local host and protocol part of 5tuple
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket() error!");
        exit(1);
    }
    else
       printf("socket() successful...\n");
    //host byte order
    cur_addr.sin_family = AF_INET;
    //short network byte order
    cur_addr.sin_port = htons(port);
    //auto fill with my IP
    cur_addr.sin_addr.s_addr = INADDR_ANY;
    //zero rest of struct
    memset(&(cur_addr.sin_zero), 0, 8);
    //bind - associates a local addres to an unnamed socket
    if(bind(sockfd, (struct sockaddr *) &cur_addr, sizeof(struct sockaddr)) == -1){
        perror("error: bind()\n");
        exit(1);
    }
    else
        cout << "bind() successful..." << endl;
}

//recvs first information from client which is the number of arguments
void recvNum(int & numargs, int & newfd)
{
    int numbytes = 0;
    char *buf;
    numbytes = recv(newfd, &numargs, 4, 0);
    if( numbytes == -1){
        perror("recv() failed to receive number of arguments");
        exit(1);
    }
    else{
       cout << "Server: received " << numbytes << " bytes: "
             << numargs << endl;
    }
    
    return;
}

//receive one argument from client and return pointer
char* recv_arg(const int & newfd)
{
     char * buf;
     int numbytes = 0;
     numbytes = recv(newfd, buf, MAXDATASIZE-1, 0);
     if(numbytes == -1){
        perror("recv()");
        exit(1);
     }
     else
     {
        printf("Server: recv() successful...\n");
        buf[numbytes] = '\0';
        cout << "Server: received " << numbytes << " bytes: " << buf << endl;
     }
     //sleep(1);
   return buf;
}

//go through each entry in a directory and 
//send files over to client
void send_files(char* args[], int & new_fd)
{
  int child_id;
  int child_stat= 0;
  int statt;
  int i = 0;
  DIR *dir;
  struct stat statbuf;
  string dir_name;
  struct dirent *direntp;
  //char *to_exec[4] = {NULL};
  //to_exec[0] = "ls";
  //to_exec[1] = "-l";
while(1)
 {
  if((child_id = fork()) == 0){
      //check if valid directory
      //cout << "args[" << i << "]: " << args[i] << endl;
     
     
     //user can handle creating directory
     //send filename followed 
     // send FILENAME: asdasdasd
     //user takes into buf this arg
     //creates file based off of this and then
     //populates it with data until 
     //next buf reads END:
     //if done close fstream and then move on to taking in next file
      if((dir = opendir(args[i])) == NULL){
          perror("opendir()");
          closedir(dir);
          return;
      }
    
      while((direntp = readdir(dir)))
      {
          string arpath = args[i];
          string pathway = arpath + "/" + direntp->d_name;
          stat(pathway.c_str(), &statbuf);
          if(!(S_ISDIR(statbuf.st_mode)))
          {
             //
             //cout << "pathway: " << pathway << endl;
             //0 for RD_ONLY
             int in_fd = open(pathway.c_str(), 0);
             string dname = direntp->d_name;
             string total_name = "FILENAME:" + dname;
            // cout << "TOTAL NAME IS: " << total_name << endl;
             char* to_send = (char*)total_name.c_str();
             write(new_fd, to_send, 100);
             sleep(1);
                
             sendfile(new_fd,in_fd, NULL, 25000);
             close(in_fd);
             sleep(1);
          }

       }
       
       closedir(dir);
       return;
   }
   else
   {
       //parent
       wait(NULL);
       i++;
       //if next argument is NULL return
       if(args[i] == NULL)
       {
           string mes_endClient = ":END:";
           char* final_mes = (char*)mes_endClient.c_str();
           write(new_fd, final_mes, 100);
           sleep(1);
           return;
       }
      
      //sleep before sending next batch 
       sleep(2);
   }
  }
}


////////MAIN///////////////////////////////
int main(int argc, char*  argv[] )
{
    //to be used with recvArgs function
    int num_args = 0;
    //isten on sockfd, new connection on new_fd
    int sockfd, new_fd;
    //current address info, address of cur program
    struct sockaddr_in cur_addr;
    //remote address information
    struct sockaddr_in rem_addr;
    int sin_size;
    int port;

    if(argc != 2){
        cout << "Server Usage: " << argv[0] << " [port#] " << endl;
        exit(1);
    }

    port = strtol(argv[1], (char **)NULL, 10);
    if(port < 1024){
        printf("Error: Port must be an int >= 1024\n");
    }
    //setup socket and whatnot
    setup(sockfd,cur_addr,port);
    if(listen(sockfd, BACKLOG) == -1){
        perror("ERROR: listen()");
        exit(1);
    }
    else
        printf("listen() successful...\n");

   sin_size = sizeof(struct sockaddr_in);

//////////////////////////////////////////////////////////////////
  int pid = 0;


   while(1)
   {
    printf("going to block via accept()....\n");
    new_fd = accept(sockfd, (struct sockaddr*)&rem_addr,(socklen_t *)&sin_size);
    cout << "WAITING" << endl;
    if(new_fd == -1){
        perror("ERROR: accept()");
    }
        else
            printf("accept() successful...\n");
   
      if((pid = fork()) == 0)
      {
          //
          break;
      }
     

    }
    

    
    // receive num of arguments:
  
    recvNum(num_args, new_fd);
    //create array to take in arguments
    char* args[num_args+1];
    args[num_args] = NULL;
    sleep(1);
    //take in arguments one by one and populate array
   for(int i = 0 ; i < num_args; i++)
   {
    args[i] = recv_arg(new_fd);
    sleep(1);
    //cout << "Args[" << i << "]: " << args[i] << endl;
   }
   //send results of ls to client
    send_files(args, new_fd);
    //close sockets
    close(new_fd);
    
 ////////////////////////////////////////////////////////////////////
   
   
    close(sockfd);
    printf("Server exiting\n");
    cout << flush;
    
    //weird bug argc cannot access address xsomethingsomething
    //I'm not accessing anything with argc after this point 
    exit(0);
        return 0;
}
