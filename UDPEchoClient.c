#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ECHOMAX 255 /*Longest string to echo*/

void DieWithError(char *errorMessage) {
    printf("%s", errorMessage);
    exit(1);
}

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

int simulatePacketLoss(float loss_rate);
ACKPacket createACKPacket(int ack_type, int ack_number);

int main(int argc, char *argv[]) {
    int sock; /*Socket descriptor*/
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr; /* Source address of echo */
    unsigned short echoServPort;
    unsigned int fromSize;
    char *servIP;
    char *echoString;
    unsigned int echoStringLen;
    int recvPacketSize;
    float loss_rate = 0;
    
    if ( (argc < 3) || (argc > 5) ) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>] [<Loss Rate>]\n"), argv[0];
        exit(1);
    }
    
    servIP = argv[1];
    echoString = argv[2];
    
    if ((echoStringLen = strlen(echoString)) > ECHOMAX) /* Check input length */
        DieWithError("Echo word too long");
    
    if(argc == 4)
        echoServPort = atoi(argv[3]);
    else
        echoServPort = 7;
    
    if (argc == 5) {
        loss_rate = atof(argv[4]);
    }
    
    /* Create a datagram/UDP socket */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
        DieWithError("socket() failed");
    
    /*Construct the server address structure*/
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
    echoServAddr.sin_family = AF_INET;   /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);  /* Server port */
    
    // set the filename to the server
    /*send the string to the server*/
    if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");
    
    int base = -2;
    int sequence_number = 0;
    char dataBuffer[8192];
    for (;;) {
        ACKPacket aPacket;
        NormalPacket nPacket;
        /*Receive a response*/
        fromSize = sizeof(fromAddr);
        if ((recvPacketSize = recvfrom(sock, &nPacket, sizeof(nPacket), 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
            DieWithError("recvfrom() failed");
        }
        // Get the sequence number
        sequence_number = nPacket.sequence_number;
        
        if (!simulatePacketLoss(loss_rate)) {
            if (nPacket.sequence_number == 0 && nPacket.type == 1) {
                printf("Received packet from %s\n", inet_ntoa(echoServAddr.sin_addr));
                memset(dataBuffer, 0, sizeof(dataBuffer));
                strcpy(dataBuffer, nPacket.data);                
                aPacket = createACKPacket(2, base);
                // printf("%s\n", dataBuffer);
            }
            else if (nPacket.sequence_number == (base + 1)) {
                strcat(dataBuffer, nPacket.data);
                base = nPacket.sequence_number;
                aPacket = createACKPacket(2, base);
            }
            else if (nPacket.sequence_number == -1) {
                // when bit error is detected
                // type 4 means tell sender to retransmit
                printf("Detect bit error from the packet...\n");
                aPacket = createACKPacket(4, base);
                printf("Hey retansmit please #%d\n", base);
                if (sendto(sock, &aPacket, sizeof(aPacket), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=sizeof(aPacket))
                    DieWithError("send() sent a different number of bytes than expected");
            }
            
            // if it's a terminal packet
            if (nPacket.type == 4 && sequence_number == base) {
                base = -1;                
            }
            
            // send ack to the sender
            if (base >= 0) {
                printf("------------------------------------  Sending ACK #%d\n", base);
                if (sendto(sock, &aPacket, sizeof(aPacket), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=sizeof(aPacket))
                    DieWithError("send() sent a different number of bytes than expected");
            }
            else if (base == -1) {
                printf("Recieved Teardown Packet\n");
                printf("Sending Terminal ACK %d\n", base);
                if (sendto(sock, &aPacket, sizeof(aPacket), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=sizeof(aPacket))
                    DieWithError("send() sent a different number of bytes than expected");
            }
            
            if (nPacket.type == 4 && base == -1) {
                printf("Received: %s\n", dataBuffer); // show the final content         
                break;
            }
        }
        else {
            printf("Simulated packet lose\n");
        }
    }
    

    close(sock);
    exit(0);
}

int simulatePacketLoss(float loss_rate) {
    double rand = drand48();
    if (rand < loss_rate) {
        return 1;
    }
    else {
        return 0;
    }
}

ACKPacket createACKPacket(int ack_type, int ack_number) {
    ACKPacket packet;
    packet.type = ack_type;
    packet.ack_number = ack_number;
    
    return packet;
}

