#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WEBSITES 100
#define TRIALS 40
#define DO_NORMALIZE 0
#define MIN 1
#define SUM 2


int get_index(int web, int trial){
	return (web-1)*TRIALS+trial -1;
}

inline int min(int a, int b){
	return a < b ? a : b;
}

int parse(char* fname, double aug){
	FILE* fp;
	int count,length;
	if((fp = fopen(fname, "r")) == NULL){
		printf("%s cannot be opened!\n", fname);
		exit(1);
	}

	count = 0;
	while(!feof(fp)){
		if(0 > fscanf(fp, "%d", &length))
			continue;
		if(abs(length) == 52 || abs(length) == 40)
			continue;
		count++;
	}
	
	fclose(fp);
	return count == 0 ? 1 : (int)ceil(count*aug);
}

void gen_gamma_matrix(int power, char* folder, char* fmatrix, char* gamma_matrix, int method, double aug){
	int index, i, j, web, trial;
	int dim = WEBSITES*TRIALS;
	int sizes[WEBSITES*TRIALS] = {0};
	FILE* fin, *fout;
	char fname[200];
	double dis;
	double gamma = pow(2, power);

#if DO_NORMALIZE
	//fill sizes[]
	for(web = 1; web <= WEBSITES; web++){
		for(trial = 1; trial <= TRIALS; trial++){
			index = get_index(web, trial);
			memset(fname, 0 , 200);
			sprintf(fname, "%s%d_%d.txt",folder, web, trial);
			sizes[index] = parse(fname, aug);
		}
	}
#endif

	//write to gamma matrix
	if((fin = fopen(fmatrix, "r")) == NULL){
		printf("%s cannot be opened!\n", fmatrix);
		exit(1);
	}

	if((fout = fopen(gamma_matrix, "w")) == NULL){
		printf("%s cannot be opened!\n", gamma_matrix);
		fclose(fin);
		exit(1);
	}

	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			fscanf(fin, "%lf", &dis);
		//	dis = dis/(sizes[i]+sizes[j]);
#if DO_NORMALIZE
			//normalize matrix
			switch(method){
				case MIN:
					dis = dis/min(sizes[i],sizes[j]);
					break;
				case SUM:
					break;
				default:
					break;
			}
#endif
			//apply gamma
			dis = exp(-gamma*dis*dis);
			fprintf(fout, "%E ", dis);
		}
		fprintf(fout, "\n");
	}

	fclose(fout);
	fclose(fin);
}


int main(int argc, char** argv){
	if(argc != 6){
		printf("./gen_gamma_matrix <pow> <folder> <input_matrix> <output_matrix> <aug>\n");
		exit(1);
	}
	double aug = atof(argv[5]);
	gen_gamma_matrix(atoi(argv[1]),argv[2],argv[3], argv[4], MIN, aug);
	return 0;
}








