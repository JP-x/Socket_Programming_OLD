/*

*  Fall 2013

*

* First Name: Jonathan

* Last Name: Padilla

* Username:  jpadi004

* email address: jpadi004@ucr.edu

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
#include <iostream>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <pthread.h>


#define MAXDATA 25000


using namespace std;

pthread_mutex_t copy_mutex;

const int NUM_THREADS = 10;
//struct thread_id contains all needed info to call on each of the functions below
//to be placed into an array so each thread has its own unique struct and information
struct thread_id{

    int tid;
    string thread_dir; // + "[id]" + "Files";
    
    struct hostent *hosten;
    int sockfd, numbytes;
    char buf[MAXDATA];
    struct sockaddr_in host_addr;
    int port;
    int argc;
    char* args[10];
};

//will be used with each thread. Each thread will have its own specified struct thread_id
    struct thread_id thread_data_array[NUM_THREADS];
//will need for each seperate thread; 
//access to all structs created.
//to be used by each of the functions below
// for directories to create (thread1files, thread2files, ... etc)
// structs used will contain thread ids and the name of the directory to create
//struct thread_data thread_data_array[NUM_THREADS];
//passes in hostent pointer to fill with information about host
//also sets up other sin options in struct sockaddr
void get_host(struct hostent  *hosen , struct sockaddr_in  &hos_addr, int &port, int &sockfd, char* args[] ){
    //check to see if valid host
    if((hosen = gethostbyname(args[1])) == NULL){
        cout << "ERROR" << endl;
        perror("gethostbyname()");
        exit(EXIT_FAILURE);
    }
    else
        cout << "Host found: " << hosen->h_name;

    //get sockfd
    if((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1){
        perror("socket()");
        exit(1);
    }
    else
        cout << "Client- socket() sockfd successful..." << endl;
    
    //populate host_addr with needed data
    hos_addr.sin_family = AF_INET;
    cout << "Server - using " << hosen->h_name << " and port " << port << endl;
    //htons coverts an unsigned short int from host byte order to network byte order
    hos_addr.sin_port = htons(port);
    //cast to in_addr * then dereference
    hos_addr.sin_addr = *((struct in_addr *)hosen->h_addr);
    //zero rest of struct
    memset(&(hos_addr.sin_zero), '\0', 8);
    
    return; 
}

//sends all args to host server ....
void send_args(int hostfd, int start, int end, char* argv[])
{
    // send num for server to use to know how many args to take in
    int num_args = end-start;
    //first send number of arguments to server so can receive arguments properly;
    if(write(hostfd, &num_args, sizeof(num_args)) == -1){
        perror("write() error! Could not send num of args.");
        exit(1);
     }
     else
         cout << "write() successful. Argument number sent..." << endl << flush;

  //send each arg one at a time
  for(int i = start; i < end; i++)
  {
     // cout << "SENDING: " << argv[i] << endl << flush;
   if(write(hostfd, argv[i], strlen(argv[i])) == -1){
     perror("write() error!");
         exit(1);
      }
      else
          cout << "write() successful..." << endl << flush;
      //sleep before sending next arg
      sleep(2);
   }
   return;
}

//makes file
//inserts current buffer into file
void make_file(const string &complete_path, const char* buf)
{
         ofstream file;
         file.open(complete_path.c_str());
         file << buf;
         file.close();
}

//recv from server  files contained in the directory
//recv file name then create file via ofstream
//populate file with data in buf
//fills specified thread directory
void recv_files3(const int & sockfd, const struct thread_id & cur_thread ){
    //thread_id.dir_name will be used as the directory to fill in thsi case
   char buf[MAXDATA];
   int numbytes = 1;
   while(numbytes != 0){
        numbytes = recv(sockfd, buf, MAXDATA-1, 0);
        sleep(1);
       if(numbytes == -1){
          perror("recv()");
         exit(1);
        }
      else
      {
        //do not print out results if no bytes received
        if(numbytes != 0){
         printf("Client: recv() successful...\n");
         buf[numbytes] = '\0';
         cout << "Client: received filename" << numbytes << "bytes\n";// << buf << endl;
         //now have file name; must recv again in order to get rest of info
         //to place into file going to open
         string filename = buf;
         string file_to_create;
         if(filename.find("FILENAME:") != -1){
            int index_col = filename.find(':');
            file_to_create = filename.substr(index_col+1);
            //cout << "file to create : " << file_to_create;
         }
         //return if have reached end of files to receive 
         if(filename.find(":END:") != -1)
             return;

         numbytes = recv(sockfd, buf, MAXDATA-1,0);
         sleep(1);
                 if(numbytes == -1){
                     perror("recv()");
                     exit(1);
                     }
                 else{
                 //do not print out results if no bytes received
                 if(numbytes != 0)
                 {
                   printf("Client: recv() successful...\n");
                   buf[numbytes] = '\0';
                   cout << "Client: received filedata " << numbytes << "bytes\n";// << buf << endl;
                 }

                 }
        //create file from received info 
        string  complete_path = "./" + cur_thread.thread_dir + "/" + file_to_create;
        make_file(complete_path, buf);
        }
      }

   }
}



//creates string with thread directory to create
//then creates directory in same folder
string make_thr_directory(const int & tid)
{
   char tid_c = (char)(((int)'0')+tid);
   string t = "Thread";
   string f = "files";
   string dname_to_return = t + tid_c + f;
   //dname to return =   [ Thread1files ]
   string dir_to_create = "./" + dname_to_return;
   //create directory in current directory 
   mkdir(dir_to_create.c_str(), S_IRWXU);// | S_IRWXG | S_IROTH | S_IXOTHa);
   //return thread directory name
   return dname_to_return;
}


//function to be called on by pthread create
//goes through each function to make each of the files
void *copy_files(void * thread)
{
    int start_index = 3;
   //now current thread_id struct contains info such as 
   //#thread , port # a hosten * socketfd , and the directory of the thread
   struct thread_id cur_thread = *(thread_id*) thread;
   
   //tests to see if is valid hosts and continues to set up ports/addresses w/sockaddr_in
   get_host(cur_thread.hosten, cur_thread.host_addr, cur_thread.port, cur_thread.sockfd, cur_thread.args);
   //connect
   if(connect(cur_thread.sockfd, (struct sockaddr *)& cur_thread.host_addr, sizeof(struct sockaddr)) == -1){
         perror("connect()");
         exit(1);
     }
     else
         cout << "Client - connect() successful..." << endl;

   //now connected send args to client
   send_args(cur_thread.sockfd, start_index, cur_thread.argc, cur_thread.args);
    
    //call on recv_files to... receive files
    recv_files3(cur_thread.sockfd, cur_thread);
    sleep(1);
    close(cur_thread.sockfd);
     cout << "Client Exiting" << endl;
   
}



int main(int argc, char* argv[]){
    //struct thread_id thread_data_array[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    struct hostent *hosten;
    int sockfd, numbytes;
    char buf[MAXDATA];
    struct sockaddr_in host_addr;
    int port;
    int rc;
   //takes hostname argv[1]
   //and list of direcory and file argument argv[2] ---- argv[n]
   if(argc < 3) 
   {
       cout << "Invalid command input." << endl;
       cout << "Format is: client [hostname] [port] [directories and files] ... ... " << endl;
       return 0;
   }
   //strtol converts string to a long int
   port = strtol(argv[2], (char**)NULL, 10);
   if(port < 1024){
       cout << "Port must be an integer >= 1024" << endl;
       exit(1);
   } 
    //initialize values in each struct to be used by the copy_files function
    for(long  t = 0 ; t< NUM_THREADS ; t++){
        thread_data_array[t].tid = t;
        thread_data_array[t].thread_dir = make_thr_directory(thread_data_array[t].tid);
        thread_data_array[t].port = port;
        thread_data_array[t].hosten = NULL;
        thread_data_array[t].sockfd = 0;
        thread_data_array[t].argc = argc;
        //place copy of argument list into struct
        int i = 0;
        for(i = 0; i < argc ; i++){
            thread_data_array[t].args[i] = argv[i];
        }
        //set last argument to null
        thread_data_array[t].args[i] = NULL;

        pthread_mutex_init(&copy_mutex, NULL);
        pthread_mutex_lock(&copy_mutex);
        cout << "Creating thread: " << t << endl;         
       
        if( int rc = pthread_create(&threads[t], NULL , copy_files ,(void*)  &thread_data_array[t])){
          cout << "ERROR: return code from pthread_create() " << rc << endl;
          exit(-1);
          }
     //end for loop
    }
    

    pthread_mutex_unlock(&copy_mutex);
   
   //wait for each thread to complete
    for(int z = 0; z < NUM_THREADS; z++)
        pthread_join(threads[z],NULL);

    sleep(1);
    close(sockfd);
    cout << "Client exiting" << endl;
    
    //clean up
    pthread_mutex_destroy(&copy_mutex);
    pthread_exit(NULL);
    return 0;
}

