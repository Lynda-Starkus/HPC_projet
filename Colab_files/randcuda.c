#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int dim,i;
    FILE *fp;
    fp = fopen("cudamatrix.txt","w");
    printf("Enter dimension: ");
    char buf[10];
    scanf("%d",&dim);
    sprintf(buf,"%d",dim);
    fputs(buf,fp);
    memset(buf,'\0',sizeof(buf));
    putc('\n',fp);
    int mat[dim*(dim+1)];
    for(i=0;i<dim*(dim+1);i++)
    {
        mat[i] = rand()%40;
        while(mat[i]==0)
        {
            mat[i] = rand()%40;
        }
        sprintf(buf,"%d",mat[i]);
        fputs(buf,fp);
        memset(buf,'\0',sizeof(buf));
        putc(' ',fp);
        if(i%dim==(dim-1))
            putc('\n',fp);
    }
}