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

#define START_BYTE 0xEC
#define END_BYTE 0xED
#define CSI_COMMAND 0xF1

struct bob_msg{
	u_char start_byte;
	u_char command;
	u_char end_byte;
};

int validate_msg(struct bob_msg *msg){
	return (msg->start_byte == START_BYTE && msg->end_byte == END_BYTE);
}

//默认包大小为200
uint32_t packet_size = 1024;

//libpcap的全局变量
pcap_t *handle;
struct bpf_program fp;
int recv_socket;
char *iface = "wlan0";
FILE *file;

void stop_capture(int sig)
{
    printf("send SIGINT signal to terminate packet capture\n");
    //free resources
    pcap_freecode(&fp);
    pcap_close(handle);
    exit(0);
}

void break_loop(int sig)
{
    //timeout,关闭pcap_breakloop
    pcap_breakloop(handle);
    printf("timeout,break paploop\n");
}

void packet_process(u_char *user,const struct pcap_pkthdr *h,const u_char *bytes)
{

	//get bob's command: 1 for start | 2 for connect_ap
	
	char *command = "./init.sh";
	printf("packets comes\n");
	struct lorcon_packet *packet = (struct lorcon_packet *)(bytes+21);
	int plen = h->len - 25 - sizeof(struct lorcon_packet);
	printf("%d\n",plen);
	struct bob_msg *msg = (struct bob_msg *)(packet->payload);
	if(validate_msg(msg)==0){
		printf("invalid msg format\n");
		return;
	}
	u_char cmd = msg->command;
	switch(cmd){
		case CSI_COMMAND:
			//call the init script
			if(system(command)<0){
				printf("system error\n");
			}
			break;
		//other command
		default:
			break;
			
	}
	exit(0);
//	//cancel alarm
//	alarm(0);
//	
//	//jump over radiotap header:25+24+1024
//	//here four-byte ECC code may be added !!! radiotap header is 21
//	struct lorcon_packet *packet = (struct lorcon_packet *)(bytes+21);
//	int cnt;
//	int plen = h->len - 25 - sizeof(struct lorcon_packet);
//	printf("%d\n",plen);
//	cnt = fwrite(packet->payload,1,plen,file);
//	if(plen < 1024){
//		//transmit over
//		fclose(file);
//		exit(0);
//	}
//
//	//set alarm
//	alarm(1);
	
}


int main(int argc, char** argv)
{

	//alarm 1s to break pcap_loop
//	signal(SIGALRM,break_loop);
//	const char *filepath;
//	//1.open file for write
//	if(argc !=2 ){
//		printf("Usage: ./rv filepath\n");
//		return -1;
//	}
//	filepath = argv[1];
//	file = fopen(filepath,"wb");
//	if(file==NULL){
//		printf("open file error\n");
//		return -1;
//	}

	//1.初始化libpcap
    if(init_libpcap()<0){
            goto fail;
    }


    //2.wait for bob's signal
    if(pcap_loop(handle,-1,packet_process,NULL)<0){
            printf("pcap_loop error\n");
            goto fail;
    }
	
	printf("pcap_loop returns here\n");	
	
fail:
    pcap_freecode(&fp);
    pcap_close(handle);
	fclose(file);

	return 0;
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
    if((rule=fopen("filter_rule.conf","r"))==NULL){
        perror("open config file error,please create the config file for filter\n");
        return -1;
    }
    char filter[100];
    if(fgets(filter,100,rule)==NULL){
        perror("fgets config file error\n");
        fclose(rule);
        return -1;
    }
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
    fclose(rule);
    pcap_freecode(&fp);
    return 0;
}
