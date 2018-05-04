	
/****************************
 * (fullmesh test of kmeans)
 * (huili@ruijie.com.cn)
 * (2018/05/03)
 ****************************/
	
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <errno.h>
#include <string.h>
	
double calc(double *,int,double *);
int reduce(double *,int,int);
	
#define K 2
	
double *sbuf;
double *rbuf;
double *points;
double *clusters;
double *newclusters;
	
void pr(double*,int,int);
void data_process(char*,double*,int,int);
	
int main(int argc, char **argv){
	
	if(argc!=4) 
	{printf("usage:\n");printf("[file][dim@file][num@file]\n");exit(-1);}
	
	int dim = atoi(argv[2]);
	int num = atoi(argv[3]);
	
	points = new double[num*(dim)];
	rbuf = new double[num*(dim)];
	clusters = new double[K*(dim-1)];
	newclusters = new double[K*(dim-1)];
	
	int i=0; int j=0; int iter = 0; int iter_max = 50;
	int numhosts=0;
	int rank = 0;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&numhosts);	

	sbuf = new double[num*dim/numhosts];

	if(rank == 0){

	data_process(argv[1],points,num,dim);
	printf("points\n");
	pr(points,num,dim);
	/*
	for(i=0;i<K;i++)
		for(j=0;j<dim-1;j++)
			clusters[i*(dim-1)+j] = points[i*dim+j];
	*/
	for(j=0;j<dim-1;j++)
		clusters[j] = points[j];
	for(j=0;j<dim-1;j++)
		clusters[dim-1+j] = points[2*dim+j];
	//for(j=0;j<dim-1;j++)
	//	clusters[2*(dim-1)+j] = points[300*dim+j];
	printf("clusters\n");
	pr(clusters,K,dim-1);
	}//if(rank == 0)

	MPI_Bcast(points,num*dim,MPI_DOUBLE,0,MPI_COMM_WORLD);

	for(iter = 0; iter<iter_max; iter++){
	
	MPI_Bcast(clusters,K*(dim-1),MPI_DOUBLE,0,MPI_COMM_WORLD);

	printf("rank:%d cpu done\n",rank);
	double d = 0.0;

	int chunk_size = num*dim/numhosts;
	int start_points = rank*(num*dim)/numhosts;
	int end_points = start_points + chunk_size;

	for(i=start_points;i<end_points;){
		double min_d = calc(clusters,dim-1,points+i);
		points[i+dim-1] = 1.0;
		int index = 0;
		if(rank == 1)printf("i:%d cluster:0 min_d:%lf index:0\n",i/dim,min_d); 
		for(j=1;j<K;j++){
			d = calc(clusters+j*(dim-1),dim-1,points+i);
			if(min_d > d){min_d=d;index=j;}
			if(rank == 1)printf("i:%d cluster:%d min_d:%lf index:%d\n",i/dim,j,min_d,index); 
		}//for	
		points[i+dim-1] = index+1;
		i += dim;
	}
	memcpy(sbuf,points+start_points,sizeof(double)*chunk_size);
	MPI_Allgather(sbuf,chunk_size,MPI_DOUBLE,rbuf,chunk_size,MPI_DOUBLE,MPI_COMM_WORLD);
	if(rank == 0)pr(rbuf,num,dim);
	
	int *count = new int[K];
	for(i=0;i<K;i++) count[i] = 0;
	memset(newclusters,0,sizeof(double)*K*(dim-1));

	for(i=0;i<num;i++){

	int cid = (int)rbuf[i*dim+dim-1];
	//if(rank == 0) printf("[i:%d cid:%d]\n",i,cid);
	count[cid-1]++;

	for(j=0;j<dim-1;j++){
	newclusters[(cid-1)*(dim-1)+j] += rbuf[i*dim+j];
	}//for(j=0;
	if(rank == 0) printf("[cid:%d] newclusters:[",cid); 
	for(j=0;j<dim-1;j++)
	if(rank == 0) printf("%lf ",newclusters[(cid-1)*(dim-1)+j]);
	if(rank == 0) printf("]\n");

	}//for(i=0;

	for(i=0;i<K;i++){
	//printf("count[%d]:%d\n",i,count[i]);
	count[i] = (count[i] == 0 ? 1: count[i]);
	}

	for(i=0;i<K;i++)
	for(j=0;j<dim-1;j++)
		newclusters[i*(dim-1)+j]/=count[i];

	if(rank == 0)pr(newclusters,K,dim-1);
	memcpy(clusters,newclusters,sizeof(double)*K*(dim-1));
	if(rank == 0)pr(clusters,K,dim-1);
	
	}//iter	
	MPI_Finalize();
	return 0;	
}//main

double calc(double *d1,int dim, double *d2){
	double cal = 0.0;
	int i;
	for(i=0; i<dim; i++)
		cal+=(d1[i]-d2[i])*(d1[i]-d2[i]);

	return sqrt(cal);
}//int

int reduce(double *rb, int num,int dim){
	int i = 0;
	for(i=0;i<num*dim;i++){
	sbuf[i] += rb[i];	
	}//reduce
}//int

void pr(double *a,int num, int dim){
	int i = 0; int j = 0;
	for(i = 0; i<num; i++){
		for(j = 0; j<dim-1; j++){
		printf("%lf ",a[i*dim+j]);
		}
		printf("%d \n",(int)(a[i*dim+dim-1]));
	}
}//void

void data_process(char *file, double *a, int n, int d){
	//ti si li hui
	FILE *fp = fopen(file, "r");
	char c[1];
	char str[128];
	char *cp = str;int i = 0;
	char *end;

	while(fread(c,sizeof(char),1,fp)>0){
		double val = 0;
		if(c[0]=='.'||(c[0]>='0'&&c[0]<='9'))
		{sprintf(cp,"%c",c[0]);cp++;}
		else{*cp='\0';val = strtod(str,&end);	
			if(errno == 22 && val!=0){	
			a[i] = val;
			i++;cp=str;if(i==(n*d)){break;}}}
	}//while
	fclose(fp);
}//void
