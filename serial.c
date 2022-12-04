#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mmio.h"



void FindPredessecorTree(int root_index, int* csc, int* rows, int* colors, int root_color, int new_color){
  int start = csc[root_index];
  int end = csc[root_index+1];
  for(int i = start; i < end; i++){
    //If the Vetrice is being pointed by another Vetrice of the same colors, they belong in the SCC
    if(root_color == colors[rows[i]]){
      //printf("%d\n",rows[i] );

      //Change the color here to appoint it to a certain scc index.
      //We change it here to avoid the recursive algorith from cyrcling
      colors[rows[i]] = new_color;
    //  printf("%d\n",rows[i] );
      //The predeccesor Vertex that belongs to the tree repeats the procedure
      //until all Vetrices in the SCChave the same scc_tree_index
      FindPredessecorTree(rows[i], csc, rows, colors, root_color, new_color);

    }
  }
  colors[root_index]=new_color;
}

void coloringSCC( int *rows, int *csc, int v){
  int scc_number = 0;
  int scc_tree_index = -1;//All Vetrices in the same SCC will have the same number
  int *colors=(int*)calloc(v,sizeof(int));
  bool flag=true;
  //colors initialize
        for(int i = 0; i < v; i++){
            colors[i] = i;

        }
  while(flag){
    flag=false;

    //Check if a color changed
    bool flag_parallel=true;
    while(flag_parallel){
      flag_parallel = false;
      for(int i = 0; i < v; i++){
        if(colors[i]>-1){
          int start = csc[i];
          int end = csc[i+1];
          //For all Vetrices pointing to current Vetrice
          for(int j = start; j < end; j++){
            if(colors[i] > colors[rows[j]] && colors[rows[j]]>-1){
              colors[i] = colors[rows[j]];
              flag_parallel = true;
            }
          }
        }
      }
    }
    for(int i = 0; i<v;i++){
       //printf("%d ", colors[i]);
     }
     printf("\n" );
    for(int i = 0; i < v; i++){
      if(colors[i]==i){
        FindPredessecorTree(i, csc, rows, colors, colors[i], scc_tree_index);
        scc_tree_index--;
        scc_number++;
      }
    }

  //colors initialize
  for(int i = 0; i < v; i++){
    if(colors[i]>-1){
      flag=true;
      colors[i] = i;
    }
    }
  }
  for(int i = 0; i < v; i++){
    //printf("%d\n",colors[i]);
  }
  printf("%d\n",scc_number );
}




int  main(int argc, char const *argv[]) {

  int ret_code;
  MM_typecode matcode;
  FILE *f;
  int M, N, nz;
  int *I, *J;
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

  printf("MPHKA\n" );

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
      fscanf(f, "%d %d %lg\n", &I[i], &J[i] &val[i]);
      I[i]--;  /* adjust from 1-based to 0-based */
      J[i]--;
  }


  if (f !=stdin) fclose(f);

  /************************/
  /* now write out matrix */
  /************************/

  mm_write_banner(stdout, matcode);
  mm_write_mtx_crd_size(stdout, M, N, nz);
  int *csr = (int*)calloc(N+1,sizeof(int));
  int *csc = (int*)calloc(N+1,sizeof(int));
  //for (int i=0; i<nz; i++)

      //fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);
printf("okay\n");
  int j = 0;
  for(int i = 0; i < N; i++){
    csc[i] = j;
    while(J[j]==i){
      j++;
    }
  }
  csc[N]=j;


  coloringSCC(I,csc,N);
  return 0;
}
