#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mmio.h"
#include <pthread.h>

int* colors;
int* temp_colors;
int* I;
int* csc;
int edges, thread_num = 4;
float step;

void* parallel_for(void* arg){
  int input =*(int*)arg;
  //Every Thread goes through a a "step" number of Nodes 
  //The total nodes is thread_num*step so when this function is run by all threads it went through all Nodes
  for(int i = (int)(input*step); i < (int)((input+1)*step);i++){
    if(colors[i]>-1){
      int start = csc[i];
      int end = csc[i+1];
      //For all Vetrices pointing to current Vetrice
      for(int j = start; j < end; j++){
        if(colors[i] > temp_colors[I[j]] && temp_colors[I[j]]>-1){
          colors[i] = temp_colors[I[j]];
        }
      }
    }
  }
  free(arg);
  return NULL;
}

void FindPredessecorTree(int root_index, int* csc, int* rows, int* colors,int* temp_colors, int root_color, int new_color){
  if(colors[root_index]==root_color){
    int start = csc[root_index];
    int end = csc[root_index+1];
    //Add Vertex to SCC by changing color to the SCC index that it belongs
    colors[root_index]=new_color;
    for(int i = start; i < end; i++){
      //If the Vetrice is being pointed by another Vetrice of the same colors, they belong in the SCC
      if(root_color == temp_colors[rows[i]]){
        //The predeccesor Vetrice that belongs to the tree repeats the procedure
        //until all Vetrices in the SCChave the same scc_tree_index
        FindPredessecorTree(rows[i], csc, rows, colors,temp_colors, root_color, new_color);

      }
    }
  }
}

void coloringSCC( int *rows, int *csc, int v){
  int scc_number = 0;
  int scc_tree_index = -1;//All Vetrices in the same SCC will have the same number
  int temp_counter;
  bool flag=true;
  pthread_t th[4];
  //colors initialize
        for(int i = 0; i < v; i++){
            colors[i] = i;
            temp_colors[i]=i;

        }
  while(flag){
    flag=false;

    //Check if a color changed
    bool flag_parallel=true;
    while(flag_parallel){
      flag_parallel = false;
      for(int i = 0; i < thread_num; i++){
        int* a = malloc(sizeof(int));
        *a = i;
        pthread_create(&th[i],NULL,&parallel_for,a);
      }
      for(int i = 0; i < thread_num; i++){
        pthread_join(th[i],NULL);
      }
      temp_counter =0;

      for(int i = 0; i < v; i++){
        if(colors[i]==i){
          temp_counter++;

        }
        if(colors[i]!=temp_colors[i]){
          temp_colors[i]=colors[i];
          flag_parallel = true;
        }
       }
    }

    int *unique_colors = (int*)malloc(sizeof(int)*temp_counter);
    temp_counter = 0;

    for(int i = 0; i < v; i++){
      if(colors[i]==i){
        unique_colors[temp_counter] = colors[i];
        temp_counter++;
      }
    }
    for(int i = 0; i < temp_counter; i++){
        FindPredessecorTree(unique_colors[i], csc, rows, colors,temp_colors, unique_colors[i], scc_tree_index);
        scc_tree_index--;
        scc_number++;
    }
    free(unique_colors);
  //colors initialize
  for(int i = 0; i < v; i++){
    if(colors[i]>-1){
      flag=true;
      colors[i] = i;
    }
    temp_colors[i]=colors[i];
    }
  }

  printf("%d\n",scc_number );
  free(colors);
  free(temp_colors);
}




int  main(int argc, char const *argv[]) {

  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int M, N, nz;
  int  *J;
  double *val;

  if (argc < 2)
{
  fprintf(stderr, "Usage: %s [martix-market-filename]\n", argv[0]);
  exit(1);
}
  else
  {
      if ((f = fopen(argv[1], "r")) == NULL)
          exit(1);
  }

  if (mm_read_banner(f, &matcode) != 0)
  {
      printf("Could not process Matrix Market banner.\n");
      exit(1);
  }


  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&
          mm_is_sparse(matcode) )
  {
      printf("Sorry, this application does not support ");
      printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
      exit(1);
  }

  /* find out size of sparse matrix .... */

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0)
      exit(1);


  /* reseve memory for matrices */

  I = (int *) malloc(nz * sizeof(int));
  J = (int *) malloc(nz * sizeof(int));
  val = (double *) malloc(nz * sizeof(double));


  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

  for (int i=0; i<nz; i++)
  {
      fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
      I[i]--;  /* adjust from 1-based to 0-based */
      J[i]--;
  }
  free(val);

  if (f !=stdin) fclose(f);
  mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);

  colors = (int*)calloc(N,sizeof(int));
  temp_colors = (int*)calloc(N,sizeof(int));

  csc = (int*)calloc(N+1,sizeof(int));

  step = N/(float)thread_num;

  int j = 0;
  for(int i = 0; i < N; i++){
    csc[i] = j;
    while(J[j]==i){
      j++;
    }
  }
  csc[N]=j;
  free(J);


  coloringSCC(I,csc,N);
  free(I);
  free(csc);
  return 0;
}
