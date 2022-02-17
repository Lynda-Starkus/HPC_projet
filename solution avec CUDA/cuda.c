#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#define MAXBLOCKSIZE 512
int Size;
float *a, *b, *finalVec;
float *m;

FILE *fp;

void InitProblemOnce(char *filename);
void InitPerRun();
void ForwardSub();
void BackSub();

/*
 Calculation du multiplicateur 
 */
__global__ void multiplier(float *m_cuda, float *a_cuda, int Size, int t)
{   
	int idx = threadIdx.x + blockIdx.x * blockDim.x;
	if( idx>= Size-1-t) return;
	*(m_cuda+Size*(idx+t+1)+t) = *(a_cuda+Size*(idx+t+1)+t) / *(a_cuda+Size*t+t);
}

//Conversion de la matrice en une triangulaire supérieure

__global__ void upper(float *m_cuda, float *a_cuda, float *b_cuda,int Size, int j1, int t)
{
	int xidx = blockIdx.x * blockDim.x + threadIdx.x;
	int yidx = blockIdx.y * blockDim.y + threadIdx.y;

	if(xidx >= Size-1-t) return;
	if(yidx >= Size-t) return;	
	
	a_cuda[Size*(xidx+1+t)+(yidx+t)] -= m_cuda[Size*(xidx+1+t)+t] * a_cuda[Size*t+(yidx+t)];
	
	if(yidx == 0){
		b_cuda[xidx+1+t] -= m_cuda[Size*(xidx+1+t)+(yidx+t)] * b_cuda[t];
	}
}
void InitMat(float *ary, int nrow, int ncol);
void InitAry(float *ary, int ary_size);
void PrintMat(float *ary, int nrow, int ncolumn);
void PrintAry(float *ary, int ary_size);
void checkCUDAError(const char *msg);

unsigned int totalKernelTime = 0;

int main(int argc, char *argv[])
{
	if(argc<2){
		printf("Entrez le nom de fichier cudamatrix.txt \n");
		return 1;
	}
    InitProblemOnce(argv[1]);
    InitPerRun(); //Initialiser m à 0


    struct timeval time_start;
    gettimeofday(&time_start, NULL);	
    
    // Exécuter le kernel
    ForwardSub();
    
    struct timeval time_end;
    gettimeofday(&time_end, NULL);
    unsigned int time_total = (time_end.tv_sec * 1000000 + time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec);
    
    
        printf("La matrice M est : \n");
        PrintMat(m, Size, Size);

        printf("La matrice A est : \n");
        PrintMat(a, Size, Size);

        printf("Le vecteur B est : \n");
        PrintAry(b, Size);
    
    BackSub();
        printf("Le vecteur X est : \n");
        PrintAry(finalVec,Size);
    
    printf("\nTemps d'exécutions + transferts mémoires\t%f sec\n", time_total * 1e-6);
    printf("Temps pour les CUDA kernels\t%f sec\n",totalKernelTime * 1e-6);   
    free(m);
    free(a);
    free(b);
}
/*Initialiser les différentes matrices 
 */
void InitProblemOnce(char *filename)
{	
	fp = fopen(filename, "r");
	fscanf(fp, "%d", &Size);	
	a = (float *) malloc(Size * Size * sizeof(float));
	InitMat(a, Size, Size);
	b = (float *) malloc(Size * sizeof(float));
	
	InitAry(b, Size);		
	 m = (float *) malloc(Size * Size * sizeof(float));
}

void InitPerRun() 
{
	int i;
	for (i=0; i<Size*Size; i++)
			*(m+i) = 0.0;
}


 
void ForwardSub()
{
	int t;
    float *m_cuda,*a_cuda,*b_cuda;
	
	
	cudaMalloc((void **) &m_cuda, Size * Size * sizeof(float));
	 
	cudaMalloc((void **) &a_cuda, Size * Size * sizeof(float));
	
	cudaMalloc((void **) &b_cuda, Size * sizeof(float));	

	cudaMemcpy(m_cuda, m, Size * Size * sizeof(float),cudaMemcpyHostToDevice );
	cudaMemcpy(a_cuda, a, Size * Size * sizeof(float),cudaMemcpyHostToDevice );
	cudaMemcpy(b_cuda, b, Size * sizeof(float),cudaMemcpyHostToDevice );
	
	int block_size,grid_size;
	
	block_size = MAXBLOCKSIZE;
	grid_size = (Size/block_size) + (!(Size%block_size)? 0:1);

	dim3 dimBlock(block_size);
	dim3 dimGrid(grid_size);
	
	int blockSize2d, gridSize2d;
	blockSize2d = 4;
	gridSize2d = (Size/blockSize2d) + (!(Size%blockSize2d?0:1)); 
	
	dim3 dimBlockXY(blockSize2d,blockSize2d);
	dim3 dimGridXY(gridSize2d,gridSize2d);

    struct timeval time_start;
    gettimeofday(&time_start, NULL);
	for (t=0; t<(Size-1); t++) {
		multiplier<<<dimGrid,dimBlock>>>(m_cuda,a_cuda,Size,t);
		cudaThreadSynchronize();
		upper<<<dimGridXY,dimBlockXY>>>(m_cuda,a_cuda,b_cuda,Size,Size-t,t);
		cudaThreadSynchronize();
		checkCUDAError("Upper");
	}
	struct timeval time_end;
    gettimeofday(&time_end, NULL);
    totalKernelTime = (time_end.tv_sec * 1000000 + time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec);
	
	cudaMemcpy(m, m_cuda, Size * Size * sizeof(float),cudaMemcpyDeviceToHost );
	cudaMemcpy(a, a_cuda, Size * Size * sizeof(float),cudaMemcpyDeviceToHost );
	cudaMemcpy(b, b_cuda, Size * sizeof(float),cudaMemcpyDeviceToHost );
	cudaFree(m_cuda);
	cudaFree(a_cuda);
	cudaFree(b_cuda);
}

void BackSub()
{
	// Un nouveau vecteur pour la solution finale
	finalVec = (float *) malloc(Size * sizeof(float));
	// Résolution
	int i,j;
	for(i=0;i<Size;i++){
		finalVec[Size-i-1]=b[Size-i-1];
		for(j=0;j<i;j++)
		{
			finalVec[Size-i-1]-=*(a+Size*(Size-i-1)+(Size-j-1)) * finalVec[Size-j-1];
		}
		finalVec[Size-i-1]=finalVec[Size-i-1]/ *(a+Size*(Size-i-1)+(Size-i-1));
	}
}

void InitMat(float *ary, int nrow, int ncol)
{
	int i, j;
	printf("Matrice A est : \n");
	for (i=0; i<nrow; i++) {
		for (j=0; j<ncol; j++) {
			fscanf(fp, "%f",  ary+Size*i+j);
			printf("%f ",*(ary+Size*i+j));
		}
		printf("\n");
	}  
}

void PrintMat(float *ary, int nrow, int ncol)
{
	int i, j;
	
	for (i=0; i<nrow; i++) {
		for (j=0; j<ncol; j++) {
			printf("%f ", *(ary+Size*i+j));
		}
		printf("\n");
	}
	printf("\n");
}

void InitAry(float *ary, int ary_size)
{
	int i;
	printf("\n Vecteur b est \n");
	for (i=0; i<ary_size; i++) {
		fscanf(fp, "%f",  &ary[i]);
		printf("%f ",ary[i]);
	}
}  

void PrintAry(float *ary, int ary_size)
{
	int i;
	for (i=0; i<ary_size; i++) {
		printf("%.2f ", ary[i]);
	}
	printf("\n\n");
}
void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) 
    {
        fprintf(stderr, "Erreur Cuda: %s: %s.\n", msg, 
                                  cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }                         
}