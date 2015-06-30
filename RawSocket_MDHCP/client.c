#include<stdio.h> 
#include<string.h>	//memset
#include<sys/socket.h>
#include<stdlib.h> 
#include<errno.h> 	//For errno - the error number
#include<netinet/udp.h> //Provides declarations for udp header
#include<netinet/ip.h>  //Provides declarations for ip header
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<netinet/if_ether.h>  //For ETH_P_ALL
#include<sys/ioctl.h>
#include<sys/types.h>
#include<unistd.h>
#include<time.h>
#include<ifaddrs.h>

unsigned short csum(unsigned short * ,int);
char * GetUDPData(unsigned char * , int);
void ProcessPacket(unsigned char * , int);
void Listen();
int isValidIpAddress(char * );
int SendPacket(char*, int , char* , int);

/*
0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
   +---------------+---------------+---------------+---------------+
   |                            xid (4)                            |
   +-------------------------------+-------------------------------+
   |           secs (2)            |           flags (2)           |
   +-------------------------------+-------------------------------+
   |                          ciaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          yiaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          siaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          giaddr  (4)                          |
   +---------------------------------------------------------------+
   |                                                               |
   |                          chaddr  (16)                         |
   |                                                               |
   |                                                               |
   +---------------------------------------------------------------+
   |                                                               |
   |                          sname   (64)                         |
   +---------------------------------------------------------------+
   |                                                               |
   |                          file    (128)                        |
   +---------------------------------------------------------------+
   |                                                               |
   |                          options (variable)                   |
   +---------------------------------------------------------------+
*/
struct mdhcphdr
{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	char sname[64];
	char file[128];
	uint8_t magiccookie[4];
	uint8_t options[0];
} * mdhcph_in;

struct sockaddr_in source, dest;
int udp,i,j; 
//int listeningOnPort = 50501;
int listeningOnPort = 67;
struct iphdr * iph;
struct udphdr * udph;
char * time_result;
int GOT_IT = 0;
char * eth4="eth4";
uint32_t client_ip=0x0A00020B;
//uint32_t server_ip=0x0A00020A;
char * ip_str="10.0.2.11";
//char port_str[100]="50500";
char port_str[100]="68";
int port_int = 0;


//global variavles for mdhcp packet parsing:
int mB,mM;
uint32_t leaseTime;
uint32_t serverId;


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

//checking if ip is valid
int isValidIpAddress(char *ipAddress)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
	return result;
}

//sending request for time then getting response
int main (int argc, char** argv)
{
	if (argc == 5) 
	{
		eth4 = argv[1];
		ip_str = argv[2];
		port_int=atoi(argv[3]);
		listeningOnPort=atoi(argv[4]);
	}
	else
	{
		return 1;
	}
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;

	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) 
	{
        	if (ifa->ifa_addr->sa_family==AF_INET) 
		{
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			addr = inet_ntoa(sa->sin_addr);
			if (strcmp(ifa->ifa_name,eth4)==0)
			{
				client_ip = ntohl(inet_addr(addr));
				printf("Interface: %s\tAddress: %s=%X\n", ifa->ifa_name, addr,client_ip);
			}
        	    
	        }
	}
    
    	freeifaddrs(ifap);

	char mdhcp_data[4096];
	memset (mdhcp_data, 0, 4096);
	struct mdhcphdr *mdhcph_out = (struct mdhcphdr *) mdhcp_data;	
	
	mdhcph_out->op=0x01;//request
	mdhcph_out->htype=0x01;//10mb, I guess
	mdhcph_out->hlen=0x06;//hwaddr length
	mdhcph_out->hops=0x00;
	mdhcph_out->xid=htonl(0x12345678);//crazy id
	mdhcph_out->secs=0x0000;//seconds do not matter
	mdhcph_out->flags=0x0040;//B=0 & M=1
	mdhcph_out->ciaddr=htonl(client_ip);
	//236bytes up until this point
	mdhcph_out->magiccookie[0]=0x63;
	mdhcph_out->magiccookie[1]=0x82;
	mdhcph_out->magiccookie[2]=0x53;	
	mdhcph_out->magiccookie[3]=0x63;
	//mdhcph->yiaddr=my_ip;
	//the rest ignored by mdhcp
	
	mdhcph_out->options[0]=61;
	mdhcph_out->options[1]=2;
	mdhcph_out->options[2]=0xFA;
	mdhcph_out->options[3]=0xAA;//client-id - random
	mdhcph_out->options[4]=105;
	mdhcph_out->options[5]=2;

	uint8_t b0 = (uint8_t)listeningOnPort;
     	uint8_t b1 = (uint8_t)(listeningOnPort>>8);

	mdhcph_out->options[6]=b1;
	mdhcph_out->options[7]=b0;
	//mdhcph_out->options[7]=0x43;
	//mdhcph_out->options[6]=0xC5;
	//mdhcph_out->options[7]=0x45;//client port - 50501
	mdhcph_out->options[8]=0xFF;//END_TAG

	int leng = 236+4+9;
	char msg[leng];
	for (i=0; i<leng;i++)
	{
		//printf("%X\n",(uint8_t)(mdhcp_data)[i]);
		msg[i]=(mdhcp_data)[i];
	}

	if (isValidIpAddress(ip_str)==0)
	{
		printf("Incorrect IP address!\n");
		return 1;
	}
	
	
	if (((port_int<=0)||(port_int>65535))||((listeningOnPort<=0)||(listeningOnPort>65535)))
	{
		printf("Incorrect port!\n");
		return 1;
	}

	printf("Your destination: %s:%d\n",ip_str,port_int);
     	time_result = "";
	int res_sent = SendPacket(ip_str, port_int, msg, leng);
	while (GOT_IT == 0)
	{
		Listen();
	}
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
			//check if datagram came in from the server
			if ((strcmp(inet_ntoa(source.sin_addr),ip_str)==0)&&(ntohs(source.sin_port)==port_int))
			{
				//get data from the packet
				mB=0;mM=0;
				leaseTime=0;
				serverId=0;
        			char * msg = GetUDPData(buffer, size);

				if ((mM==1)&&(mB==0)&&(mdhcph_in->op==0x02)&&(ntohl(mdhcph_in->xid)==0x12345678))
				{
					printf ("\nMDHCPACK from %08X:%d!\n",(uint32_t)ntohl(mdhcph_in->siaddr),port_int);
					GOT_IT = 1;
					printf ("\nNow I own multicast address %08X for %d seconds!\n",
						(uint32_t)ntohl(mdhcph_in->yiaddr),leaseTime);
					return;
				}
			}
		}
	}
	printf("Nah, nevermind.\n\n");
}

//extract data
char * GetUDPData(unsigned char * Buffer , int Size)
{
	//get rid of the header
	int header_size =  sizeof(struct ethhdr) + (iph->ihl*4) + sizeof udph;

 	char * msg = malloc(Size-header_size);
	
	printf("Got a udp datagramm (%d bytes) to port %d from %s:%d:\n", Size-header_size,ntohs(udph->dest), inet_ntoa(source.sin_addr),
		ntohs(source.sin_port));
	int i;
	
	mdhcph_in = (struct mdhcphdr * )(Buffer+header_size);

	printf("Parsing some of the MDHCP:\n");
	printf("Operation: %d\n", (uint8_t)mdhcph_in->op);
	printf("Client address: %X\n", (uint32_t)ntohl(mdhcph_in->ciaddr));
	printf("Transaction ID: %X\n", (uint32_t)ntohl(mdhcph_in->xid));
	printf("Flags: %04X\n", (uint16_t)mdhcph_in->flags);
		
	if (mdhcph_in->flags&0x0080)
	{
		mB=1;
	}
	else
	{
		mB=0;
	}
	if (mdhcph_in->flags&0x0040)
	{
		mM=1;
	}
	else
	{
		mM=0;
	}	
	printf("	B = %d\n",mB);
	printf("	M = %d\n",mM);
	printf("Options:\n");
	int o;	
	for(o=236+4;o<Size-header_size;o)
	{
		uint8_t opnumb = (uint8_t)(Buffer+header_size)[o];
		o++;

		printf("	%d: %02X",o, opnumb);
		if (opnumb==255)
		{
			printf(" - END TAG\n");
			break;
		}
		if (o==Size-header_size)
		{
			printf(" - Unknown option!");
			break;
		}
		uint8_t l = (uint8_t)(Buffer+header_size)[o++];
		uint8_t dat[l];
		int ind;
		for (ind=0;ind<l;ind++)
		{
			if (o==Size-header_size)
			{
				break;
			}
			dat[ind]=(uint8_t)(Buffer+header_size)[o++];
		}
		switch (opnumb)
		{
			case 54:
			serverId=dat[0]*256*256*256+dat[1]*256*256+dat[2]*256+dat[3];
			printf(" - Server identifier: %X\n", serverId);
			break;
			case 51:
			leaseTime=dat[0]*256*256*256+dat[1]*256*256+dat[2]*256+dat[3];
			printf(" - Lease time: %d\n", leaseTime);
			break;

			default:
			printf(" - Unknown option: %d ", l);
			for (ind=0;ind<l;ind++)
			{
				printf("%02X",dat[ind]);
			}
			printf("\n");
		}
			
	}

	printf("\nMSG:\n");
	for (i=0; i<Size-header_size;i++)
	{
		printf("%02X",(uint8_t)(Buffer+header_size)[i]);
		msg[i]=(Buffer+header_size)[i];
	}
	printf("\nTotal size of MDHCP header: %d\n",Size-header_size);
	
	return msg;
}

//send the packet
int SendPacket(char* ip_str, int port_int, char* msg, int leng)
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
	for (i=0; i<leng;i++)
	{
		data[i]=msg[i];
	}
     
	//some address resolution
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr (ip_str);




     
	//Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + leng;
	iph->id = htonl (12345); //Id of this packet
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
	udph->len = htons(8 + leng); //tcp header size
	udph->check = 0; //leave checksum 0 now, filled later by pseudo header

	//Now the UDP checksum using the pseudo header
	psh.source_address = inet_addr( source_ip );
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udp_length = htons(sizeof(struct udphdr) + leng );
     
	int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + leng;
	pseudogram = malloc(psize);
     
	memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + leng);
     
	udph->check = csum( (unsigned short*) pseudogram , psize);
     

	//Send the packet
	if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
		perror("sendto failed");
	}
	//Data send successfully
	else
	{
		printf ("Packet Send. Length : %d (%d) \n" , iph->tot_len, leng);
	}

	return 0;
}