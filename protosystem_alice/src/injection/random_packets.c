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

#include "util.h"

#define SEQ_POS 43

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
uint32_t packet_size=100;
uint32_t packet_id=0;
struct lorcon_packet *packet;

//libpcap的全局变量
pcap_t *handle;
struct bpf_program fp;
int recv_socket;
char *iface = "wlan0";
__le16 seq = 0;
FILE *file;

struct timeval sdtime;
struct timeval rtime;
int warning = 0;

uint32_t delay_us;
int32_t num_packets;


#define PAYLOAD_SIZE	2000000

static int init_libpcap(void);

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
    payload_memcpy(packet->payload, packet_size, ((packet_id++)*packet_size) % PAYLOAD_SIZE);
    packet->seq = seq;
    //记录发送时间
        if(gettimeofday(&sdtime,NULL)<0){
           perror("gettimeofday send time error\n");
        }
    tx80211_txpacket(&tx,&tx_packet);
}

void stop_capture(int sig)
{
    printf("send SIGINT signal to terminate packet capture\n");
    printf("warning:%d\n",warning);
    //free resources
    pcap_freecode(&fp);
    pcap_close(handle);
    fclose(file);
    exit(0);
}

void break_loop(int sig)
{
    //timeout,关闭pcap_breakloop
    pcap_breakloop(handle);
    printf("timeout,break paploop\n");
}

void terminate(int sig)
{
    pcap_freecode(&fp);
    pcap_close(handle);
    fclose(file);
    exit(0);
}

//a-b,假设a比b靠后,c中结构体的赋值是语言支持的,而数组的直接赋值不支持
//数组不支持直接赋值是因为数组是一个地址,赋值时长度不容易控制
struct timeval timesub(struct timeval a,struct timeval b)
{
    struct timeval ret;
    if(a.tv_usec<b.tv_usec){
        ret.tv_usec=a.tv_usec+1000000-b.tv_usec;
        ret.tv_sec=a.tv_sec-1-b.tv_sec;
    }else{
        ret.tv_sec=a.tv_sec-b.tv_sec;
        ret.tv_usec=a.tv_usec-b.tv_usec;
    }
    return ret;
}

void packet_process(u_char *user,const struct pcap_pkthdr *h,const u_char *bytes)
{
	/*
     *TODO 
     *
     *
     */
    //重置定时器
	if(h->len<145)
		return;
    struct itimerval zero;
    memset(&zero,0,sizeof(zero));
    if(setitimer(ITIMER_REAL,&zero,NULL)<0){
        perror("setitimer zero error:");
    }

    //获取当前时间rtime,并与sdtime比较,以记录从发包到收包所消耗的时间
    if(gettimeofday(&rtime,NULL)<0){
        perror("gettimeofday error:rtime\n");
    }
    //记录rss和seq
    __le16 temp_seq=*((__le16 *)(bytes+SEQ_POS));
    printf("seq:%u\n",temp_seq);
    //比较temp_seq和seq,若temp_seq小于seq,说明之前的包,超时时间设的有点短,warning
    if(temp_seq<seq){
       printf("WARNING:original packet considered loss appear:%d,%d\n",temp_seq,seq);
       warning++;
       return;
    }
    //显示时间
    struct timeval tmp=timesub(rtime,sdtime);
    printf("%ld:%06ld\n",tmp.tv_sec,tmp.tv_usec);
    //记录到文件中
	/*
    int len=fprintf(file,"%d:%d\n",temp_seq,rssi);
    if(len<0){
        perror("fprintf error\n");
    }
	*/
    //休眠100ms,10packets/s
    //发包的时间间隔也有要考虑,多改改试试
    usleep(delay_us);
    //发送下一个数据包
    seq++;
	//check num_packets
	if(num_packets-- <= 0){
		printf("breakloop due to num reached\n");
		send_beacon(0);
		raise(SIGINT);
		return;
		//return to pcap_loop
    	pcap_freecode(&fp);
    	//pcap_close(handle);
    	fclose(file);
		exit(0);
	}
    send_beacon(seq);
    //重置定时器
    struct itimerval new; 
    memset(&new,0,sizeof(new));
    new.it_value.tv_sec=0;
    //休眠0.1s,即100ms,即100000us
    new.it_value.tv_usec=delay_us;
    if(setitimer(ITIMER_REAL,&new,NULL)<0){
        perror("setitimer error:");
        exit(-1);
    }
}

int main(int argc, char** argv)
{
//we can intergate rss and csi together

	//set command args to control packets_num and time interval	

	uint32_t mode = 1;
	

	if(argc > 3){
		printf("Usage: random_packets <number> <delay in us>\n");
		return -1;
	}
	
	if(argc<3 || !sscanf(argv[2],"%u",&delay_us)){
		delay_us = 100000;
	}

	if(argc<2 || !sscanf(argv[1],"%u",&num_packets)){
		num_packets = 200;
	}



	//初始化sdtime,rtime
	memset(&sdtime,0,sizeof(sdtime));
	memset(&rtime,0,sizeof(rtime));

	//设置信号处理函数
	signal(SIGINT,stop_capture);
	signal(SIGALRM,break_loop);
	//signal(SIGUSR1,terminate);


	/* Generate packet payloads */
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
		get_mac_address(packet->addr2, "mon0");
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
	} else if (mode == 1) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		//memcpy(packet->addr2, "\x00\x16\xea\x56\x34\x12", 6);
		memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);
	}
	packet->seq = 0;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet.plen = sizeof(*packet) + packet_size;


	//3.初始化libpcap
	if(init_libpcap()<0){
        	goto fail;
    	}

	//4.发包交互流程开始
while(num_packets--){ 
	//seq starts at 1, 0 as a termination flag
	seq++;
	send_beacon(seq);
	//sleep(1);
	//continue;
	printf("send packets in main\n");
	struct itimerval new; 
	memset(&new,0,sizeof(new));
	new.it_value.tv_sec=0;
	
	new.it_value.tv_usec=delay_us;
	if(setitimer(ITIMER_REAL,&new,NULL)<0){
	    perror("setitimer error:");
	    return -1;
	}
	
	//this is a loop!
	pcap_loop(handle,-1,packet_process,NULL);
}

//packets num reached, stop
printf("packets num reached\n");

//tell bob to terminate, seq==-1 as a flag
send_beacon(0);


//release resources:
pcap_freecode(&fp);
pcap_close(handle);

return 0;

fail:
    pcap_freecode(&fp);
    pcap_close(handle);
    return -1;

	
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
    /*
     *获取socket描述符
     */
    recv_socket=pcap_fileno(handle);
    printf("recv_socket:%d\n",recv_socket);
    /*
     * 设置为nonblock模式,注意nonblock模式下只能使用pcap_dispatch,如果没有数据，
     * pcap_dispatch立即返回0
     * 注意handle->fd就是socket描述符,也可以使用fcntl(handle->fd,F_SETFL,flags)
     * 设置非阻塞模式
     */
    //这里没啥用
    //if(pcap_setnonblock(handle,1,errbuf)<0){
    //    pcap_perror(handle,"pcap_setnonblock error:");
    //    return -1;
    //}
    //编译和设置过滤规则
    //从文件中读取过滤规则
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
    //使用BPF时,要enable "immediate mode"防止系统buffer for us
    //好像只有BSD系列的支持
    //on Linux, this is currently not necessary - what buffering is done
    //doesn't have a timeout for the delivery of packets
    //unsigned int on=1;
    //ioctl(recv_socket,BIOCIMMEDIATE,&on);
    return 0;
}
