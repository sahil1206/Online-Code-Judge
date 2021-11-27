/*
	Name - Sahil Jain		
	Roll No. - 20CS60R64

	To compile this code -
	g++ server.cpp -o -server

	To run this code provide two parameters-
	argv[1]- Port Number in the range in 1024 to 65535
	
	./server <port no.>
	
	To quit server press "CTRL + C".

*/


//FTP server code

//HEADER FILES
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstring>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

//MACROS
#define MAX_CLIENTS 20
#define MAXLINE 4096

//function to print error
void error(const char *err)
{
	perror(err);
	exit(1);
}
	
int generate_port()
{
	int port;
	srand(time(0));

	port  = rand();
	while(port<1024 || port>= 65535)
	{
		port = rand();
	}
	//cout<<port;
	return port;
}

int get_ext(char *input)
{
	char * ext;

	ext = strtok(input," ");
	ext = strtok(NULL," ");
	ext = strtok(NULL," ");
	
	int value = -1;
	if(strcmp(ext, "c") == 0){value = 1;}
    else if(strcmp(ext, "c++") == 0){value = 2;}
    else if(strcmp(ext, "cpp") == 0){value = 2;}
	
	return value;	

}

int get_filename(char *input,char *ptr)
{
	char * filename = NULL;
	filename = strtok(input," ");
	filename = strtok(NULL," ");

	strncpy(ptr,filename,strlen(filename));
	//cout<<"filename is:"<<filename<<endl;
	return 1;
}

int get_prog_name(char *input,char *ptr)
{
	char * prog_name;
	prog_name = strtok(input,".");

	strncpy(ptr,prog_name,strlen(prog_name));
	//cout<<"filename is:"<<filename<<endl;
	return 1;
}

int check_ext(char *input)
{	
	char * filename = NULL;
	char temp_ext[1024];
	strcpy(temp_ext,input);

	filename = strtok(input," ");
	filename = strtok(NULL," ");
	filename = strtok(NULL,".");
	//cout<<filename<<endl;


	char * ext;

	ext = strtok(temp_ext," ");
	ext = strtok(NULL," ");
	ext = strtok(NULL," ");
	//cout<<ext<<endl;
	
	if(strcmp(ext, "c") == 0){
		if(strcmp(filename,ext))
			return 1;
	}
    else if(strcmp(ext, "c++") == 0 ||strcmp(ext, "cpp")==0)
    {
     	if(strcmp(filename,"cpp"))
			return 1;	
    }
    
    return 1;


}
//to display the list of all files and sub directories present in the current directory
void list(int client_sock_fd)
{
    struct dirent *dir_ptr;
    DIR *dr = opendir(".");
    char reply[1024];
    string t = "LIST OF FILES:\n";

    if(dr == NULL)
        {
            return;
        }
    //cout<<"List of Files:"<<endl;
    while((dir_ptr = readdir(dr))!=NULL)
    {
        //write
        t = t + " " + dir_ptr->d_name + "\n";
    }
    strcpy(reply, t.c_str());        
    write(client_sock_fd, reply, strlen(reply)); 
    closedir(dr);
}

int input_required(char * prog_name)
{
	int req = -1;
	struct dirent *dir_ptr;
    DIR *dr = opendir(".");

    char c_temp[1024];
    memset(c_temp,'\0',1024);

    string t = "input_"+ string(prog_name) +".txt";
    strcpy(c_temp, t.c_str());

    if(dr == NULL)
        {
            return -2;
        }
    //cout<<"List of Files:"<<endl;
    while((dir_ptr = readdir(dr))!=NULL)
    {
        //t = t + dir_ptr->d_name + "\n";
        if(strcmp(dir_ptr->d_name,c_temp)==0)
        	req = 1;
    } 
    closedir(dr);
    return req;
}

void delete_file(int control_fd, char * input)
{
	char * wbuffer;
	char file_name[1024];
	string temp = "";

	wbuffer=(char *)malloc((MAXLINE)*sizeof(char));

	memset(wbuffer,'\0',MAXLINE);
	memset(file_name,'\0',1024);


	if(get_filename(input,file_name)<0)
	{	
		temp = "No such file exists!\n";
	 	memset(wbuffer,'\0',MAXLINE);
		strcpy(wbuffer, temp.c_str());
		write(control_fd,wbuffer,strlen(wbuffer));
	}
		
	if (remove(file_name) == 0)
	{
		temp = string(file_name) + " Deleted successfully from the server!\n";
	 	memset(wbuffer,'\0',MAXLINE);
		strcpy(wbuffer, temp.c_str());
		write(control_fd,wbuffer,strlen(wbuffer)); 
	}
   else
   {
        temp = "ERROR in deleting this file!\n";
	 	memset(wbuffer,'\0',MAXLINE);
		strcpy(wbuffer, temp.c_str());
		write(control_fd,wbuffer,strlen(wbuffer));
   }
}

int get_command(char* command)
{
	char cpy[1024];
	strcpy(cpy, command);

	//to get the command 
	char *str = strtok(cpy, " ");
	int value = -1;
	//populated value valriable to indicate back to main which input was entered
    if(strcmp(str, "LIST") == 0){value = 1;}
    else if(strcmp(str, "DELE") == 0){value = 2;}
    else if(strcmp(str, "RETR") == 0){value = 3;}
    else if(strcmp(str, "STOR") == 0){value = 4;}
    else if(strcmp(str, "QUIT") == 0){value = 5;}
    else if(strcmp(str, "CODJUD") == 0){value = 6;}

    //printf("Code is %d\n",value);
    return value;
}


//get data connection
int get_data_connection(int portno,int *data_socket_fd)
{
	int socket_fd,n;
	int data_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;

	//socket creation
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd<0) //to check if socket creation succeed
		{error("Socket opening failed");return -1;}

	//Setting options for the socket
    int option=1;
	socklen_t option_len=sizeof(option);
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,option_len);

	//to clear server_addr
	bzero((char * )&server_addr,sizeof(server_addr));

    //initialise server_addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(portno);

	//BINDING()
	if(bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))<0)
		{error("Binding failed");return -1;}


	//LISTEN()
	listen(socket_fd,1);
	client_len = sizeof(client_addr);

	//ACCEPT()
	data_fd = accept(socket_fd,(struct sockaddr *)&client_addr,&client_len);
	if(data_fd<0){
		error("ACCEPT failed");
		return -1;
	}

	*data_socket_fd = socket_fd;

	return data_fd;
}

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

int retrieve_file(int control_fd,int data_fd, char * input)
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
		perror("File_name does not exist!\n");
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

int store_file(int control_fd,int data_fd, char * input)
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

	ofstream file;
	file.open(file_name);

	//to read file from server and write to its own directory
	while(1)
	{
		//cout<<"read kiya\n";
		if(read(data_fd,rbuffer,MAXLINE)<=0)
			break;
		file << rbuffer;
		memset(rbuffer,'\0',MAXLINE);
	}

	
	//to close file
	file.close();

	//to reset all memory allocation
	memset(file_name,'\0',1024);
	memset(wbuffer,'\0',MAXLINE);
	memset(rbuffer,'\0',MAXLINE);
	memset(check,'\0',MAXLINE);

	return 1;
}


int compile_code(char * input)
{
	//variable initialisations
	int check = -1;
	char file_name[1024];
	char temp_ext[1024];
	char temp2_ext[1024];
	char prog_name[1024];
	string temp = "";
	char * command_op;

	
	//to initialise all char arrrays
	command_op = (char *)malloc(1024*sizeof(char));
	memset(file_name,'\0',1024);
	memset(prog_name,'\0',1024);
	memset(temp_ext,'\0',1024);
	memset(command_op,'\0',1024);

	strcpy(temp_ext,input);
	strcpy(temp2_ext,input);

	//to get filename
	int fn = get_filename(input,file_name);

	//to get extension
	int ext = get_ext(temp_ext);
	strcpy(temp_ext,file_name);

	//to get program name
	int pn = get_prog_name(temp_ext,prog_name);


	//check any failure
	if(fn<0 || ext<0)
		cout<<"error in reading file name!";

	//to check if user provided correct extention
	if(check_ext(temp2_ext)==-1)
	{
		return -2;
	}

	//to compile c file
	if(ext==1)
	{
		temp = "gcc " + string(file_name) + " -o " + string(prog_name);
		strcpy(command_op, temp.c_str());

		cout<<"Command given:"<<command_op<<endl;
		check = system(command_op);
		remove(file_name);
		if(check == 0)
			return 1;
		else
			return -1;
	}

	//to compile c++/cpp file
	if(ext==2)
	{
		temp = "g++ " + string(file_name) + " -o " + string(prog_name);
		strcpy(command_op, temp.c_str());
		//cout<<"Command given:"<<command_op<<endl;

		check = system(command_op);
		remove(file_name);
		if(check == 0)
			return 1;
		else
			return -1;
	}

	//if compiled successfully
	return -1;

}

int execute_code(char * input)
{
	int check = -1;
	char file_name[1024];
	char temp_ext[1024];
	char prog_name[1024];
	string temp = "";
	char * command_op;
	char c_temp[1024];
	string line = "";		

	command_op = (char *)malloc(1024*sizeof(char));
	memset(file_name,'\0',1024);
	memset(prog_name,'\0',1024);
	memset(temp_ext,'\0',1024);
	memset(command_op,'\0',1024);
	memset(c_temp,'\0',1024);

	strcpy(temp_ext,input);

	int fn = get_filename(input,file_name);
	int ext = get_ext(temp_ext);
	strcpy(temp_ext,file_name);
	int pn = get_prog_name(temp_ext,prog_name);

	int i_req = input_required(prog_name);

	if(fn<0 || ext<0 || pn<0)
		cout<<"error in reading file name!";

	//make temp_input file to provide input
	ofstream out;            //to write in output file
	ifstream in;			//to get input from temp_input file
	fstream d;				//to get line from temp_input file

	temp = "output_" + string(prog_name) +".txt";
	strcpy(c_temp, temp.c_str());
	//cout<<"Outfile name:"<<c_temp<<endl;
	out.open(c_temp);


	temp = "input_" + string(prog_name) +".txt";
	memset(c_temp,'\0',1024);
	strcpy(c_temp, temp.c_str());
	//cout<<"Input file name:"<<c_temp<<endl;
	in.open(c_temp);

	
	if(i_req==1)
	{
	  while(getline(in,line))
	  {
		d.open("temp_input.txt",ios::out);
		//cout<<line<<endl;
		d<<line<<endl;

		temp = "timeout 1 ./" +string(prog_name) + " 0< temp_input.txt"  +" 1>> output_"+string(prog_name)+".txt";
		strcpy(command_op, temp.c_str());
		//cout<<"Command given:"<<command_op<<endl;
	    check = system(command_op);
	    if(check != 0)
			return -1;
		d.close();	    
		remove("temp_input.txt");
	  }
	}


	else if(i_req == -1)
	{
		temp = "./" +string(prog_name) +" 1>> output_"+string(prog_name)+".txt";
		strcpy(command_op, temp.c_str());
	    check = system(command_op);
	    if(check != 0)
			return -1;
	}


	remove(prog_name);
	return 1;
	
}

int evaluate(char * input)
{	
	char file_name[1024];
	char temp_ext[1024];
	char prog_name[1024];
	string temp = "";
	char * command_op;
	char opfile[1024];
	char testcase[1024];
	//string t_line = "";
	//string o_line = "";	
	char * t_line;
	char * o_line;	

	command_op = (char *)malloc(1024*sizeof(char));
	o_line = (char *)malloc(MAXLINE*sizeof(char));
	t_line = (char *)malloc(MAXLINE*sizeof(char));
	memset(file_name,'\0',1024);
	memset(prog_name,'\0',1024);
	memset(temp_ext,'\0',1024);
	memset(command_op,'\0',1024);
	memset(opfile,'\0',1024);
	memset(testcase,'\0',1024);

	strcpy(temp_ext,input);
	int fn = get_filename(input,file_name);
	int ext = get_ext(temp_ext);
	strcpy(temp_ext,file_name);
	int pn = get_prog_name(temp_ext,prog_name);

	//make temp_input file to provide input
	//ifstream out;            //to write in output file
	//fstream tc;			//to get input from temp_input file

	temp = "output_" + string(prog_name) +".txt";
	strcpy(opfile, temp.c_str());

	temp = "testcase_" + string(prog_name) +".txt";
	strcpy(testcase, temp.c_str());
	
	//to open file and send its data to client
	FILE * op;
	FILE * tc;

	op = fopen(opfile,"r");
	tc  = fopen(testcase,"r");
	if(op==NULL||tc == NULL)
	{
		perror("No testcase file exists for this program!\n");
		return -1;
	}

	int i=1;
	while(fgets(o_line,MAXLINE,op) != NULL)
	{

		fgets(t_line,MAXLINE,tc);


		if(strncmp(t_line,o_line,strlen(t_line)-2)!=0)
		{
			fclose(op);
	        fclose(tc);
	        remove(opfile);
			return i;
		}
		
		memset(t_line,'\0',MAXLINE);
		memset(o_line,'\0',MAXLINE);

		//cout<<endl;
		i++;

	}

	fclose(op);
	fclose(tc);
	remove(opfile);

	return 0;

}

//Main function
int main(int argc, char **argv)
{
	//VARIABLE INITIALISATION
	char * rbuffer;                 //to read from client buffer
	char * wbuffer;                 //to write in client buffer
	int check;                      //to check error
	int master_socket_fd;           //to store master socket file descriptors
	socklen_t option_len;           //to store optionns length	
	fd_set readfds;	                //set of file descriptors to read				
	int portno = atoi(argv[1]);     //assign port no. passed in CMD line arguments
	int client_count;               //to count number of clients connected
	int max_fd;                     //to store maximum fd for select()
	int client_fd;                  //to store new socket fd for client
	int client_sock_fd[MAX_CLIENTS];//to store multiple client fds 
	int client_number[MAX_CLIENTS];	//each client number/id;
	int data_fd[MAX_CLIENTS];       //to store fd of data connection
	int data_sock_fd[MAX_CLIENTS];		//to store data socket_fd
	char reply[1024];               //to reply to client
	int flag = 0;                       // to check status



	//to handle insuffecient arguments
	if(argc != 2)
	{
		fprintf(stderr,"Insuffecient Arguments provided,please refer readme.txt");
		exit(1);
	}


	//SOCKET ESTABLISHMENT
	master_socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(master_socket_fd<0)
		error("Connection Not Established");

	printf("SOCKET Established!\n");

	//set options for socket to reuse port no and server address
	int option=1;
	option_len=sizeof(option);
	setsockopt(master_socket_fd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&option,option_len);

	//Initialise Server address
	struct sockaddr_in server_addr;

	//to clear server_addr
	bzero((char * )&server_addr,sizeof(server_addr));

	//Initialise Server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //to B
	server_addr.sin_port = htons(portno);

	//BINDING()
	if(bind(master_socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))<0)
		error("Binding failed");
	printf("Server Bound to port : %d \n",portno);

	//LISTEN()
	check = listen(master_socket_fd,10);   //listen to max 10 clients
	if(check<0)
		error("Listening failed\n");
	printf("Listening..... \n");	
	
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	//Initialise client_sock_fd to 0
	for(int i=0;i<20;i++)
	{
		client_sock_fd[i]=0;
		client_number[i]=0;
		data_fd[i] = 0;
		data_sock_fd[i]=0;
	}

	//Initialise client count to 0
	client_count=0;

	//Initialising read and write buffer
	rbuffer=(char *)malloc(1024*sizeof(char));
	wbuffer=(char *)malloc(1024*sizeof(char));


	//To accept new clients and process clients querry
	while(1)
	{
		FD_ZERO(&readfds);				    //setting fd set to zero
		FD_SET(master_socket_fd,&readfds);	//setting master fd set to zero
		max_fd = master_socket_fd;

		for(int i=0;i<MAX_CLIENTS;i++)
		{
			FD_SET(client_sock_fd[i],&readfds);
			if(client_sock_fd[i]>0)
			{
				max_fd = client_sock_fd[i];
			}
		}

		//Using select() to handle multiole client request
		check = select(max_fd+1,&readfds,NULL,NULL,NULL);
		if(check<0)
			error("Error in select");


	//to accept new clients	
	if(FD_ISSET(master_socket_fd,&readfds))
	{
			//ACCEPT new incoming connection from clients
			client_fd=accept(master_socket_fd,(struct sockaddr *) &client_addr,&client_len);
			client_count++;
			
			//printf("Connected with client port number: %d \n",p_no);
			//fflush(NULL);
			for(int i=0;i<MAX_CLIENTS;i++)
			{
				if(client_sock_fd[i]==0)
				{
					client_sock_fd[i]=client_fd;
					client_number[i]=client_count;
					break;
				}							
			}
	}
	//Processing clients request
	else
	{
		//iterating over all clients to process thier request 
		for(int i=0;i<20;i++)
	    {
		if(FD_ISSET(client_sock_fd[i],&readfds))
		{
			
		//reset buffer using bzero
		memset(rbuffer,'\0',1024);

		//READING from client
		check = read(client_sock_fd[i],rbuffer,1024);
		if(check<=0)
		{
			continue;
		}
		int rsize = strlen(rbuffer);

		//to remove extra new line character from buffer
		rbuffer[rsize-1] = '\0';

		//error checking for reading from clients side
		if(check<=0)
			error("Read failed");

		printf("Client Id:%d sent message :%s\n",i+1,rbuffer);
		
		//storing user command in 3 input variabe
		char  input[2048];
		strcpy(input,rbuffer);
		char  input2[2048];
		strcpy(input2,rbuffer);
		char  input3[2048];
		strcpy(input3,rbuffer);

		//here get the command 
		int code = get_command(rbuffer);

		//setting up data connection to transfer data
		if(code==3||code == 4||code == 6)
		{
			int data_port= generate_port();
			string s_port = to_string(data_port);
			memset(wbuffer,'\0',1024);
	 		strcpy(wbuffer, s_port.c_str());
			//sprintf(wbuffer,"5454");
		    write(client_sock_fd[i],wbuffer,strlen(wbuffer));
		    data_fd[i] = get_data_connection(data_port,&data_sock_fd[i]);
		    if(data_fd[i]<0)
		    {
		    	printf("Error in opening data Connection!\n");
		    	continue;
		    }
		}


		//to process FTP command given by client

		switch(code)
		{
			//to list file in the current directory
		 	case 1: 
		 		list(client_sock_fd[i]);
		 		break;

		 	//to delete file provided by client 
		 	case 2:
		 		delete_file(client_sock_fd[i],rbuffer);
		 		break;

		 	//to send file requested by client
		 	case 3:

		 		if(check_testcase(input2)==1)
		 		{
		 			sprintf(reply, "Permission Denied By server! You cannot retrieve testcase/input File!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
		 		}
				else{
					retrieve_file(client_sock_fd[i],data_fd[i],rbuffer);
                	close(data_fd[i]);
                	close(data_sock_fd[i]);
					cout<<"RETRIEVE DONE!\n";
				}
		 		
		 		break;

		 	//to store file sent by client
		 	case 4:
		 		store_file(client_sock_fd[i],data_fd[i],rbuffer);
                close(data_fd[i]);
                close(data_sock_fd[i]);

		 		break;

		 	//to terminate client's FTP connection
		 	case 5:
		 		printf("Client Id: %d ending FTP connection!\n",i+1);
		 		FD_CLR(client_sock_fd[i],&readfds);
		 		break;

		 	//to perform CODJUD	
		 	case 6:
		 		//to retrieve source code from client
		 		store_file(client_sock_fd[i],data_fd[i],rbuffer);
		 		close(data_fd[i]);
                close(data_sock_fd[i]);
		 		//cout<<"flag is:"<<endl;
                memset(reply,'\0',1024);
                //to compile code
                flag  = compile_code(input);
                //cout<<"flag is:"<<flag<<endl;
                if(flag==1){
                	cout<<"Code compiled successfully!\n";
                	sprintf(reply, "Compiled successfully!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }
                else{
                	cout<<"ERROR in compilation!\n";
                	sprintf(reply, "ERROR!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }

                //to Execute
                //cout<<"to Execute:"<<input2<<endl;
                flag = execute_code(input2);
                 if(flag==1){
                	cout<<"Code Executed successfully!\n";
                	sprintf(reply, "Code Executed successfully!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }
                else if(flag==-1){
                	cout<<"ERROR!\n";
                	sprintf(reply, "ERROR!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }
                else if(flag==-2){
                	cout<<"Extension Mismatch!\n";
                	sprintf(reply, "Extension Mismatch!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }


                //to evaluate code
                flag = evaluate(input3);
                 if(flag==0){
                	cout<<"ALL test cases passed!\n";
                	sprintf(reply, "ALL test cases passed!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }
                else if(flag==-1){
                	sprintf(reply, "No testcase file exists for this program!\n");
                	write(client_sock_fd[i], reply, strlen(reply));
                }
                else{
                	cout<<"ERROR in testcase:"<<flag<<endl;
                	sprintf(reply, "ERROR in testcase: %d \n",flag);
                	write(client_sock_fd[i], reply, strlen(reply));
                }

		 		break;

		 	//to handle invalid command given by user
		 	default:
                sprintf(reply, "Invalid Command!");
                write(client_sock_fd[i], reply, strlen(reply));
                break;
		 }

		//END OF CLIENT REQUEST
		}
	}
  }
 } 
	//closing the server FTP connection
	close(master_socket_fd);
	return 0;
}
