#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#define MAXWEB 100
#define TRIALS 40
#define OLD_NEW 0

int is_removed(int cur_trial, int trials, int n_removed, int start){
	for(int i  = 0; i < n_removed; i++){
		if(cur_trial == ((start-1+i)%trials)+1)
			return 1;
	}
	return 0;
}


void parse(int websites, int trials, int trainsize, char* fmatrix, int gen_testing){

	int label,id;
	int dim = websites*trials;
	int buffersize = MAXWEB*TRIALS;
	int instances = trials/10;
	double cdis;
	FILE* fin;
	FILE** fout_train, **fout_test;
	char outname[100];
	double* tmp = (double*)malloc(buffersize*sizeof(double));
	
	if((fin = fopen(fmatrix,"r")) == NULL){
		cout<<"cannot open file "<<fmatrix<<endl;
		exit(1);
	}

#if OLD_NEW
	FILE* f_old,*f_new;
	if((f_old = fopen("./oldbase","w")) == NULL){
		cout<<"cannot open oldbase "<<endl;
		exit(1);
	}
	if((f_new = fopen("./newtrain","w")) == NULL){
		fclose(f_old);
		cout<<"cannot open newtrain "<<endl;
		exit(1);
	}

	int oldindex = 1;
	int newindex = 1;
	for(int web = 1; web <= websites; web++){
		for(int trial = 1; trial <= trials; trial++){
			//read the line
			for(int i = 0; i < buffersize; i++)
				fscanf(fin, "%lf", &(tmp[i]));

			int k,step;
			if(trial <= 20){	//old_base
				fprintf(f_old,"%d 0:%d ", web, oldindex++);
				for(k = 1, step = 1; k <= dim;){
					if(step == 21){
						k += 20;
						step = 1;
						continue;
					}
					fprintf(f_old, "%d:%lf ",k,tmp[k-1]);
					step++;
					k++;
				}
				fprintf(f_old, "\n");
			}
			else{
				fprintf(f_new,"%d 0:%d ", web, newindex++);
				for(k = 1, step = 1; k <= dim;){
					if(step == 21){
						k += 20;
						step = 1;
						continue;
					}
					fprintf(f_new, "%d:%lf ",k,tmp[k-1]);
					step++;
					k++;
				}
				fprintf(f_new, "\n");
			}
		}
	}
	fclose(f_old);
	fclose(f_new);
#else

	fout_train = (FILE**)malloc(10*sizeof(FILE*));
	fout_test = (FILE**)malloc(10*sizeof(FILE*));

	for(int i = 0; i < 10; i++){
		if(gen_testing){
			memset(outname, 0, 100);
			sprintf(outname, "./gnorm_cus_testing_%d", i+1);
			if((fout_test[i] = fopen(outname,"a+")) == NULL){
				cout<<"cannot open file "<<outname<<endl;
				exit(1);
			}
		}
		memset(outname, 0, 100);
		sprintf(outname, "./gnorm_cus_training_%d", i+1);
		if((fout_train[i] = fopen(outname,"a+")) == NULL){
			cout<<"cannot open file "<<outname<<endl;
			exit(1);
		}
	}

	int testfold;
	int removed = trials-instances-trainsize;

	for(int web = 1; web <= websites; web++){
		for(int trial = 1; trial <= trials; trial++){
			//read the line
			for(int i = 0; i < buffersize; i++)
				fscanf(fin, "%lf", &(tmp[i]));

			testfold = (trial-1)/instances+1;
			for(int fold = 1; fold <= 10; fold++){
				if(fold == testfold){
					if(gen_testing){
						fprintf(fout_test[fold-1],"%d 0:%d ", web, (web-1)*trials+trial);
						for(int i = 1; i <= dim; i++){
							fprintf(fout_test[fold-1], "%d:%lf ",i,tmp[i-1]);
						}
						fprintf(fout_test[fold-1], "\n");
					}
				}
				else{
					if(1 == is_removed(trial, trials, removed, fold*instances+1))
						continue;
					fprintf(fout_train[fold-1],"%d 0:%d ", web, (web-1)*trials+trial);
					for(int i = 1; i <= dim; i++){
						fprintf(fout_train[fold-1], "%d:%lf ",i,tmp[i-1]);
					}
					fprintf(fout_train[fold-1], "\n");
				}
			}
		}
	}

	for(int i = 0; i < 10; i++){
		if(gen_testing){
			fclose(fout_test[i]);
		}
		fclose(fout_train[i]);
	}
	free(fout_test);
	free(fout_train);
#endif
	fclose(fin);
	free(tmp);
}


int main(int argc, char** argv){
	if(argc != 5){
		cout<<"usage: ./gen_stratify <websites> <trainsize> <matrix> <gen_testing>"<<endl;
		exit(1);
	}
	int websites = atoi(argv[1]); //800;
	int trials = TRIALS;
	int trainsize = atoi(argv[2]);
	char* matrix = argv[3];
	int gen_testing = atoi(argv[4]);
	parse(websites, trials, trainsize, matrix, gen_testing);
	return 0;
}


