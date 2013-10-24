#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <cstring>
#include <vector>
using namespace std;


#define PARSE_SSH 1

class Filter{
	public:
		vector <string> tornodes;
		vector <string> src_ips;
	public:
		Filter();
		int read_tornodes(char* fname);
		bool is_tornode(char* ip_addr);
		bool is_src_ip(char* src_ip);
		bool is_sshtraffic(char* src, unsigned int sport, char* dst, unsigned int dport);
};


Filter::Filter(){
	src_ips.clear();
	src_ips.push_back("130.245.120.100"); // local machine's IP address
}

int Filter::read_tornodes(char* fname){
	//read tor nodes' ip from a file fname, put them into vector tornodes 
	FILE* fp = NULL;
	char ip[20];

	fp = fopen(fname,"r");
	if(!fp){
		printf("cannot load file %s\n",fname);
		exit(1);
	}
	
	tornodes.clear();
	while(!feof(fp)){
		fscanf(fp,"%s",ip);
		if(strcmp(ip,"") != 0)
			tornodes.push_back(ip);
		memset(ip,0,20);
	}
	fclose(fp);
/*
	cout << "size: "<<tornodes.size()<<endl;
	for(int i = 0; i < tornodes.size(); i++)
		cout << tornodes.at(i)<<endl;
*/
	return 0;
}


bool Filter::is_tornode(char* ip_addr){
	for(int i = 0; i < tornodes.size(); i++){
		if(tornodes[i].compare(ip_addr) == 0)
			return true;
	}
	return false;
}

bool Filter::is_src_ip(char* src_ip){
	for(int i = 0; i < src_ips.size(); i++){
		if(src_ips[i].compare(src_ip) == 0)
			return true;
	}
	return false;
}

bool Filter::is_sshtraffic(char* src, unsigned int sport, char* dst, unsigned int dport){
	if(is_tornode(src) && sport >= 30000 && sport <= 30002)
		return true;
	if(is_tornode(dst) && dport >= 30000 && dport <= 30002)
		return true;
	return false;
}

int main(int argc, char** argv){
	if(argc != 5){
		printf("argc != 5\n");
		printf("./capfilter capfname tornodesfname if_remove_ack output_fname\n");
		exit(1);
	}
	char* capfname = argv[1];
	char* tornodesfname = argv[2];
	int remove_ack = atoi(argv[3]);
	char* outname = argv[4];

	Filter cfilter;
	cfilter.read_tornodes(tornodesfname);

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
	if(handle == NULL){
		fprintf(eout, "cannot open file %s\n", capfname);
		fclose(eout);
		exit(1);
	}

	// processing packets from fname
	
	u_short ether_type;
	char src[20];
	char dst[20];
	struct ether_header* e_header;

	// open output file

	FILE* fout = NULL;
	fout = fopen(outname,"w");
	if(!fout){
		printf("cannot open output file %s\n",outname);
		exit(1);
	}

	while(packet = pcap_next(handle, &header)){
		pkt_ptr = (u_char*)packet;
		e_header = (struct ether_header*)packet;
		ether_type = ntohs(e_header->ether_type);

		if(ether_type != ETHERTYPE_IP){
//			printf("unknown ether_type\n");
			continue;
		}

		pkt_ptr += sizeof(struct ether_header);
		ip_header = (struct ip*)(pkt_ptr);
		pkt_ptr += sizeof(struct ip);
		tcp_header = (struct tcphdr*)(pkt_ptr);

		memset(src,0,20);
		memset(dst,0,20);
		strcpy(src, inet_ntoa(ip_header->ip_src));
		strcpy(dst, inet_ntoa(ip_header->ip_dst));
		
		pkt_length = ntohs(ip_header->ip_len);
		sport = ntohs(tcp_header->source);
		dport = ntohs(tcp_header->dest);
	
//		printf("src: %s -- dst: %s , len: %d, src_p: %u, dst_p: %u\n", src, dst, pkt_length, sport, dport);

#if PARSE_SSH
		if(!cfilter.is_sshtraffic(src, sport, dst, dport))
			continue;
#endif
		if(pkt_length > 1500)
			continue;
		if(remove_ack == 1 && (pkt_length == 52 || pkt_length == 40))
			continue;
		if(cfilter.is_tornode(src) || cfilter.is_tornode(dst)){
			if(cfilter.is_src_ip(src))
				pkt_length *= -1;
			fprintf(fout, "%d\n", pkt_length);
		}
	}

	fclose(fout);
	return 0;
}

