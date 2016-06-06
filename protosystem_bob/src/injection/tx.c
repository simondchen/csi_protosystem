/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <tx80211.h>
#include <tx80211_packet.h>

#include <pcap/pcap.h>

#include <signal.h>

#define SEQ_POS 43

#include "util.h"

static void init_lorcon();

struct lorcon_packet
{
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	u_char	payload[0];
} __attribute__ ((packed));

struct tx80211	tx;
struct tx80211_packet	tx_packet;
uint8_t *payload_buffer;
uint32_t packet_id = 0;
//默认包大小为200
uint32_t packet_size = 1024;
struct lorcon_packet *packet;

//libpcap的全局变量
pcap_t *handle;
struct bpf_program fp;
int recv_socket;
char *iface = "wlan0";
FILE *file;

#define PAYLOAD_SIZE	2000000

static inline void payload_memcpy(uint8_t *dest, uint8_t *src, uint32_t length)
{
	uint32_t i;
	for (i = 0; i < length; ++i) {
		dest[i] = src[i];
	}
}



void send_beacon(__le16 seq)
{
	int ret;
   	packet->seq = seq;
	ret = tx80211_txpacket(&tx, &tx_packet);
	if (ret < 0) {
		fprintf(stderr, "Unable to transmit packet: %s\n",
				tx.errstr);
		exit(1);
	}
}

int main(int argc, char** argv)
{
	const char *filepath;

	FILE *file;
	//FILE *fout;

	size_t cnt;

	__le16 seq = 0;

	if(argc != 2){
		printf("Usage: ./tx filepath\n");
		return -1;
	}
	
	filepath = argv[1];
	printf("%s\n",filepath);


	//1.open file for write, byte format
	file=fopen(filepath,"rb");
	if(file==NULL){
		printf("open file error\n");
		return -1;
	}

	/*
	fout=fopen("test.dat","w+");
	if(fout==NULL){
		printf("open file error\n");
		return -1;
	}
	*/


	//2. init lorcon
	printf("Initializing LORCON\n");
	init_lorcon();

	//3. allocat packet and set header
	packet = malloc(sizeof(*packet) + packet_size);
	if (!packet) {
		perror("malloc packet");
		exit(1);
	}
	packet->fc = (0x08 
				| (0x0 << 8) );
	packet->dur = 0xffff;

	memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
	memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
	memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);

	packet->seq = seq;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet.plen = sizeof(*packet) + packet_size;


	//4. read file and send
	//size and nmemb!
	//differ cnt == packet_size and cnt < packet_size
	while((cnt = fread(packet->payload,1,packet_size,file)) == packet_size){
		printf("%d\n",cnt);
		//fwrite(packet->payload,1,cnt,fout);
		send_beacon(seq++);		
	}
	if(cnt > 0){
		printf("%d\n",cnt);
		//fwrite(packet->payload,1,cnt,fout);
		tx_packet.plen = sizeof(*packet) + cnt;
		send_beacon(seq++);
	}

	free(packet);
	fclose(file);
	//fclose(fout);
	return 0;
}

static void init_lorcon()
{
	/* Parameters for LORCON */
	int drivertype = tx80211_resolvecard("iwlwifi");

	/* Initialize LORCON tx struct */
	if (tx80211_init(&tx, "wlan0", drivertype) < 0) {
		fprintf(stderr, "Error initializing LORCON: %s\n",
				tx80211_geterrstr(&tx));
		exit(1);
	}
	if (tx80211_open(&tx) < 0 ) {
		fprintf(stderr, "Error opening LORCON interface\n");
		exit(1);
	}

	/* Set up rate selection packet */
	tx80211_initpacket(&tx_packet);
}
