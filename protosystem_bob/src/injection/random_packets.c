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
static int init_libpcap();

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
uint32_t packet_size = 100;
struct lorcon_packet *packet;

//libpcap的全局变量
pcap_t *handle;
struct bpf_program fp;
int recv_socket;
char *iface = "wlan0";
//__le16 seq = 0;
FILE *file;


#define PAYLOAD_SIZE	2000000

static inline void payload_memcpy(uint8_t *dest, uint32_t length,
		uint32_t offset)
{
	uint32_t i;
	for (i = 0; i < length; ++i) {
		dest[i] = payload_buffer[(offset + i) % PAYLOAD_SIZE];
	}
}



void send_beacon(__le16 seq)
{
	int ret;
	payload_memcpy(packet->payload, packet_size,
		((packet_id++)*packet_size) % PAYLOAD_SIZE);
    	packet->seq = seq;
	ret = tx80211_txpacket(&tx, &tx_packet);
	if (ret < 0) {
		fprintf(stderr, "Unable to transmit packet: %s\n",
				tx.errstr);
		exit(1);
	}
}

void stop_capture(int sig)
{
    printf("send SIGINT signal to terminate packet capture\n");
    //free resources
    pcap_freecode(&fp);
    pcap_close(handle);
    fclose(file);
    exit(0);
}

void packet_process(u_char *user,const struct pcap_pkthdr *h,const u_char *bytes)
{
	if(h->len<145)
		return;
   	__le16 seq=*((__le16 *)(bytes+SEQ_POS));
	//0 as a termination flag
	if(seq == 0){
		//segment fault occurs here
		printf("terminate packet capture due to signal from alice\n");
		raise(SIGINT);
		return;
		//free resources
		pcap_freecode(&fp);
		//pcap_close(handle);
		fclose(file);
		exit(0);
		
	}
   	printf("seq:%d\n",seq);
   	send_beacon(seq);
}


int main(int argc, char** argv)
{
	uint32_t mode=1;


	//设置信号处理函数
   	signal(SIGINT,stop_capture);

	printf("Generating packet payloads \n");
	payload_buffer = malloc(PAYLOAD_SIZE);
	if (payload_buffer == NULL) {
		perror("malloc payload buffer");
		exit(1);
	}
	generate_payloads(payload_buffer, PAYLOAD_SIZE);

	/* Setup the interface for lorcon */
	printf("Initializing LORCON\n");
	init_lorcon();

	/* Allocate packet */
	packet = malloc(sizeof(*packet) + packet_size);
	if (!packet) {
		perror("malloc packet");
		exit(1);
	}
	packet->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
	packet->dur = 0xffff;
	if (mode == 0) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		get_mac_address(packet->addr2, "wlan0");
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
	} else if (mode == 1) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		//memcpy(packet->addr1, "\x00\x16\xea\x56\x34\x12 ", 6);
		//memcpy(packet->addr2, "\x00\x16\xea\x56\x34\x12 ", 6);
		memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);
	}
	__le16 seq= 0x8000;
	packet->seq = seq;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet.plen = sizeof(*packet) + packet_size;


	//3.初始化libpcap
    if(init_libpcap()<0){
            goto fail;
    }


    //4.发包交互流程开始
    if(pcap_loop(handle,-1,packet_process,NULL)<0){
            printf("pcap_loop error\n");
            goto fail;
    }
	
	
	
fail:
    pcap_freecode(&fp);
    pcap_close(handle);
    return -1;


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

int init_libpcap(void)
{
    char errbuf[1024]={'\0',};
    handle=pcap_open_live(iface,BUFSIZ,1,0,errbuf);
    if(handle==NULL){
            printf("pcap_open_live error:%s\n",errbuf);
            return -1;
    }
    recv_socket=pcap_fileno(handle);
    printf("recv_socket:%d\n",recv_socket);
    FILE *rule=NULL;
    if((rule=fopen("conf/filter_rule.conf","r"))==NULL){
        perror("open config file error,please create the config file for filter\n");
        return -1;
    }
    char filter[100];
    if(fgets(filter,100,rule)==NULL){
        perror("fgets config file error\n");
        fclose(rule);
        return -1;
    }
    fclose(rule);
    //去掉换行符
    int len=strlen(filter);
    filter[len-1]='\0';
    //printf("rule:%s\n",filter);
    if(pcap_compile(handle,&fp,filter,0,0)<0){
        pcap_perror(handle,"pcap_compile error:");
        return -1;
    }
    if(pcap_setfilter(handle,&fp)<0){
        pcap_perror(handle,"pcap_setfilter error:");
        return -1;
    }
    pcap_freecode(&fp);
    return 0;
}
