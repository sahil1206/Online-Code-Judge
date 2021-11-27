/*
	Name - Sahil Jain		
	Roll No. - 20CS60R64

	To compile this code -
	g++ client.cpp -o -client

	To run this code provide two parameters-
	argv[1]- LocalAddress (eg 127.0.0.1)
	argv[2]- Port Number in the range in 1024 to 65535
	
	./client <ip address> <port no.>
	To end connection, give command "exit".


	To Use FTP Protocol:
	1) LIST             - to list all files present in directory of the server (same as "ls -al").
	2) DELE <file_name> - to delete file with the given filename
	3) RETR <file_name> - to retrieve file from the server's end
	4) STOR <file_name> - to store file in the server side.
	5) QUIT             - to disconnect form the FTP server.

*/

//HEADER FILES
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <netdb.h>
using namespace std;


//MACROS
#define MAXLINE 4096


//function to print error
void error(const char *err)
{
	perror(err);
	exit(1);
}


//to get the flename from the given input
int get_filename(char *input,char *ptr)
{
	char * filename = NULL;
	filename = strtok(input," ");
	filename = strtok(NULL," ");

	if(filename == NULL)
	{
		return -1;
	}
	else
	{
		strncpy(ptr,filename,strlen(filename));
		//cout<<"filename is:"<<filename<<endl;
		return 1;
	}


}

//to get command code given by the user
int get_command_code(char* command)
{
	char cpy[1024];
	strcpy(cpy, command);

	//to get the command 
	char *str = strtok(cpy, " ");
	int value = -1;
	//to assign integer value to commands
    if(strcmp(str, "LIST") == 0){value = 1;}
    else if(strcmp(str, "DELE") == 0){value = 2;}
    else if(strcmp(str, "RETR") == 0){value = 3;}
    else if(strcmp(str, "STOR") == 0){value = 4;}
    else if(strcmp(str, "QUIT") == 0){value = 5;}
    else if(strcmp(str, "CODJUD") == 0){value = 6;}

    //to return command code
    return value;
}



int connect_to(int port)
{
	//varriable decclaration
	int client_data_fd, check;       
	struct sockaddr_in server_addr;
	struct hostent *server;
	int option=1;
	char *buffer;

	//SOCKET ESTABLISHMENT
	client_data_fd = socket(AF_INET,SOCK_STREAM,0);
	if(client_data_fd<0) //to check if socket creation succeed
		error("Socket opening failed");

	//Setting options for the socket
	socklen_t option_len=sizeof(option);
	setsockopt(client_data_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,option_len);

	server = gethostbyname("127.0.0.1");
	if(server==NULL)
	{
		fprintf(stderr,"Error: Host does not exist");
	}

	//to clear server addr
	bzero((char *) &server_addr, sizeof(server_addr));

    //initialise server_addr
	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
	
	if(connect(client_data_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
		error("Connection NOT established");

	return client_data_fd;
}


// to check client cannot retrieve testcase or input file from the server
int check_testcase(char * input)
{
	int req = -1;
    char c_temp[1024];
    memset(c_temp,'\0',1024);
    char d_temp[1024];
    memset(d_temp,'\0',1024);

    char file_name[1024];
    memset(file_name,'\0',1024);

    get_filename(input,file_name);
    string t = "testcase_";
    string u = "input_";
    strcpy(c_temp, t.c_str());
    strcpy(d_temp, u.c_str());
    //t = t + dir_ptr->d_name + "\n";
        if(strncmp(file_name,c_temp,strlen(c_temp))==0 || strncmp(file_name,d_temp,strlen(d_temp))==0 )
        	req = 1;
    return req;

}

//to receive file while retreiving from server
int get_retrieve(int data_fd,int control_fd,char * input)
{
	// variables declaration
	char file_name[1024]; //to store filename
	char * wbuffer;
	char * rbuffer;
	char check[MAXLINE];
	char * temp;
	char temp_file_name[2048];
	int n;
	int control_finished = 0;
	int data_finished = 0;

	//dynamically allocate memory to read and write buffer
	rbuffer=(char *)malloc((MAXLINE)*sizeof(char));
	wbuffer=(char *)malloc((MAXLINE)*sizeof(char));

	//initialising all cahr arrays
	memset(file_name,'\0',1024);
	memset(wbuffer,'\0',MAXLINE);
	memset(rbuffer,'\0',MAXLINE);
	memset(check,'\0',MAXLINE);

	//cout<<"input is:"<<input<<"***"<<endl;
	int fn = get_filename(input,file_name);

	//cout<<"Filename: ***"<<file_name<<"****"<<endl;	
	memset(rbuffer,'\0',MAXLINE);

	//to receive file name from the server
	FILE *fp;
	fp = fopen(file_name,"w");
	if(fp==NULL)
	{
		perror("Error in creating file");
		exit(1);
	}

	//to read file from server ad write to its own directory
	while(1)
	{
		n = read(data_fd,rbuffer,MAXLINE);
		//cout<<"read kiya\n";
		if(n<=0)
			break;
		fprintf(fp,"%s",rbuffer);
		memset(rbuffer,'\0',MAXLINE);
	}
	
	//to close file
	fclose(fp);


	//to reset all memory allocation
	memset(file_name,'\0',1024);
	memset(wbuffer,'\0',MAXLINE);
	memset(rbuffer,'\0',MAXLINE);
	memset(check,'\0',MAXLINE);

	
	return 1;

}



//to store file in the server's end
int store_file(int data_fd,int control_fd,char * input)
{
	char file_name[1024]; //to store filename
	char * wbuffer;
	char check[MAXLINE];
	string temp = "";

	wbuffer=(char *)malloc((MAXLINE+1)*sizeof(char));

	memset(file_name,'\0',1024);
	memset(wbuffer,'\0',MAXLINE);
	memset(check,'\0',MAXLINE);


	if(get_filename(input,file_name)<0)
	{
		perror("File_name does not exist!");
		temp = "404 File Doesnot exist!\n";
		memset(wbuffer,'\0',MAXLINE);
		strcpy(wbuffer, temp.c_str());
		write(control_fd,wbuffer,strlen(wbuffer));

	}

	memset(wbuffer,'\0',MAXLINE);

	//to open file and send its data to client
	FILE * fp;

	fp = fopen(file_name,"r");
	if(fp==NULL)
	{
		perror("Error in opening file\n");
		exit(1);
	}
	while(fgets(wbuffer,MAXLINE,fp) != NULL)
	{

		if(write(data_fd,wbuffer,strlen(wbuffer)) < 0)
		{
			perror("Error in reading file\n");
			exit(1);
		}
		//cout<<"Send kiya!\n";
		memset(wbuffer,'\0',MAXLINE);
	}

	fclose(fp);
	return 1;
}




//driver function
int main(int argc, char const *argv[])
{
	int socket_fd, portno, n;
	char * rbuffer; //initialise read buffer size of 1000
	char * wbuffer;
	struct sockaddr_in server_addr;
	struct hostent *server;
	int data_fd;

	//to handle insuffecient arguments
	if(argc != 3)
	{
		fprintf(stderr,"Insuffecient Arguments provided,please refer readme.txt");
		exit(1);
	}

	

	//Initialising read and write buffer
	rbuffer=(char *)malloc(MAXLINE*sizeof(char));
	wbuffer=(char *)malloc(MAXLINE*sizeof(char));

	//portno passed in Argument 1
	portno= atoi(argv[2]);

	//SOCKET ESTABLISHMENT
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd<0) //to check if socket creation succeed
		error("Socket opening failed");

	//Setting options for the socket
    int option=1;
	socklen_t option_len=sizeof(option);
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,option_len);


	server = gethostbyname(argv[1]);
	if(server==NULL)
	{
		fprintf(stderr,"Error: Host does not exist");
	}

	//to clear server addr
	bzero((char *) &server_addr, sizeof(server_addr));

    //initialise server_addr
	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portno);
	
	if(connect(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
		error("Connection NOT established");

	printf("Connected to FTP server...\n"); 

	

	//FTP porotcal started from clients side
	while(1)
	{
		//resettinf buffer
		memset(rbuffer,'\0',MAXLINE);
		memset(wbuffer,'\0',MAXLINE);

		//Input Message in client side
        printf("Enter command :\n"); 
		
		//taing user command from terminal
		fgets(wbuffer,MAXLINE,stdin);

		//write querry from client to server
		n=write(socket_fd,wbuffer,strlen(wbuffer));
		if(n<0)
			error("Write failed");


		//storing user command in inout variabe
		char * input = wbuffer;
		int input_size = strlen(input);
		input[input_size-1] = '\0';

		char  input2[2048];
		strcpy(input2,input);

		//get the code of the command given
		int code = get_command_code(wbuffer);

		//to establish data connection
		if(code==3||code == 4||code == 6)
		{
			//sleep(0.5);
			//to get data port number from the server
		    n=read(socket_fd,rbuffer,MAXLINE);
		    //cout<<rbuffer<<endl;
		    int data_port_no = atoi(rbuffer);
		    //cout<<"Port No Received:"<<data_port_no<<endl;
			data_fd = connect_to(data_port_no);
			//reset buffer toread from the server
			memset(rbuffer,'\0',MAXLINE);
		}


		if(code ==3)
		{
			//cout<<"retreiving File...."<<endl;
			if(check_testcase(input2)==1)
		 		{
		 			read(socket_fd,rbuffer,MAXLINE);
		    		cout<<rbuffer;
		 		}
				else{
				get_retrieve(data_fd,socket_fd,input);
				cout<<"RETRIEVE DONE!\n";
				}
			
			continue;
		}

		if(code ==4)
		{
			//cout<<"Sending File...."<<endl;
			store_file(data_fd,socket_fd,input);
			close(data_fd);
			continue;
		}


		//if client wants to quit this connection
		if(code==5)
		{
			break;
		}

		//if client wants to quit this connection
		if(code==6)
		{
			//sending source code to server
			store_file(data_fd,socket_fd,input);
			close(data_fd);
			
			//to read compilation status
			memset(rbuffer,'\0',MAXLINE);
			n=read(socket_fd,rbuffer,MAXLINE);
			if(n<0)
				error("Read failed");
			printf("Compilation status: %s",rbuffer);

			//to read execution status
			memset(rbuffer,'\0',MAXLINE);
			n=read(socket_fd,rbuffer,MAXLINE);
			if(n<0)
				error("Read failed");
			printf("Execution: %s",rbuffer);

			//to read evaluation status
			memset(rbuffer,'\0',MAXLINE);
			n=read(socket_fd,rbuffer,MAXLINE);
			if(n<0)
				error("Read failed");
			printf("Result/Evaluation: %s \n",rbuffer);

			continue;
		}

		//reset buffer toread from the server
		memset(rbuffer,'\0',MAXLINE);
		memset(wbuffer,'\0',MAXLINE);
		
		//read the reply from the server
		n=read(socket_fd,rbuffer,MAXLINE);
		if(n<0)
			error("Read failed");
		printf("Server : %s \n",rbuffer);

	}

	//close the connection
	cout<<"Ending this connection"<<endl;
	close(socket_fd);

	return 0;
}
