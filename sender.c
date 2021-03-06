/************************************************************************
 * Adapted from a course at Boston University for use in CPSC 317 at UBC
 * 
 *
 * The interfaces for the STP sender (you get to implement them), and a 
 * simple application-level routine to drive the sender.  
 *
 * This routine reads the data to be transferred over the connection
 * from a file specified and invokes the STP send functionality to 
 * deliver the packets as an ordered sequence of datagrams.
 *
 * Version 1.0 
 * 
 *
 *************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/file.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "stp.h"

#define STP_SUCCESS 1
#define STP_ERROR -1
#define CLK_TCK 60 // not sure the units??

//Sender states
#define STP_SYN_SENT   0x24
#define STP_CLOSING   0x25
#define FIN_WAIT   0x26	//We do not neet to implement this state.

int SenderMaxWin = 5000;        /* Maximum window size */
typedef struct {
  
//  int DELETE_ME;     /* used only to make this compile */
	int state;	 /* protocol state: normally ESTABLISHED */
	int sock; 	/* UDP socket descriptor */

	unsigned short swnd;       /* latest advertised sender window */
	unsigned short NBE;        /* next byte expected */
	unsigned short LbACK;     /* last byte ACKed */
	unsigned short LBSent; 	/* last byte Sent not ACKed */

	double timer; /* timer for timeouts on SYN and DATA etc */
	unsigned short ISN;        /* initial sequence number */

	pktbuf *sendQueue;         /* Pointer to the first node of the send queue */
  
  
  
} stp_send_ctrl_blk;



/* ADD ANY EXTRA FUNCTIONS HERE */
// the following timer is from http://www.dreamincode.net/code/snippet2169.htm
// There is a function called readWithTimer in stp.c and stp.h that deals with time-out issue. 
clock_t BeginTimer()
{
    //timer declaration
    clock_t Begin; //initialize Begin
 
    Begin = clock() * CLK_TCK; //start the timer
 
    return Begin;
}
clock_t EndTimer(clock_t begin)
{
    clock_t End;
    End = clock() * CLK_TCK;   //stop the timer
    return End;
}

// frees the memory for the stp_send_ctrl_blk
/*
void FreeStp_Send_CB(struct stp_send_ctrl_blk *stp_send_CB) {
    if ( stp_send_CB->state )
	free(stp_send_CB->state);
    if ( stp_send_CB->sock )
	free(stp_send_CB->sock);
    if ( stp_send_CB->swnd )
	free(stp_send_CB->swnd);
    if ( stp_send_CB->NBE )
	free(stp_send_CB->NBE);
    if ( stp_send_CB->LbACK )
	free(stp_send_CB->LBSent);
    if ( stp_send_CB->timer )
	free(stp_send_CB->timer);
    if ( stp_send_CB->ISN )
	free(stp_send_CB->ISN);

}
*/

/*
 * Send STP. This routine is to send a data packet no greater than
 * MSS bytes. If more than MSS bytes are to be sent, the routine
 * breaks the data into multiple packets. It will keep sending data
 * until the send window is full. At which point it reads data from
 * the network to, hopefully, get the ACKs that open the window. You
 * will need to be careful about timing your packets and dealing with
 * the last piece of data.
 * 
 * The function returns STP_SUCCESS on success, or STP_ERROR on error.
 */
int stp_send (stp_send_ctrl_blk *stp_CB, unsigned char* data, int length) {
  
  /* YOUR CODE HERE */
  return 0;
} 

//Creates UDP sockets
int open_udp(char *destination, int destinationPort,int receivePort)
{
	int      fd;
	uint32_t dst;
	struct   sockaddr_in sin;
  
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) 
	{
      perror("Error creating UDP socket");
      return -1;
    }
  
	/* Bind the local socket to listen at the local_port. */
	printf ("Binding locally to port %d\n", receivePort);
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(receivePort);
  
	if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
    {
      perror("Bind failed");
      return (-2);
    }
  
	dst = hostname_to_ipaddr(destination);
  
	if (!dst) {
		printf("Invalid sending host name: %s\n", destination);
		return -4;
	}
	printf ("Configuring  UDP \"connection\" to <%u.%u.%u.%u, port %d>\n", 
          (ntohl(dst)>>24) & 0xFF, (ntohl(dst)>>16) & 0xFF, 
          (ntohl(dst)>>8) & 0XFF, ntohl(dst) & 0XFF, destinationPort);
  
	memset((char *)&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(destinationPort);
	sin.sin_addr.s_addr = dst;
	if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
	{
      perror("connect");
      return(-1);
	}
	printf ("UDP \"connection\" to <%u.%u.%u.%u port %d> configured\n", 
          (ntohl(dst)>>24) & 0xFF, (ntohl(dst)>>16) & 0xFF, 
          (ntohl(dst)>>8) & 0XFF, ntohl(dst) & 0XFF , destinationPort);
	
	return fd;
}

/*
 * Open the sender side of the STP connection. Returns the pointer to
 * a newly allocated control block containing the basic information
 * about the connection. Returns NULL if an error happened.
 *
 * Note, to simplify things you should use connect(). When used with a
 * UDP socket all packets then sent and received on the given file
 * descriptor go to and are received from the specified host. Reads
 * and writes are still completed in a datagram unit size, but the
 * application does not have to do the multiplexing and
 * demultiplexing. This greatly simplifies things but restricts the
 * number of "connections" to the number of file descriptors and isn't
 * very good for a pure request response protocol like DNS where there
 * is no long term relationship between the client and server.
 */
stp_send_ctrl_blk * stp_open(char *destination, int destinationPort,
                             int receivePort) {

    printf ("Configuring  UDP \"connection\" to %s, sending to port %d listening for data on port %d\n", 
          destination, destinationPort, receivePort);
    
	stp_send_ctrl_blk *stp_CB = (stp_send_ctrl_blk *) malloc(sizeof(*stp_CB));
	
	
	
	if ((stp_CB->sock = open_udp(destination, destinationPort,receivePort) ) < 0) /* UDP socket descriptor */
	{
		return NULL; 
	}
	
	stp_CB->state = STP_SYN_SENT;	 /* protocol state*/
	stp_CB->swnd = SenderMaxWin;    /* latest advertised sender window */
	stp_CB->NBE = 0;        /* next byte expected */
	//stp_CB->LbACK;     /* last byte ACKed */
	//stp_CB->LBSent; 	/* last byte Sent not ACKed */

	stp_CB->ISN = 0;        /* initial sequence number */

	//stp_CB->sendQueue;
  
	
	
	
	sendpkt2(stp_CB-> sock, STP_SYN, stp_CB->swnd, stp_CB->NBE, 0,0,0);
	
	//readWithTimer(int fd, char *pkt, int ms);
	
	return stp_CB;
}



/*
 * Make sure all the outstanding data has been transmitted and
 * acknowledged, and then initiate closing the connection. This
 * function is also responsible for freeing and closing all necessary
 * structures that were not previously freed, including the control
 * block itself. Returns STP_SUCCESS on success or STP_ERROR on error.
 */
int stp_close(stp_send_ctrl_blk *stp_CB) {
  
  /* YOUR CODE HERE */
  return 0;
}


/*
 * This application is to invoke the send-side functionality. Feel
 * free to rewrite or write your own application to test your
 * code. Some examples of other applications that could be used
 * instead:
 * 
 * - A program that reads or generates a log and transmits it through
 *   STP;
 * - A program that reads the standard input and transmits it through
 *   STP;
 */
int main(int argc, char **argv) {
  
  stp_send_ctrl_blk *stp_CB;
  
  char *destinationHost;
  int receivePort, destinationPort;
  int file;
  
  /* You might want to change the size of this buffer to test how your
   * code deals with different packet sizes.
   */
  unsigned char buffer[STP_MSS];
  int num_read_bytes;
  
  /* Verify that the arguments are right*/
  if (argc != 5) {
    fprintf(stderr, "usage: SendApp DestinationIPAddress/Name receiveDataOnPort sendDataToPort filename \n");
    exit(1);
  }
  
  /*
   * Open connection to destination.  If stp_open succeeds the
   * stp_CB should be correctly initialized.
   */
  
  destinationHost = argv[1];
  receivePort = atoi(argv[2]);
  destinationPort = atoi(argv[3]);
  
  stp_CB = stp_open(destinationHost, destinationPort, receivePort);
  if (stp_CB == NULL) {
    /* YOUR CODE HERE */
	perror("stp_control_block cannot be NULL");
	exit(1);
  }
  
  /* Open file for transfer */
  file = open(argv[4], O_RDONLY);
  if (file < 0) {
    perror(argv[4]);
    stp_close(stp_CB);
    exit(1);
  }
  
  /* Start to send data in file via STP to remote receiver. Chop up
   * the file into pieces as large as max packet size and transmit
   * those pieces.
   */
  while(1) {
    num_read_bytes = read(file, buffer, sizeof(buffer));
    
    /* Break when EOF is reached */
    if(num_read_bytes <= 0)
      break;
    
    if(stp_send(stp_CB, buffer, num_read_bytes) == STP_ERROR) {
      /* YOUR CODE HERE */
	perror("STP_ERROR on send");
	exit(1);
    }
  }
  
  /* Close the connection to remote receiver */   
  if (stp_close(stp_CB) == STP_ERROR) {
    /* YOUR CODE HERE */
	perror("STP_CLOSE error");
	exit(1);
  }
  
  return 0;
}


