#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ECHOMAX 255             /* Longest string to echo */

// define the normal packet send to the client
typedef struct {
    int size;
    int type;
    int sequence_number;
    char data[ECHOMAX + 1];
} NormalPacket;

typedef struct {
    int type;
    int ack_number;
} ACKPacket;

/* Error handling function */
void DieWithError(char *errorMessage) {
    printf("%s", errorMessage);
    exit(1);
}

int simulateBitError(float bit_error_pro);

// create the terminate packet when the file content is
// all sent to the receiver
NormalPacket createTerminatePacket(int sequence_number, int size) {
    NormalPacket packet;
    packet.type = 4;
    packet.sequence_number = sequence_number;
    packet.size = size;
    memset(packet.data, 0, sizeof(packet.data));
    
    return packet;
}

NormalPacket createNormalPacket(int sequence_number, int size, char *data) {
    NormalPacket packet;
    packet.type = 1;
    packet.size = size;
    packet.sequence_number = sequence_number;
    memset(packet.data, 0, sizeof(packet.data));
    strcpy(packet.data, data);
    
    return packet;
}

int main(int argc, char *argv[]) {
    int sock; /* Socket */
    int windowSize;
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[ECHOMAX]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int recvMsgSize; /* Size of received message */
    float error_rate = 0;
    
    if (argc < 1 || argc > 4) /* Test for correct number of arguments */ {
        fprintf(stderr, "Usage: %s <Server Port> [<Window Size>] [<Error Rate>]\n", argv[0]) ;
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);   /* First arg: local port */
    windowSize = atoi(argv[2]);     /* Window size */
    if (argc == 4) {
        error_rate = atof(argv[3]);     /* Error Rate */
    }
    
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError( "socket () failed") ;
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort); /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind () failed");
    
    char *dataBuffer = NULL;
    // Get file content first
    while (1) {
        // clear buffer
        for (int i = 0; i < ECHOMAX; i++) {
            echoBuffer[i] = '\0';
        }
        
        printf("Waiting for request for file...\n");
        /* Block until receive message from a client */
        if ((recvMsgSize = (int)recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed") ;
        
        printf("File name = %s\n", echoBuffer);
        FILE *fp = fopen(echoBuffer, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            dataBuffer = (char *)malloc(sizeof(*dataBuffer) * (size + 1));
            fseek(fp, 0L, SEEK_SET);
            fread(dataBuffer, sizeof(char), size, fp);
            dataBuffer[size] = '\0';
        }
        
        if (dataBuffer != NULL) {
            // printf("Content: %s\n", dataBuffer);
            break;
        }
        else {
            strcpy(echoBuffer, "Could not find file.");
            /* Send received datagram back to the client */
            if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
                DieWithError("sendto() sent a different number of bytes than expected");
        }
    }
    
    // when break, it means the file exist
    // send file data as packet to recviver
    int nonAck = 1;
    int base = -1;
    int sequence_number = 0;
    
    // determine how many bytes are required for the file content
    int number_of_packets = (int)strlen(dataBuffer) / ECHOMAX;
    if (strlen(dataBuffer) % ECHOMAX > 0) {
        number_of_packets++;
    }
    
    while (nonAck) {
        // send data to the receiver
        while (sequence_number <= number_of_packets && (sequence_number - base) <= windowSize) {
            NormalPacket packet;
            if (sequence_number == number_of_packets) {
                // should end the connection
                packet = createTerminatePacket(sequence_number, 0);
                packet = createTerminatePacket(-1, 0);
                printf("Sending terminate packet to the receiver...\n");
            }
            else {
                // read data from the data buffer
                char data[ECHOMAX];
                strncpy(data, dataBuffer + sequence_number * ECHOMAX, ECHOMAX);
                if (simulateBitError(error_rate)) {
                    packet = createNormalPacket(sequence_number, 0, data);
                }
                else {
                    packet = createNormalPacket(-1, 0, data);
                }
            }
            
            /* Send received datagram back to the client */
            if (sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(packet))
                DieWithError("sendto() sent a different number of bytes than expected");
            
            sequence_number++;
        }
        
        // if the window size is full
        // check the ack packet
        printf("Now the window buffer is full...(size = %d)\n", windowSize);
        ACKPacket aPacket;
        if ((recvMsgSize = (int)recvfrom(sock, &aPacket, sizeof(aPacket), 0, (struct sockaddr *)&echoClntAddr, &cliAddrLen)) > 0) {
            if (aPacket.type == 4) {
                printf("Retransmit the data...\n");
                while (sequence_number <= number_of_packets && (sequence_number - base) <= windowSize) {
                    NormalPacket nPacket;
                    if (sequence_number == number_of_packets) {
                        printf("Sending Terminal Packet\n");
                    }
                    else {
                        char data[ECHOMAX];
                        strncpy(data, dataBuffer + sequence_number * ECHOMAX, ECHOMAX);
                        printf("Sending Packet: %d\n", sequence_number);
                    }
                    
                    if (sendto(sock, &nPacket, sizeof(nPacket), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(nPacket))
                        DieWithError("sendto() sent a different number of bytes than expected");
                    
                    sequence_number++;
                }
            }
        }
        
        if (aPacket.type != 8) {
            printf("----------------------- Recieved ACK: %d\n", aPacket.ack_number);
            if(aPacket.ack_number > base){
                base = aPacket.ack_number;
            }
        }
        else {
            printf("Recieved Terminal ACK\n");
            nonAck = 0;
        }
    }
    
    close(sock);
    
    return 0;
}

int simulateBitError(float bit_error_pro) {
    double rand = drand48();
    if (rand < bit_error_pro) {
        return 1;
    }
    else {
        return 0;
    }
}


