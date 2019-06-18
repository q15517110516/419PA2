# Programming Assignment 2
## General Instructions
1. This is an individual programming assignment.
2. You are allowed to consult any Internet resource, books, the course TA, or the instructor. You are not allowed
to exchange code snippets or anything across groups. While you may read on general socket programming
on the Internet and look at code examples, this should be only for the purpose of understanding. Do all the
coding yourself, do not copy or cut-paste code from ANY website.
3. You can use only socket programming for the project. Do not use any sophisticated libraries downloaded
from the web.
4. You should use C/C++ for coding.
5. You should start early and allocate sufficient time for testing your code.
6. It is important that your code has adequate comment and proper indentation. Comment during/before
coding, not at the end. For every variable/function you should have a comment. Use appropriate code
comments for explaining the logic where necessary.
7. Provide documentation for your code. This should explain the code structure in terms of directories, files,
how to compile etc. Detailed instructions for submitting the documentation along with code are at the end
of the document.
8. Choose proper and intuitive names for the variable, function, file, directory, etc.
9. All of the above aspects of your code will be evaluated, not just whether or not the code runs.
10. The below projects involve multiple hosts. You may want to write the code such that your code can be run on
a single host where the multiple hosts are mimicked by using the loopback address (127.0.0.1) with different
ports. This may help during the initial phases, where you need to do a lot of debugging. The IP address/port
specification for the different hosts is ideally provided through configuration files or as arguments to your
code.
## Task 1: File Transfer Over UDP
In this task, you will modify the program you developed in Programming Assignment 1 to allow a client to
download files from a server over UDP. The detailed process is the same as Program Assignment 1.
1. The server starts and waits for requests from the client.
2. The client sends a filename to the server.
3. The server receives the filename.
4. If the file is not present, the server informs the client by sending back an error message.
5. If the file is present, the server continuously reads the file into a buffer and sends the buffer content to the
client until file-end is reached. File end is marked by EOF.
6. Once the transmission finishes, the client verifies that the received file is correct. You need decide the
mechanism that enables verification at the client.
The user interface need not be sophisticated, but should be usable. It can be text-based or graphical. Your program
should use UDP sockets.
## Task 2: Simulate and Cope with Bit Errors
Since UDP does not provide reliable data transfer, you will implement a pipeline reliable data transfer mechanism
in your program to deal with bit error.
1. You can choose either Go-Back-N or Selective Repeat.
2. The window size N should be an input argument to your program. You can set the window size to one
when debugging your program, .
3. At the server side, before a packet is sent to the client, you flip a random coin. With probability p, you flip
some bits in the packet, where p 2 (0, 1) is another input argument to your program.
4. Your program should allow the client to check whether the a received packet contains bit errors. If so, the
client should send back ACK (or NACK) to inform the server about the bit errors, and the server will resend
the packet as needed.
## Task 3: Test Your Program
You will run different experiments to test the performance of your reliable data transfer mechanism.
1. You should test different combinations of file size F, window size N, and error probability p and measure
the time for the client to successfully download a file.
2. You will draw figures to demonstrate the relationship between T, time needed for downloading a file, and
file size F, window size N, and error probability p. For each figure, you fix two of the three parameters, and
show the relationship between T and the remaining parameter.
## Bonus Task: Deal with Packet Loss
You will implement a mechanism to deal with packet loss using a countdown timer.
1. At the client side, on receiving a packet, you flip another random coin. With probability q, the client drops
the packet, where q 2 (0, 1) is another input argument.
2. Your program should allow the server to detect packet loss and retransmit the packet as needed.
3. You may refer to Chapter 5 of the reference book ”TCP/IP Sockets in C Practical Guide for Programmers”
for how to implement a countdown timer. You can also find a sample code on Page 56.
4. You should test different combinations of file size F, window size N, timeout interval S, and packet loss
probability q, and measure the time for the client to successfully download a file.
5. You will draw figures to demonstrate the relationship between T, time needed for downloading a file, and
file size F, window size N, timeout interval S, and packet loss probability q. For each figure, you fix three of
the four parameters, and show the relationship between T and the remaining parameter.
