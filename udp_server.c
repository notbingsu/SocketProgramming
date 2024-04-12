/**************************************

udp_ser.c: the source file of the server in udp transmission

**************************************/

#include "headsock.h"



#define BACKLOG 10

void str_ser(int sockfd); // transmitting and receiving function

int main(int argc, char *argv[])

{

	int sockfd, con_fd, ret;

	struct sockaddr_in my_addr;

    struct sockaddr_in their_addr;

    int sin_size;



    pid_t pid;



	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {			//create socket

		printf("error in socket");

		exit(1);

	}

	my_addr.sin_family = AF_INET;

	my_addr.sin_port = htons(MYUDP_PORT);

	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bzero(&(my_addr.sin_zero), 8);

	ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));	//bind socket

    if (ret == -1) {

        printf("error in binding");

        exit(1);

    }

	printf("start receiving\n");

	printf("waiting for data\n");

    sin_size = sizeof(struct sockaddr_in);

    str_ser(sockfd); //receive packet and response

	close(sockfd);

	exit(0);

}



void str_ser(int sockfd)

{

    char buf[BUFSIZE];

    FILE *fp;

    char recvs[DATALEN];

    struct ack_so ack;

    int len, end, n = 0;

    long lseek = 0;

    end = 0;

    struct sockaddr_in addr;

    len = sizeof(struct sockaddr);

    printf("receiving data!\n");

    int du = 0;
    char temp[DATALEN * PACKLEN];
    int temp_n = 0;
    while (!end) {
    du = (du + 1) % PACKLEN;

    if ((n = recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&addr, &len)) == -1) {
        printf("error when receiving\n");
        continue;
    }

    if (recvs[n - 1] == '\0') {
        end = 1;
        n--;
    }

    memcpy(temp + temp_n, recvs, n);
    temp_n += n;

    if (du == 0) {
        ack.num = 1;
        ack.len = 0;
        if ((n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&addr, len)) == -1) {
            printf("send error!");//send ack
            exit(1);
        }

        // Copy data from temp to buf
        memcpy(buf + lseek, temp, temp_n);
        lseek += temp_n;
        temp_n = 0; // Reset temp_n for next iteration
    }
    }

    	

	if ((fp = fopen ("myreceive.txt","wt")) == NULL)

	{

		printf("File doesn't exist\n");

		exit(0);

	}

	fwrite (buf , 1 , lseek , fp);					//write data into file

	fclose(fp);

	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);

}



// void str_ser1(int sockfd)

// {

// 	char recvs[MAXSIZE];

// 	int n = 0, len;

// 	struct sockaddr_in addr;

// 	len = sizeof (struct sockaddr_in);

// 	if ((n=recvfrom(sockfd, &recvs, MAXSIZE, 0, (struct sockaddr *)&addr, &len)) == -1) {      //receive the packet

// 		printf("error receiving");

// 		exit(1);

// 	}

// 	recvs[n] = '\0';

// 	printf("the received string is :\n%s", recvs);

// }