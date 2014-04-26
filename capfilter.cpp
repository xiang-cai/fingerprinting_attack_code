#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <sys/time.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_set>
using namespace std;

#define SSH 1
#define TOR 2

#define PARSE SSH

typedef struct _ret{
	unsigned long long local_delay;
	unsigned long long monitored_delay;
	unsigned long local_bytes;
	unsigned long monitored_bytes;
}RET;

class Filter{
	public:
		unordered_set <string> server_ips;
		unordered_set <string> client_ips;
		int PROXYPORT_MIN;
		int PROXYPORT_MAX;
	public:
		Filter(char* clientipfname, char* serveripfname, int portmin, int portmax);
		int read_ips(unordered_set<string> &set, char* fname);
		bool is_onload(u_char* payload);
		bool contains(unordered_set<string> set, string str);
		bool is_monitoredtraffic(char* src, unsigned int sport, char* dst, unsigned int dport);
		bool should_break(struct pcap_pkthdr* header, struct timeval* pre_local_time, struct timeval* pre_local_time_downstream, unsigned long long* start, bool is_onload, bool is_local, bool is_downstream, unsigned long long local_delay, unsigned long long local_last);
		RET parse_one(char* capfname, int proxy_port_min, int proxy_port_max, int remove_ack, char* monitoredoutname, char* localoutname, char* c2stau, char* s2ctau, char* timeseq);
		unsigned long long pre_gap(struct pcap_pkthdr* header, struct timeval* pre_time, char* src, char* dst);
};

Filter::Filter(char* clientipfname, char* serveripfname, int portmin, int portmax){
	read_ips(client_ips, clientipfname);
	read_ips(server_ips, serveripfname);
	PROXYPORT_MIN = portmin;
	PROXYPORT_MAX = portmax;
}

bool Filter::contains(unordered_set<string> set, string str){
	if(set.find(str) != set.end())
		return true;
	return false;
}

int Filter::read_ips(unordered_set<string> &set, char* fname){
	//read ips from a file fname, put them into the hashset set 
	FILE* fp = NULL;
	char ip[20];

	fp = fopen(fname,"r");
	if(!fp){
		printf("cannot load file %s\n",fname);
		exit(1);
	}
	
	set.clear();
	while(!feof(fp)){
		fscanf(fp,"%s",ip);
		if(strcmp(ip,"") != 0)
			set.insert(ip);
		memset(ip,0,20);
	}
	fclose(fp);
	return 0;
}

bool Filter::is_onload(u_char* payload){
	const char* onload = "\x05\x04\x00\x01\xab\xcd\xef\x00\x00\x50";
	/*
	for(int i = 0; i < 10; i++){
		printf("%02X", (unsigned char)payload[i]);
	}
	printf("\n");
	*/
	for(int i = 0; i < 10; i++){
		if((unsigned char)payload[i] != (unsigned char)onload[i]){
	//		printf("%d th char not the same: %02X -- %02X\n", i, (unsigned char)payload[i], onload[i]);
			return false;
		}
	}
	return true;
}

bool Filter::is_monitoredtraffic(char* src, unsigned int sport, char* dst, unsigned int dport){
	if((contains(client_ips, src) && contains(server_ips, dst)) || (contains(client_ips, dst) && contains(server_ips, src))){
		if(PARSE == TOR){
			// tor, no fixed ports
			return true;
		}
		else if(PARSE == SSH){
			// ssh, need to check ports to
			if((sport <= PROXYPORT_MAX && sport >= PROXYPORT_MIN) || (dport <= PROXYPORT_MAX && dport >= PROXYPORT_MIN))
				return true;
		}
	}
	return false;
}

unsigned long long Filter::pre_gap(struct pcap_pkthdr* header, struct timeval* pre_time, char* src, char* dst){
	unsigned long long now = (unsigned long long)((*header).ts.tv_sec)*1000000+(*header).ts.tv_usec;
	unsigned long long pre = (unsigned long long)(pre_time->tv_sec)*1000000+pre_time->tv_usec;
	unsigned long long ret = 0;
	if(pre == 0 || now < pre)
		ret = 0;
	else
		ret = now - pre;
	pre_time->tv_sec = (*header).ts.tv_sec;
	pre_time->tv_usec = (*header).ts.tv_usec;

//	if(ret > 30000){
//		printf("now: %lld , gap: %lld : %s --> %s\n", now, ret, src, dst);
//	}
	return ret;
}

bool Filter::should_break(struct pcap_pkthdr* header, struct timeval* pre_time, struct timeval* pre_time_downstream, unsigned long long* start, bool found_onload, bool is_local, bool is_downstream, unsigned long long local_delay, unsigned long long local_last){
	unsigned long long now = (unsigned long long)((*header).ts.tv_sec)*1000000+(*header).ts.tv_usec;
	unsigned long long pre = (unsigned long long)(pre_time->tv_sec)*1000000+pre_time->tv_usec;
//	printf("now: %llu -- pre: %llu\n", now, pre);
	if(pre == 0)
		*start = now;

//	printf("is_local: %d, local_delay: %lld, now: %lld, pre: %lld, now-pre: %lld, found_onload: %d\n",is_local == true, local_delay, now, pre, now-pre, found_onload == true);	
//	if(is_local){
//		if(found_onload || (pre > 0 && now-pre > 4000000)){
//			cout<<"local break"<<endl;
//			return true;
//		}
//	}
//	else{
//		if((local_delay > 0 || now - local_last > 4000000) && pre > 0 && now-pre > 4000000){
//			cout<<"monitored break"<<endl;
//			return true;
//		}
//	}
	pre_time->tv_sec = (*header).ts.tv_sec;
	pre_time->tv_usec = (*header).ts.tv_usec;

	if(is_downstream){
		pre_time_downstream->tv_sec = (*header).ts.tv_sec;
		pre_time_downstream->tv_usec = (*header).ts.tv_usec;
	}
	return false;
}


RET Filter::parse_one(char* capfname, int proxy_port_min, int proxy_port_max, int remove_ack, char* monitoredoutname, char* localoutname, char* c2stau, char* s2ctau, char* timeseq){
	bool found_onload = false;
	RET ret;
	struct timeval c2spre_time;
	struct timeval s2cpre_time;
	struct timeval pre_local_time;
	struct timeval pre_local_time_downstream;
	struct timeval pre_monitored_time;
	struct timeval pre_monitored_time_downstream;
	unsigned long long local_start,monitored_start,local_delay, monitored_delay;
	unsigned long local_bytes, monitored_bytes;
	vector<unsigned long long> c2sgaps;
	vector<unsigned long long> s2cgaps;

	struct pcap_pkthdr header;
	const u_char* packet;
	u_char* pkt_ptr;
	struct ip* ip_header;
	struct tcphdr* tcp_header;
	int pkt_length;
	int checksum;
	unsigned short sport, dport;

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* handle = NULL;

	FILE* eout = NULL;
	eout = fopen("./filtererror","a+");
	if(!eout){
		printf("cannot open output file filtererror\n");
		exit(1);
	}

	//open cap file
	handle = pcap_open_offline(capfname, errbuf);
	if(handle == NULL || pcap_datalink(handle) != 113){
		fprintf(eout, "cannot open file %s, or data link type is not 113\n", capfname);
		fclose(eout);
		exit(1);
	}
//	cout<<"link: "<<pcap_datalink(handle)<<endl;
	
	// processing packets from fname

	u_short ether_type;
	char src[20];
	char dst[20];
	struct ether_header* e_header;

	// open output file

	FILE* monitoredfout = NULL;
	FILE* localfout = NULL;
	FILE* monitoredtimefout = NULL;
	monitoredfout = fopen(monitoredoutname,"w");
	if(!monitoredfout){
		printf("cannot open output file %s\n",monitoredoutname);
		exit(1);
	}
	
	localfout = fopen(localoutname,"w");
	if(!localfout){
		printf("cannot open output file %s\n",localoutname);
		fclose(monitoredfout);
		exit(1);
	}

	monitoredtimefout = fopen(timeseq,"w");
	if(!monitoredtimefout){
		printf("cannot open output file %s\n",timeseq);
		fclose(monitoredfout);
		fclose(localfout);
		exit(1);
	}

	c2spre_time.tv_sec = c2spre_time.tv_usec = 0;
	s2cpre_time.tv_sec = s2cpre_time.tv_usec = 0;
	pre_local_time.tv_sec = pre_local_time.tv_usec = 0;
	pre_local_time_downstream.tv_sec = pre_local_time_downstream.tv_usec = 0;
	pre_monitored_time.tv_sec = pre_monitored_time.tv_usec = 0;
	pre_monitored_time_downstream.tv_sec = pre_monitored_time_downstream.tv_usec = 0;
	local_start = monitored_start = local_delay = monitored_delay = 0;
	local_bytes = monitored_bytes = 0;

	while(packet = pcap_next(handle, &header)){
		pkt_ptr = (u_char*)packet;
//		e_header = (struct ether_header*)packet;
//		ether_type = ntohs(e_header->ether_type);
		u_short* tmp = (u_short*)(pkt_ptr+14);
		ether_type = ntohs(*tmp);
//		printf("sll ether_type: %x\n", ntohs(*tmp));

		if(ether_type == 0){
			pkt_ptr += 2;
			packet += 2;
			e_header = (struct ether_header*)packet;
			ether_type = ntohs(e_header->ether_type);
		}
		if(ether_type != ETHERTYPE_IP){
//			printf("unknown ether_type %x\n", ether_type);
			continue;
		}

//		pkt_ptr += sizeof(struct ether_header);
		pkt_ptr += 16;
		ip_header = (struct ip*)(pkt_ptr);
		pkt_length = ntohs(ip_header->ip_len);
//		cout<<"packet_len: "<<pkt_length<<endl;
	/*
		pkt_ptr += sizeof(struct ip);
	*/
		int ip_header_len = (ip_header->ip_hl & 0x0f)*4;
//		cout<<"ip_header_len: "<<ip_header_len<<endl;
		pkt_ptr += ip_header_len;
		
		tcp_header = (struct tcphdr*)(pkt_ptr);
		//added
		int tcp_doff = tcp_header->doff * 4;
		pkt_ptr += tcp_doff;
//		cout<<"tcp_doff: "<<tcp_doff<<endl;
		u_char* payload = (u_char*)pkt_ptr;

		if(!found_onload){
			if(pkt_length >= ip_header_len+tcp_doff+10){
				/*
				for(int i = 0; i < 10; i++){
					printf("%02X",(unsigned char)payload[i]);
				}
				printf("\n");
				*/
				found_onload = is_onload(payload);
			}
		}
		
		memset(src,0,20);
		memset(dst,0,20);
		strcpy(src, inet_ntoa(ip_header->ip_src));
		strcpy(dst, inet_ntoa(ip_header->ip_dst));

		sport = ntohs(tcp_header->source);
		dport = ntohs(tcp_header->dest);

//		printf("src: %s -- dst: %s , len: %d, src_p: %u, dst_p: %u\n", src, dst, pkt_length, sport, dport);		

		if(PARSE == SSH){
			if(remove_ack == 1 && pkt_length <= 84)
				continue;
		}
		else{
			if(remove_ack == 1 && pkt_length <= 52)
				continue;
		}


		if(!is_monitoredtraffic(src, sport, dst, dport)){
			if(local_delay == 0 && strcmp(src, "127.0.0.1") == 0 && strcmp(dst, "127.0.0.1") == 0 && ((sport <= PROXYPORT_MAX && sport >= PROXYPORT_MIN) || (dport <= PROXYPORT_MAX && dport >= PROXYPORT_MIN))){
				local_bytes += pkt_length;
				if(should_break(&header, &pre_local_time, &pre_local_time_downstream, &local_start, found_onload, true, (sport <= PROXYPORT_MAX && sport >= PROXYPORT_MIN), local_delay, (unsigned long long)(pre_local_time.tv_sec)*1000000+pre_local_time.tv_usec)){
					if(pre_local_time_downstream.tv_sec != 0 || pre_local_time_downstream.tv_usec != 0)
						local_delay = (unsigned long long)(pre_local_time_downstream.tv_sec)*1000000+pre_local_time_downstream.tv_usec - local_start;
				}
//				pkt_length -= 52;
				if(dport <= PROXYPORT_MAX && dport >= PROXYPORT_MIN){
					pkt_length *= -1;
				}
				fprintf(localfout, "%d\n", pkt_length);
//				fprintf(localtimefout, "%llu\n", (unsigned long long)(header.ts.tv_sec)*1000000+header.ts.tv_usec);

			}
			continue;
		}
		
		//monitored packets

		if(monitored_delay == 0){
			monitored_bytes += pkt_length;
			if(should_break(&header, &pre_monitored_time, &pre_monitored_time_downstream, &monitored_start, found_onload, false, contains(server_ips, src), local_delay, (unsigned long long)(pre_local_time.tv_sec)*1000000+pre_local_time.tv_usec)){
				
				if(pre_monitored_time_downstream.tv_sec != 0 || pre_monitored_time_downstream.tv_usec != 0)
					monitored_delay = (unsigned long long)(pre_monitored_time_downstream.tv_sec)*1000000+pre_monitored_time_downstream.tv_usec - monitored_start;
				break;
			}
		}
		
		if(contains(server_ips, dst))
			c2sgaps.push_back(pre_gap(&header, &c2spre_time, src, dst));
		else
			s2cgaps.push_back(pre_gap(&header, &s2cpre_time, src, dst));

		
//		pkt_length -= 52;
		if(contains(client_ips, src))
			pkt_length *= -1;
		fprintf(monitoredfout, "%d\n", pkt_length);
		fprintf(monitoredtimefout, "%llu\n", (unsigned long long)(header.ts.tv_sec)*1000000+header.ts.tv_usec);
//		printf("src: %s -- dst: %s , len: %d, src_p: %u, dst_p: %u\n", src, dst, pkt_length, sport, dport);		
	}

	if(local_delay == 0){
//		cout<<"compute local at last"<<endl;
		if(pre_local_time_downstream.tv_sec != 0 || pre_local_time_downstream.tv_usec != 0)
			local_delay = (unsigned long long)(pre_local_time_downstream.tv_sec)*1000000+pre_local_time_downstream.tv_usec - local_start;
	}
	if(monitored_delay == 0){
//		cout<<"compute monitored at last"<<endl;
		if(pre_monitored_time_downstream.tv_sec != 0 || pre_monitored_time_downstream.tv_usec != 0)
			monitored_delay = (unsigned long long)(pre_monitored_time_downstream.tv_sec)*1000000+pre_monitored_time_downstream.tv_usec - monitored_start;
	}
	fclose(monitoredfout);
	fclose(monitoredtimefout);
	fclose(localfout);
	
	FILE* c2sout = NULL;
	c2sout = fopen(c2stau,"w");
	if(!c2sout){
		printf("cannot open output tau file %s\n",c2stau);
		exit(1);
	}
	for(int i = 0; i < c2sgaps.size(); i++){
		fprintf(c2sout,"%lld\n", c2sgaps.at(i));
	}
	fclose(c2sout);
	
	FILE* s2cout = NULL;
	s2cout = fopen(s2ctau,"w");
	if(!s2cout){
		printf("cannot open output tau file %s\n",s2ctau);
		exit(1);
	}
	for(int i = 0; i < s2cgaps.size(); i++){
		fprintf(s2cout,"%lld\n", s2cgaps.at(i));
	}
	fclose(s2cout);

	ret.local_delay = local_delay;
	ret.monitored_delay = monitored_delay;
	ret.local_bytes = local_bytes;
	ret.monitored_bytes = monitored_bytes;
	return ret;
}


int main(int argc, char** argv){
	if(argc < 8){	
		printf("./capfilter logfolder clientipfname serveripfname proxyport_min proxyport_max if_remove_ack capfname1, capfname2 ... \n");
		exit(1);
	}
	
	int i = 0;
	int proxy_port_min = atoi(argv[4]);
	int proxy_port_max = atoi(argv[5]);
	int if_remove_ack = atoi(argv[6]);
	char monitoredoutname[500];
	char localoutname[500];
	char c2stau[500];
	char s2ctau[500];
	char localdelayname[500];
	char monitoreddelayname[500];
	char overheadname[500];
	char timeseq[500];
	FILE* ldelay = NULL;
	FILE* mdelay = NULL;
	FILE* overhead = NULL;
	Filter cfilter(argv[2], argv[3], proxy_port_min, proxy_port_max);
	string tmp;
	int index;
	RET ret;
	
	memset(localdelayname,0,500);
	sprintf(localdelayname, "%slocaldelays.txt",argv[1]);
	ldelay = fopen(localdelayname,"a");
	if(!ldelay){
		printf("cannot open delay file %s\n", localdelayname);
		exit(1);
	}

	memset(monitoreddelayname,0,500);
	sprintf(monitoreddelayname, "%smonitoreddelays.txt",argv[1]);
	mdelay = fopen(monitoreddelayname,"a");
	if(!mdelay){
		printf("cannot open delay file %s\n", monitoreddelayname);
		exit(1);
	}

	memset(overheadname,0,500);
	sprintf(overheadname, "%soverhead.txt",argv[1]);
	overhead = fopen(overheadname,"a");
	if(!overhead){
		printf("cannot open overhead file %s\n", overheadname);
		exit(1);
	}

	for(i = 7; i < argc; i++){
		tmp = argv[i];
		index = tmp.find_last_of('/');
		memset(monitoredoutname,0,500);
		memset(localoutname,0,500);
		memset(c2stau,0,500);
		memset(s2ctau,0,500);
		memset(timeseq,0,500);

		tmp = tmp.substr(index+1);
		sprintf(monitoredoutname, "%s%s.size",argv[1],tmp.c_str());
		sprintf(timeseq, "%s%s.time",argv[1],tmp.c_str());
		sprintf(localoutname, "%s%s.local.txt",argv[1],tmp.c_str());
		sprintf(c2stau, "%sc2stau_%s.txt",argv[1],tmp.c_str());
		sprintf(s2ctau, "%ss2ctau_%s.txt",argv[1],tmp.c_str());
		ret = cfilter.parse_one(argv[i], proxy_port_min, proxy_port_max, if_remove_ack, monitoredoutname, localoutname, c2stau, s2ctau, timeseq);
		
		fprintf(ldelay, "%s: %lf\n", argv[i], ret.local_delay/1.0/1000000);
		fprintf(mdelay, "%s: %lf\n", argv[i], ret.monitored_delay/1.0/1000000);
		fprintf(overhead, "%s: %lu, %lu, %lf\n", argv[i], ret.monitored_bytes, ret.local_bytes, ret.monitored_bytes/1.0/ret.local_bytes);
	}
	fclose(ldelay);
	fclose(mdelay);
	fclose(overhead);
//	printf("%lf\n", delay/1.0/(argc-4));
//	sort(delays.begin(), delays.end());
//	printf("%lf\n", (double)delays.at(delays.size()/2)/1000000);
	return 0;
}

