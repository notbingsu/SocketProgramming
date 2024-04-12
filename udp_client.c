/***

udp_client.c: source file of client in udp

***/

#include "headsock.h"



// void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int len);

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len); //transmission function

void tv_sub(struct timeval *out, struct timeval *in); //time interval function



int main(int argc, char *argv[]){

    //init

    int sockfd, ret;

    long len;

    float ti, rt;

    struct sockaddr_in ser_addr;

    char **pptr;

    struct hostent *sh; // has information like name, address type, aliases etc

    struct in_addr **addrs;

    FILE *fp;



    if (argc != 2){

        printf("parameters not match");

        exit(0);

    }

    if ((sh=gethostbyname(argv[1])) == NULL){

        printf("error when gethostby name");

        exit(0);

    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create a socket

    if (sockfd < 0){

        printf("error in socket");

        exit(0);

    }



    addrs = (struct in_addr **)sh->h_addr_list;

    printf("canonical name: %s\n", sh->h_name);

    for (pptr = sh->h_aliases; *pptr != NULL; pptr++)

        printf("aliases: %s\n", *pptr);

    switch(sh->h_addrtype){

        case AF_INET:

            printf("address type: AF_INET\n");

            break;

        default:

            printf("unknown address type\n");

            break;

    }

    ser_addr.sin_family = AF_INET;

    ser_addr.sin_port = htons(MYUDP_PORT);

    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));

    bzero(&(ser_addr.sin_zero), 8);

    //connect socket *not sure if this is necessary for udp

    // ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));

    // if (ret != 0){

    //     printf("connection failed\n");

    //     close(sockfd);

    //     exit(0);

    // }

    //init end

    //open local file

    if ((fp = fopen("myfile.txt", "r")) == NULL){

        printf("file not found");

        exit(0);

    }

    //insert receive and send function

    ti = str_cli(fp, sockfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in), &len);

    rt = (len/(float)ti);

    printf("Time(ms) : %.3f\nData sent: %d bytes\nData rate: %f bps\n", ti, (int)len, rt);

    close(sockfd);

    fclose(fp);

    exit(0);

}



float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len)

{

    char *buf;

    long lsize, ci;

    char sends[DATALEN];

    struct ack_so ack;

    int n, slen;

    float time_inv = 0.0;

    struct timeval sendt, recvt;

    ci = 0;

    fseek(fp, 0, SEEK_END);

    lsize = ftell(fp);

    rewind(fp);

    printf("file length: %ld\n", lsize);

    printf("packet length: %d\n", DATALEN);

    printf("batch size: %d\n", PACKLEN);

    //allocate memory for whole file

    buf = (char *)malloc(lsize);

    if (!buf){

        printf("no memory\n");

        exit(2);

    }



    //copy file to buffer

    fread(buf, 1, lsize, fp);

    //whole file loaded in buffer



    buf[lsize] = '\0'; //append end byte

    gettimeofday(&sendt, NULL); //get current time

    int du = 0;
    long prevs_ci = ci;

    while (ci <= lsize){

    	du = (du + 1)%PACKLEN;

        if ((lsize+1-ci) <= DATALEN){ //yet to handle less than PACKLEN 

			slen = lsize+1-ci;

        }else{

            slen = DATALEN;

        }

		memcpy(sends, (buf+ci), slen);

		// n = send(sockfd, &sends, slen, 0); //send the data

        n = sendto(sockfd, &sends, slen, 0, addr, addrlen);

		if(n == -1) {

			printf("send error!");

			continue;

		}

	if (du == 0){

		n = recv(sockfd, &ack, 2, 0); //receive the ack

    		if (n == -1){ //no ack received

        	printf("error when receiving\n");

        	ci = prevs_ci; //resend same packet
            continue;

        	}
            if (ack.num != 1 || ack.len != 0){ //erroneous ack received

            printf("error in transmission\n");
            ci = prevs_ci; //resend same packet
            continue;

    	    }
        prevs_ci = ci; //set new checkpoint
    	
    	}


        ci += slen; //go to next packet

    }

    

    gettimeofday(&recvt, NULL); //get current time

    *len = ci;

    tv_sub(&recvt, &sendt); //get transmisson time

    time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;

    return time_inv;

}



void tv_sub(struct timeval *out, struct timeval *in)

{

    if ((out->tv_usec -= in->tv_usec) < 0){

        --out->tv_sec;

        out->tv_usec += 1000000;

    }

    out->tv_sec -= in->tv_sec;

}