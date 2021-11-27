# Online-Code-Judge
Online Judge - It will take codes , compile it, check for compilation errors, execute your code by providing some secret inputs and collect your code's output, check for runtime errors, and finally match your output with its hidden test outputs.


Name    - Sahil Jain
Roll No - 20CS60R64
Email   - sahil1206@kgpian.iitkgp.ac.in


Programming Lang. - C++
     System       - Linux

Status - Running Fine in my PC.

To Compile & Execute Code-

->  server.cpp
    Compile -   g++ server.cpp -o server
    Execute - 	./server <port_number>
    Eg      - 	./server 9000

-> client.cpp
    Compile -    g++ client.cpp -o client
    Execute - 	./client <local address> <port_number>
    Eg      - 	./client 127.0.0.1 9000


How to run CODJUD querry- 
Eg - CODJUD add.cpp cpp or CODJUD add.cpp c++


ERROR CASES HANDLED - 

1) CODJUD <program full name> <extention> 

- User not allowed to retrieve/delete testcase and input files drom the server.
   Eg- RETR testcase_add.txt 
	    -gives error as it is not allowed by online coding judge.

	    Similarly, RETR/DELE input_add.txt is not allowed.

- Displaying which testcase failed.
   Eg- Testcase 5 failed! 

- Check the extension given with the actual extension
   Eg-  raising error in this case,
        CODJUD add.cpp c
        CODJUD div.c cpp

- Deleting garbage files like output_add.txt , add.cpp and add.out after execution of each querry.

- Handling both extension name as c++ and cpp.
  For eg - CODJUD add.cpp c++
  		   CODJUD add.cpp cpp
  		   Both will work.

- Handling TLE (time limit Exceed) runtime error. TIME LIMIT 1 second as given in ppt.

- Checking INPUT FILE exists for the program or not.There might be some program which doesnot 
  take any input from user.

- Checking test case file exists or not, for the given program. If not exists displays appropriate message.
