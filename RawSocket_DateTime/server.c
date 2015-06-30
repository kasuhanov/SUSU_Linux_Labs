#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h> 
#include<stdlib.h>    
#include<string.h>    
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/ip.h>    //Provides declarations for ip header
#include<netinet/if_ether.h>  //For ETH_P_ALL
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<time.h>
 
void ProcessPacket(unsigned char * , int);
char * GetUDPData(unsigned char * , int);
void Listen();
int SendPacket(char * , int, char * );
unsigned short csum(unsigned short * ,int) ;

struct sockaddr_in source, dest;
int udp,i,j; 
int listeningOnPort = 8622;
struct iphdr * iph;
struct udphdr * udph;

//pseudo-header for udp checksum
struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};

//checksum
unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;
 
	sum=0;
	while(nbytes>1)
	{
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1)
	{
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}
 
	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
     
	return(answer);
}

//just keeps listening 
int main()
{
	while (1)
	{
		Listen();
	}
	printf("Finished");
	return 0;
}

//Listens for a single packet
void Listen()
{
	int saddr_size , data_size;
	struct sockaddr saddr;
	unsigned char *buffer = (unsigned char *) malloc(65536);
     
	printf("Listening on all the interfaces on UDP port %d...\n",listeningOnPort);
	int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	if(sock_raw < 0)
	{
		//Print the error with proper message
		perror("Socket Error");
		return;
	}

	saddr_size = sizeof saddr;
	//Receive a packet
	data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , (socklen_t*)&saddr_size);
	if(data_size <0 )
	{
		printf("Recvfrom error , failed to get packets\n");
		return;
	}
	//Now process the packet
	printf("Got something!\n");
	ProcessPacket(buffer , data_size);

	close(sock_raw);
	return;
}

//Process recieved packet 
void ProcessPacket(unsigned char* buffer, int size)
{
	//Get the IP Header part of this packet , excluding the ethernet header
	iph = (struct iphdr *)(buffer +  sizeof(struct ethhdr));
	if (iph->protocol==17) //UDP Protocol
	{
		//Get the UDP Header
		udph = (struct udphdr *)(buffer + (iph->ihl*4)  + sizeof(struct ethhdr));
		++udp;
		//Datagram came in onto port we designated for our application
		if (ntohs(udph->dest)==listeningOnPort)
		{
			//get the source
			memset(&source, 0, sizeof(source));
			source.sin_addr.s_addr = iph->saddr;
			source.sin_port = udph->source;
			//get data from the packet
        		char * msg = GetUDPData(buffer, size);
			//if data includes request for time...
			if (strcmp("GET_SERVER_TIME_BIATCH",msg)==0)
			{
				time_t rawtime;
				struct tm * timeinfo;
	
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				//...then send time to the source!
				printf ("Sending date and time to him:\n%s", asctime (timeinfo) );
				int res_sent = SendPacket(inet_ntoa(source.sin_addr), ntohs(udph->source), asctime (timeinfo));
				printf("Sent!\n\n");
				return;
			}
		}
	}
	//if, at any point we realized that this packet is not time-request, drop it
	printf("Nah, nevermind.\n\n");
}

//extract data
char * GetUDPData(unsigned char * Buffer , int Size)
{
	//get rid of the header
	int header_size =  sizeof(struct ethhdr) + (iph->ihl*4) + sizeof udph;

 	char * msg = malloc(Size-header_size);
	
	printf("Got a udp datagramm to port %d from %s:%d:\n", ntohs(udph->dest), inet_ntoa(source.sin_addr),
		ntohs(source.sin_port));
	int i;
	
	for (i=0; i<Size-header_size;i++)
	{
		printf("%c",(Buffer+header_size)[i]);
		msg[i]=(Buffer+header_size)[i];
	}
	printf("\n");
	
	return msg;
}
//send the packet
int SendPacket(char* ip_str, int port_int, char* msg)
{
	int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
     
	if(s == -1)
	{
		//socket creation failed, may be because of non-root privileges
		perror("Failed to create raw socket");
		return 1;
	}
     
	//Datagram to represent the packet
	char datagram[4096] , source_ip[32] , *data , *pseudogram;
     
	//zero out the packet buffer
	memset (datagram, 0, 4096);
     
	//IP header
	struct iphdr *iph = (struct iphdr *) datagram;
     
	//UDP header
	struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip));
     
	struct sockaddr_in sin;
	struct pseudo_header psh;
     
	//Data part
	data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
	strcpy(data , msg);
     
	//some address resolution
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr (ip_str);
     
	//Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + strlen(data);
	iph->id = htonl (54321); //Id of this packet
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;      //Set to 0 before calculating checksum
	iph->daddr = sin.sin_addr.s_addr;
     
	//Ip checksum
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);
     
	//UDP header
	udph->source = htons (listeningOnPort);
	udph->dest = htons (port_int);
	udph->len = htons(8 + strlen(data)); //tcp header size
	udph->check = 0; //leave checksum 0 now, filled later by pseudo header

	//Now the UDP checksum using the pseudo header
	psh.source_address = inet_addr( source_ip );
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );
     
	int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
	pseudogram = malloc(psize);
     
	memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + strlen(data));
     
	udph->check = csum( (unsigned short*) pseudogram , psize);
     

	//Send the packet
	if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
		perror("sendto failed");
	}
	//Data send successfully
	else
	{
		printf ("Packet Send. Length : %d \n" , iph->tot_len);
	}

	return 0;
}