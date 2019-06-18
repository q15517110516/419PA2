For the server, Usage: %s <Server Port> [<Window Size>] [<Error Rate>]
1. Window size and error rate are optional arguments
2. The error rate must be between 0 and 1. This value will be compared to
a random value to simulate the bit error case. Here, when bit error occurs,
we change the sequence number in the packet. At the client side, when it detects
wrong sequence number, it will return a non-ack packet. Then server employs go-back n
technique to retransmit the packet again.

For the client, Usage: %s <Server IP> <Echo Word> [<Echo Port>] [<Loss Rate>]
1. Here, Echo word should be the file name, when the server receives the request of 
the file, it will check the existence of the file. 

Running instructions:
First, using command 'gcc -o UDPEchoClient UDPEchoClient.c' and 'gcc -o UDPEchoServer UDPEchoServer.c' to generate two files named UDPEchoClient and UDPEchoServer. Then using './UDPEchoClient 127.0.0.1 filename 50000' and './UDPEchoServer 50000 10 0.8' to generate the results.

Files: UDPEchoServer.c UDPEchoClient.c README.txt DieWithError.c UDPEchoServer.o UDPEchoClient.c File.txt