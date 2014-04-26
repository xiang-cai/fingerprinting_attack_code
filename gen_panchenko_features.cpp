#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

#define STRATIFY 0

#define MaxM 30
#define REMOVE_ACK 1

#define USE_SIZEMARKER 1
#define USE_NUMMARKER 1

#define USE_HTMLMARKER 1

#define USE_TTN 1
#define USE_TTBYTES 1

#define USE_OCCURPKTS 1
#define USE_INCOMPERC 1

/*********
 * -1500,1499,..., 0 | 0,1,...,1500 |  -MaxM, ..., 0 | 0,1,...,MaxM |
 *  HTML | TOTB_IN | TOTB_OUT | -1,-2,-(3--5),-(6--8),-(9--13),-14,-(14+) | 1,2,3--5,6--8,9--13,14,14+ |
 *  # of occurring pkts IN | # of occurring pkts OUT | % of pkts IN | # of pkts IN | # of pkts OUT |
 *  
 *
 *  INDEX:
 *  1,..........1501 | 1502,...3002 | 3003 ... 3003+MaxM | 3004+MaxM ... 3004+2MaxM |
 *  3005+2MaxM | 3006+2MaxM | 3007+2MaxM | 3008+2MaxM ... 3014+2MaxM | 3015+2MaxM ... 3021+2MaxM |
 *  3022+2MaxM | 3023+2MaxM | 3024+2MaxM | 3025+2MaxM | 3026+2MaxM |
 *  **********/

class Weka{

public:
	vector<int> str;
	int** closest;

public:
	Weka();
	Weka(int websites, int trials);
	void Parse_data(char* fname, char* outname, int tag, int trial);
	void Addmarkers(int* pmarker, int* nmarker,int* inNpkts,int* outNpkts);
	void AddHTMLmarkers(int &html_marker);
	void Addoccurpkts(int &inNoccr, int &outNoccr, int &inPercent);
	void init(int websites, int trials);
};

Weka::Weka(int websites, int trials){
//	init(websites, trials);
}

Weka::Weka(){
}

// I've no idea why I wrote this init function... but just leave it there for now...
void Weka::init(int websites, int trials){
	
	FILE* fin = NULL;
	int dis;

	fin = fopen("./top100_closest.txt","r");
	if(!fin){
		printf("top100_closest.txt cannot open!\n");
		exit(1);
	}

	closest = (int**)malloc(websites*trials*sizeof(int*));
	for(int k = 0; k < websites*trials; k++)
		closest[k] = (int*) malloc (websites*sizeof(int));

	
	for(int k = 0; k < websites*trials; k++)
		for(int l = 0; l < websites; l++){
			dis = fscanf(fin, "%d", &dis);
			closest[k][l] = dis;
		}
	fclose(fin);
}

	
void Weka::AddHTMLmarkers(int &html_marker){
	int i;
	for(i = 0; i<str.size(); i++){
		if(str[i]>0)
			continue;
		else
			break;
	}
	for(;i<str.size();i++){
		if(str[i] < 0)
			continue;
		else
			break;
	}

	//i points to the first positive pkt (incoming pkt)
	if(i >= str.size()){
		html_marker = 0;
		return;
	}

	for(;i<str.size();i++){
		if(str[i] > 0){
			html_marker += str[i];
			continue;
		}
		else
			break;
	}

//	html_marker = 600*(int)ceil(html_marker/600.0);
	html_marker = (int)ceil(html_marker/600.0);
}



//this method should be called at last, because it reorder and resize the str[]
void Weka::Addoccurpkts(int &inNoccr, int &outNoccr, int &inPercent){
	vector<int>::iterator it;

	sort(str.begin(),str.end());
	it = unique(str.begin(),str.end());
	str.resize(it-str.begin());

	if(str.size() == 0)
		return;
	outNoccr = 0;
	for(int i=0;i<str.size();i++){
		if(str[i] <= 0){
			outNoccr++;
		}
		else
			break;
	}

	inNoccr = str.size() - outNoccr;
//	inPercent = 5*(int)ceil(inNoccr/1.0/outNoccr*100/5);
	if(str.empty())
		inPercent = 0;
	else
		inPercent = 5*(int)ceil(inNoccr/1.0/str.size()*100/5);

	//round to the increments of 2
	if(inNoccr %2 != 0)
		inNoccr++;
	if(outNoccr %2 != 0)
		outNoccr++;
}


void Weka::Addmarkers(int* pmarker, int* nmarker,int* inNpkts,int* outNpkts){
	int i,round;

	int markerB = 0;
	int markerN = 0;
	int flag = str[0] < 0 ? -1 : 1;
	for(i=0;i<str.size();i++){
		if(str[i] * flag > 0){
			markerB += str[i];
			markerN++;
		}
		else{
			//deal with size_marker
			round = markerB/abs(markerB)*ceil(abs(markerB)/600.0);
			if(round <= MaxM && round >= -MaxM){
				if(round < 0)
					nmarker[-round]++;
				else
					pmarker[round]++;
			}

			//deal with number_marker
			if(flag > 0){
				if(markerN == 1){
					inNpkts[0]++;
				}
				else if(markerN == 2){
					inNpkts[1]++;
				}
				else if(markerN >= 3 && markerN <= 5){
					inNpkts[2]++;
				}
				else if(markerN >= 6 && markerN <= 8){
					inNpkts[3]++;
				}
				else if(markerN >=9 && markerN <= 13){
					inNpkts[4]++;
				}
				else if(markerN == 14){
					inNpkts[5]++;
				}
				else{
					inNpkts[6]++;
				}
			}

			else{
				if(markerN == 1){
					outNpkts[0]++;
				}
				else if(markerN == 2){
					outNpkts[1]++;
				}
				else if(markerN >= 3 && markerN <= 5){
					outNpkts[2]++;
				}
				else if(markerN >= 6 && markerN <= 8){
					outNpkts[3]++;
				}
				else if(markerN >=9 && markerN <= 13){
					outNpkts[4]++;
				}
				else if(markerN == 14){
					outNpkts[5]++;
				}
				else{
					outNpkts[6]++;
				}
			}

			flag *= -1;
			markerB = str[i];
			markerN = 1;
		}
	}

	// the last chunk
	round = markerB/abs(markerB)*ceil(abs(markerB)/600.0);
	if(round <= MaxM && round >= -MaxM){
		if(round < 0)
			nmarker[-round]++;
		else
			pmarker[round]++;
	}

	//deal with number_marker
	if(flag > 0){
		if(markerN == 1){
			inNpkts[0]++;
		}
		else if(markerN == 2){
			inNpkts[1]++;
		}
		else if(markerN >= 3 && markerN <= 5){
			inNpkts[2]++;
		}
		else if(markerN >= 6 && markerN <= 8){
			inNpkts[3]++;
		}
		else if(markerN >=9 && markerN <= 13){
			inNpkts[4]++;
		}
		else if(markerN == 14){
			inNpkts[5]++;
		}
		else{
			inNpkts[6]++;
		}
	}
	else{
		if(markerN == 1){
			outNpkts[0]++;
		}
		else if(markerN == 2){
			outNpkts[1]++;
		}
		else if(markerN >= 3 && markerN <= 5){
			outNpkts[2]++;
		}
		else if(markerN >= 6 && markerN <= 8){
			outNpkts[3]++;
		}
		else if(markerN >=9 && markerN <= 13){
			outNpkts[4]++;
		}
		else if(markerN == 14){
			outNpkts[5]++;
		}
		else{
			outNpkts[6]++;
		}
	}
}

void Weka::Parse_data(char* fname, char* outname, int tag, int trial){
	FILE* fp = NULL;
	FILE* outfp = NULL;
	int i,ret,length,index;
	int inflow[1501];
	int outflow[1501];
	int pmarker[MaxM+1];
	int nmarker[MaxM+1];
	int inNpkts[7];	// 1,2,3--5,6--8,9--13,14,14+
	int outNpkts[7];// -1, ... -14+
	int html_marker; // increments of 600 bytes
	int inBtt;	// increments of 10000 bytes
	int outBtt;
	int inNtt;	// increments of 15
	int outNtt;
	int inNoccr;	// increments of 2
	int outNoccr;
	int inPercent;	// step of 5

	// initialize variables
	html_marker=inBtt=outBtt=inNtt=outNtt=inNoccr=outNoccr=inPercent=0;
	
	for(i=0;i<1501;i++)
		inflow[i]=outflow[i]=0;
	for(i=0;i<= MaxM;i++)
		pmarker[i] = nmarker[i] = 0;
	for(i=0;i<7;i++)
		inNpkts[i]=outNpkts[i]=0;


	fp = fopen(fname, "r+");
	if(!fp){
		cout<<fname<<"  cannot open!"<<endl;
		exit(1);
	}

	outfp = fopen(outname, "a+");
	if(!outfp){
		cout<<"  cannot open "<<outname<<endl;
		fclose(fp);
		exit(1);
	}

	str.clear();

	while(!feof(fp)){
		ret = fscanf(fp,"%d",&length);
		if(ret <= 0 || abs(length) > 1500)
			continue;
#if REMOVE_ACK 	//remove acks
//		if(abs(length) != 52 && abs(length) != 40)
		if(abs(length) > 84)
#endif
			str.push_back(length);
	}

	if(str.size() == 0)
		return;

	html_marker=inBtt=outBtt=inNtt=outNtt=inNoccr=outNoccr=inPercent=0;
	for(int i = 0 ;i < str.size(); i++){

		if(str[i] >= 0){
			inflow[str[i]]++;
			inBtt += str[i];
			inNtt++;

		}
		else{
			outflow[0-str[i]]++;
			outBtt += str[i];
			outNtt++;
		}
	}

//	inBtt = 10000*(int)ceil(abs(inBtt)/10000.0);
//	outBtt = 10000*(int)ceil(abs(outBtt)/10000.0);
	inBtt = (int)ceil(abs(inBtt)/10000.0);
	outBtt = (int)ceil(abs(outBtt)/10000.0);

	inNtt = 15*(int)ceil(inNtt/15.0);
	outNtt = 15*(int)ceil(outNtt/15.0);

	Addmarkers(pmarker,nmarker,inNpkts,outNpkts);
	AddHTMLmarkers(html_marker);
	Addoccurpkts(inNoccr,outNoccr,inPercent);

	fprintf(outfp, "%d ", tag);
	for(i=1500,index=1;i>=0;index++,i--){
		if(outflow[i] != 0)
			fprintf(outfp,"%d:%d ", index,outflow[i]);
	}
	for(i=0,index=1502;i<=1500;index++,i++){
		if(inflow[i] != 0)
			fprintf(outfp,"%d:%d ", index,inflow[i]);
	}

	// add more features to svm vetor

#if USE_SIZEMARKER
	for(index=3003, i=MaxM; i>=0; index++,i--){
		if(nmarker[i] != 0)
			fprintf(outfp,"%d:%d ", index,nmarker[i]);
	}
	for(index=3004+MaxM, i=0; i<=MaxM; index++, i++){
		if(pmarker[i] != 0)
			fprintf(outfp,"%d:%d ", index, pmarker[i]);
	}

#endif
#if USE_HTMLMARKER
	if(html_marker != 0)
		fprintf(outfp,"%d:%d ", 3005+2*MaxM, html_marker);
#endif

#if USE_TTBYTES
	if(inBtt != 0){
		fprintf(outfp,"%d:%d ", 3006+2*MaxM, inBtt);
	}
	if(outBtt != 0){
		fprintf(outfp,"%d:%d ", 3007+2*MaxM, outBtt);
	}
#endif

#if USE_NUMMARKER
	for(index=3008+2*MaxM, i=0; i<7; index++,i++){
		if(outNpkts[i] != 0)
			fprintf(outfp,"%d:%d ", index,outNpkts[i]);
	}
	for(index=3015+2*MaxM, i=0; i<7; index++, i++){
		if(inNpkts[i] != 0)
			fprintf(outfp,"%d:%d ", index, inNpkts[i]);
	}
#endif

#if USE_OCCURPKTS
	if(inNoccr != 0)
		fprintf(outfp,"%d:%d ", 3022+2*MaxM, inNoccr);
	if(outNoccr != 0)
		fprintf(outfp,"%d:%d ", 3023+2*MaxM, outNoccr);
#endif


#if USE_INCOMPERC
	if(inPercent != 0)
		fprintf(outfp,"%d:%d ", 3024+2*MaxM, inPercent);
#endif

#if USE_TTN
	if(inNtt != 0)
		fprintf(outfp,"%d:%d ", 3025+2*MaxM, inNtt);
	if(outNtt != 0)
		fprintf(outfp,"%d:%d ", 3026+2*MaxM, outNtt);
#endif



/*
	for(i=0; i < 100; i++,index++){
		fprintf(outfp,"%d:%d ", index,closest[20*(tag-1) + trial - 1][i]);
	}
*/	
	fprintf(outfp, "\n");
	fclose(fp);
	fclose(outfp);
}



int main(int argc, char** argv){
	if(argc != 4){
		cout<<"./SVM_all <logfolder> <websites> <trials>"<<endl;
		exit(1);
	}

	char outname[500];
	
	int websites=atoi(argv[2]);
	int trials= atoi(argv[3]);

		
	Weka Wclass; //(websites, trials);

	char name[500];
	const char* folder = argv[1];
/*
	for(int j = 21; j <= 40; j++){
		for(int i = 1; i <= websites; i++){
			memset(name, 0, 200);
			sprintf(name, "%s%d_%d.txt",folder, i,j);
			Wclass.Parse_data(name,outname,i,j);
		}
	}
*/
	int testfold;
	int perfold = trials/10;

	for(int web = 1; web <= websites; web++){
		for(int trial = 1; trial <= trials; trial++){
			memset(name,0,500);
			sprintf(name, "%s%d_%d.cap.txt",folder, web,trial);
#if STRATIFY
			testfold = (trial-1)/perfold+1;
			for(int fold = 1; fold <= 10; fold++){
				memset(outname, 0, 500);
				if(fold == testfold)
					sprintf(outname,"./svm_testing_%d",fold);
				else
					sprintf(outname,"./svm_training_%d",fold);
				Wclass.Parse_data(name,outname,web,trial);
			}
#else
			memset(outname, 0, 500);
			sprintf(outname,"./svm_fvector");
			Wclass.Parse_data(name,outname,web,trial);
#endif
		}
	}
		

	return 0;
}


