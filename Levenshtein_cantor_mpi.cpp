#include <mpi.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <time.h>

#include <iostream>
using namespace std;

#define CORES 6

#define REMOVE_ACK 1
#define SHUFFLE 0

#define METHOD 2
#define DIFF_TOR_SIZE 1
#define INCREMENT_OF_600 2
#define ADD_CHUNK_MARKER 3
#define PM_ONE 4
#define ALL_ONE 5
#define SAMPLE_MORPHING 6
#define PINGPONG 7

#define UNIT 512
#define INCREMENT 600

typedef struct _cord{
	int x;
	int y;
}CORD;

typedef int TRACE;

class Levenshtein{

public:
	vector<int> buffer;
	vector<int> sizes;
	vector<TRACE*> pool;
	vector <int> torsize;
	TRACE* str1;
	TRACE* str2;
	int websites;
	int trials;
	int ndistr[1501];
	int pdistr[1501];

public:
	Levenshtein(double aug);
	~Levenshtein();
	CORD inverse_cantor(int z);

	void get_distr(char* fname);
	void pingpong(vector<int>& trace, FILE* fp);
	void morph(vector<int>& trace, FILE* fp);
	int get_sprime(int length);
	int augment(vector<int>& trace, double aug);
	
	int Parse_data(char* fname, int type, double aug);
	TRACE* fetch_pool(int web, int trial);
//	void Pre_process();
	bool is_tor_size(int length);
	int read_torsize(const char* fname);
	double DLdis(int ms, int ns);
	double minimum(double a, double b, double c);
};


Levenshtein :: Levenshtein(double aug){
	torsize.clear();
	pool.clear();
	buffer.clear();
	sizes.clear();
	str1 = str2 = NULL;
	

	char fname[200];
	websites = 100;
	trials = 40;

	if(METHOD == SAMPLE_MORPHING)
		get_distr("./log_ssh/23_1.txt");
	
	const char* folder = "./traces/";	//this folder holds all the trace files
	for(int web = 1; web <= websites; web++){
		for(int trial = 1; trial <= trials; trial++){
			memset(fname, 0, 200);
			sprintf(fname,"%s%d_%d.txt", folder,web,trial);
			Parse_data(fname, METHOD, aug);
		}
	}
	cout<<"constructor finished, size of pool is: "<<pool.size()<<endl;
}

Levenshtein :: ~Levenshtein(){
	for(int i = 0; i < pool.size();i++)
		delete[] pool.at(i);
}

void Levenshtein::get_distr(char* fname){
	int length;
	FILE* fp;
	if((fp = fopen(fname, "r")) == NULL){
		printf("%s cannot be opened!\n", fname);
		exit(1);
	}
	for(int i = 0; i < 1501; i++)
		pdistr[i] = ndistr[i] = 0;

	while(!feof(fp)){
		if(0 > fscanf(fp, "%d", &length))
			continue;
		if(abs(length) > 1500)
			continue;

#if REMOVE_ACK
		if(abs(length) <= 84)
			continue;
#endif
		if(length < 0)
			ndistr[abs(length)]++;
		else
			pdistr[length]++;
	}
	fclose(fp);
	
	for(int i = 1; i < 1501; i++){
		pdistr[i] += pdistr[i-1];
		ndistr[i] += ndistr[i-1];
	}
}


void Levenshtein::pingpong(vector<int>& trace, FILE* fp){
	int preflag, curflag, count, length;
	count = preflag = curflag = 0;
	while(!feof(fp)){
		if(0 > fscanf(fp, "%d", &length))
			continue;
		if(abs(length) > 1500)
			continue;
		count++;
		curflag = length < 0 ? -1 : 1;
		if(curflag * preflag > 0){
			count++;
			trace.push_back(curflag*-1500);
		}
		trace.push_back(curflag*1500);
		preflag = curflag;
	}
}


int Levenshtein::get_sprime(int length){
	int max, picked, i;
	if(length >= 0){
		max = pdistr[1500];
		picked = rand()%max+1;
		for(i = 0; i < 1501 && pdistr[i] < picked; i++);
		picked = i;
		return picked;
	}
	else{
		max = ndistr[1500];
		picked = rand()%max+1;
		for(i = 1; i < 1501 && ndistr[i] < picked; i++);
		picked = -i;
		return picked;
	}
}


void Levenshtein::morph(vector<int>& trace, FILE* fp){
	int picked,flag,length;
	srand(time(NULL));
	while(!feof(fp)){
		if(0 > fscanf(fp, "%d", &length))
			continue;
		if(abs(length) > 1500)
			continue;
#if REMOVE_ACK
		if(abs(length) <= 84)
			continue;
#endif
		flag = length >= 0 ? 1 : -1;
		while(length != 0){
			picked = get_sprime(length);
			trace.push_back(picked/abs(picked));
//			trace.push_back(picked);
			if(abs(picked) >= abs(length))
				length = 0;
			else
				length = flag*(abs(length)-abs(picked)+84); 
		}
	}
}

int Levenshtein::augment(vector<int>& trace, double aug){
	int tsize = trace.size();
	int size = (int)ceil(tsize*aug);
	int extrasize = size-tsize;
	int x,y,i,j;
	vector<int> extra;
	vector<int> copy;
	
	srand(time(NULL));
	if(extrasize == 0)
		return tsize;

	for(i = 0; i < tsize; i++)
		copy.push_back(trace.at(i));

	for(i = 0 ; i < extrasize; i++){
		x = rand()%2;
		extra.push_back(x == 0 ? -1 : 1);
	}

	trace.clear();
	// begin to merge copy and extra to trace

	for(i = j = 0; i < tsize && j < extrasize;){
			y = rand()%size;
			if(y < tsize)
				trace.push_back(copy.at(i++));
			else
				trace.push_back(extra.at(j++));
	}
	while(i < tsize)
		trace.push_back(copy.at(i++));
	while(j < extrasize)
		trace.push_back(extra.at(j++));

	return size;
}


TRACE* Levenshtein::fetch_pool(int web, int trial){
	int index = (web-1)*trials+trial-1;
	if(index >= pool.size()){
		cout<<"fetching something out of pool, error! -- web "<<web<<", trial "<<trial<<endl;
		exit(1);
	}
	return pool.at(index);
}

/*
void Levenshtein::Pre_process(){
// set data to +/-1
	int i;

	for(i = 1; i < str1.size(); i++)
		str1[i] = str1[i] < 0 ? -1 : 1;
	
	for(i = 1; i < str2.size(); i++)
		str2[i] = str2[i] < 0 ? -1 : 1;
}

*/

bool Levenshtein :: is_tor_size(int length){
	int size = abs(length);
	for(int i = 0; i < torsize.size(); i++){
		if(torsize[i] == size)
			return true;
	}
	return false;
}

int Levenshtein::read_torsize(const char* fname){
	//read tor nodes' ip from a file fname, put them into vector tornodes 
	FILE* fp = NULL;
	int length,ret;

	fp = fopen(fname,"r");
	if(!fp){
		printf("cannot load file %s\n",fname);
		exit(1);
	}
	
	torsize.clear();
	while(!feof(fp)){
		ret = fscanf(fp,"%d",&length);
		if(ret > 0)
			torsize.push_back(length);
	}
	fclose(fp);
/*
	cout << "size: "<<tornodes.size()<<endl;
	for(int i = 0; i < tornodes.size(); i++)
		cout << tornodes.at(i)<<endl;
*/
	return 0;
}


int Levenshtein::Parse_data(char *fname, int type, double aug){
	FILE* fp = NULL;
	int length,size;
	int i, round;
	const char* torsize_fname = "./torsize.txt";
	
	buffer.clear();
	buffer.push_back(0);

	fp = fopen(fname, "r");
	if(!fp){
		cout<<fname<<"  cannot open!"<< errno <<endl;
		return -1;
	}
	
	switch(type){
		case SAMPLE_MORPHING:
			morph(buffer, fp);
			break;

		case PINGPONG:
			pingpong(buffer,fp);
			break;
		
		case PM_ONE :
			while(!feof(fp)){
				if(0 > fscanf(fp,"%d",&length))
					continue;
				if(abs(length) > 1500)
					continue;
			#if REMOVE_ACK
				if(abs(length) == 52 || abs(length) == 40)
			//	if(abs(length) <= 84)
					continue;
			#endif

				length = length > 0 ? 1 : -1; 
				buffer.push_back(length);
			}	
			break;

		case ALL_ONE :
			while(!feof(fp)){
				if(0 > fscanf(fp,"%d",&length))
					continue;
				if(abs(length) > 1500)
					continue;
				buffer.push_back(1);
			}	
			break;

		case INCREMENT_OF_600 :
			while(!feof(fp)){
				if(0 > fscanf(fp,"%d",&length))
					continue;
				if(abs(length) > 1500)
					continue;
			#if REMOVE_ACK
			//	if(abs(length) == 52 || abs(length) == 40)
				if(abs(length) <= 84)
					continue;
			#endif
				length = length/abs(length) * INCREMENT * (int)ceil(abs(length)/1.0/INCREMENT);
				buffer.push_back(length);
			}	
			break;
		
				
		default:
			break;
	}
	
	fclose(fp);

#if SHUFFLE
	random_shuffle(buffer.begin(),buffer.end());
#endif

//	size = augment(buffer, aug);
	TRACE* tmp = new TRACE[buffer.size()];
	for(int x = 0; x < buffer.size(); x++)
		tmp[x] = buffer.at(x);
	pool.push_back(tmp);
	sizes.push_back(buffer.size());
	return 0;
}

double Levenshtein::DLdis(int ms, int ns){
	double ret = 0;
	int min;
//	Pre_process();

	int m = ms;
	int n = ns;
	min = m < n ? m : n;
	min = min == 0 ? 1 : min;

	if(METHOD == PINGPONG || METHOD == ALL_ONE)
		return (double)abs(m-n)/min;

	int i,j;
    double subcost,transcost;
	double idcost = 2;

	double** dis = new double*[m];
	for(i=0;i<m;i++)
		dis[i]= new double[n];

	for(i = 0; i < m ;i++)
		for(j = 0; j < n; j++)
			dis[i][j] = -1;

	for(i=0; i<m; i++)
		dis[i][0]=i*idcost;
	for(j=0; j<n; j++)
		dis[0][j]=j*idcost;
	for(i=1; i<m; i++){
		for(j=1; j<n; j++){
			if(str1[i] == str2[j])
				subcost = transcost = 0;
			else{ 
				subcost = 2; //abs(str1[i]-str2[j]);
				transcost = 0.1;
			}
			dis[i][j] = minimum
			(
				dis[i-1][j] + idcost, //abs(str1[i]),  // a deletion
                dis[i][j-1] + idcost, //abs(str2[j]),  // an insertion
                dis[i-1][j-1] + subcost // a substitution
            );

			if(i>1 && j>1 && str1[i] == str2[j-1] && str1[i-1] == str2[j])
				dis[i][j] = dis[i][j] < dis[i-2][j-2]+transcost ? dis[i][j] : dis[i-2][j-2]+transcost;
		}
	}
	ret = dis[m-1][n-1]/min;
	
	//free dis
	for(i = 0 ; i < m; i++)
		delete[] dis[i];
	delete[] dis;

	return ret;
}


double Levenshtein::minimum(double a, double b, double c){
	double min = a;
	if(b < min)
		min = b;
	if(c < min)
		min = c;

	return min;
}

CORD Levenshtein::inverse_cantor(int z){
	CORD ret;
	int w = floor((sqrt(8.0*z+1)-1)/2);
	int t = (w*w+w)/2;
	ret.y = z-t;
	ret.x = w-ret.y;

	return ret;
}

int main(int argc, char** argv){
//	if(argc != 2){
//	    cout <<"argc != 2"<<endl;
//	    exit(1);
//	}

	FILE *fp = NULL;
	double dist = -1;
	CORD ret;
	int i,j,begin,end,round,head;
	char outname[100];
	double aug = atof(argv[1]);

	struct timeval tv;
    struct timezone tz;
	
	Levenshtein Lclass(aug);
// parameters need to be modified 
	const char* prefix = "./cantor_SSH_100_40_";
	int dim = Lclass.websites*Lclass.trials;
	int total = (dim*dim-dim)/2/CORES; // each node's task

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &round);

	cout<<"myrank is "<<round<<endl;


	memset(outname, 0, 100);
	sprintf(outname,"%snode%d", prefix, round);

	fp = fopen(outname, "a+");
	if(!fp){
		cout<<"cannot open file "<<outname<<" !"<<endl;
		exit(1);
	}
/*
	begin = 10*(round-1)+round;
	end = (round == 10) ? websites : round*11;
*/

	begin = round*total;	// round == 0,1,2, ... ,5
	end = begin+total-1;

	int web_x,trial_x,web_y,trial_y;
	int ms,ns;
	long long tstart,tend;
//	gettimeofday(&tv,&tz);
//	tstart = tv.tv_sec*1000000ULL+tv.tv_usec;

	for(int index = begin; index <= end; index++){
		ret = Lclass.inverse_cantor(index);
		web_x = (ret.x/Lclass.trials) + 1;
		trial_x = (ret.x%Lclass.trials) + 1;

		web_y = ((dim-1-ret.y)/Lclass.trials) + 1;
		trial_y = ((dim-1-ret.y)%Lclass.trials) + 1;
		
		Lclass.str1 = Lclass.fetch_pool(web_x,trial_x);
		Lclass.str2 = Lclass.fetch_pool(web_y,trial_y);
		
		ms = Lclass.sizes.at((web_x-1)*Lclass.trials+trial_x-1);
		ns = Lclass.sizes.at((web_y-1)*Lclass.trials+trial_y-1);
		dist = Lclass.DLdis(ms, ns);
		
		if (dist != -1){
			fprintf(fp,"%d;%d;%d;%d;%d;%d;%lf\n", web_x, trial_x, web_y, trial_y, ms, ns, dist);
			fprintf(fp,"%d;%d;%d;%d;%d;%d;%lf\n", web_y, trial_y, web_x, trial_x, ns, ms, dist);
		}
	}
	
	fclose(fp);
//	gettimeofday(&tv,&tz);
//	tend = tv.tv_sec*1000000ULL+tv.tv_usec;
	cout << "round "<<round<<" end" <<endl;

//	cout<<"time to compute "<<end-begin+1<<" distances is: "<<tend-tstart<<" us"<<endl;
	MPI_Finalize();
	return 0;
}


