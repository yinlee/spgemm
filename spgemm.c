#include <stdlib.h>
#include <stdio.h>

#include "pthread_launch.h"
#include "spgemm.h"
#include "spad.h"
#include "bind_defs.h"
#include "group_templates.h"



// actual kernel
void kernel(
     const float *A_val, const float *B_val, float *C_val, Entry *C_inter, const int *A_idx, const int *A_ptr, const int *B_idx, const int *B_ptr,int *C_ptr, int* C_idx, const int* C_idx_orig,const int* C_ptr_orig,int m, int k, int ptid, int num_cores)
{

  // start recording all stats (all cores)
  // use the last thread, b/c this wakes up last?
  if (ptid == 0 )
  {
    // stats_on();
  }


  
  // need to wait after transpose!
  pthread_barrier_wait(&start_barrier);

  // if (cinfo.used == 0) return;


  // MOVE_STACK_ONTO_SCRATCHPAD();

  spgemm_manycore(A_val, B_val, C_val, C_inter, A_idx, A_ptr, B_idx, B_ptr, m, k, ptid, num_cores);

  pthread_barrier_wait(&start_barrier);
  if(ptid==0){
    int c_start=0;
    for(int i=0;i<m;i++){
        // printf("%d,\n",i);


      for(int j=0;j<C_inter[i].length;j++){


        // printf("%d,\n",i);
        C_val[c_start+j]=C_inter[i].sub_array[j];
        // C_inter[i].sub_array[j];

        C_idx[c_start+j]=C_inter[i].sub_idx[j];
        // if((c_start+j)<100){
        // printf("c_idx %d: %d, %d\n",i,C_idx[c_start+j],C_idx_orig[c_start+j]);

        // }
      }
      C_ptr[i]=c_start;
      c_start+=C_inter[i].length;
      // if((C_ptr[i]!=C_ptr_orig[i])){

      //   printf("c_ptr %d: %d, %d\n",i,C_ptr[i],C_ptr_orig[i]);
      // }
    }
    
  // for(int i=0;i<(1048576);i++){
  //     if((C_idx[i]!=C_idx_orig[i])){

  //       // printf("c_ptr %d: %d, %d\n",i,C_idx[i],C_idx_orig[i]);
  //       i++;
  //       printf("c_ptr %d: %d, %d\n",i,C_idx[i],C_idx_orig[i]);

  //       exit(0);
  //     }
  // }

  }



  // RECOVER_DRAM_STACK();
}

// helper functions
Kern_Args *construct_args(const float *A_val, const float *B_val, float *C_val, Entry *C_inter, const int *A_idx, const int *A_ptr, const int *B_idx, const int *B_ptr ,int *C_ptr, int *C_idx, const int* C_ptr_orig, const int* C_idx_orig, int m, int k, int ptid, int num_cores)
{

  Kern_Args *args = (Kern_Args *)malloc(sizeof(Kern_Args));

  args->A_val = A_val;
  args->B_val = B_val;
  args->C_val = C_val;
  args->A_idx = A_idx;
  args->A_ptr = A_ptr;
  args->B_idx = B_idx;
  args->B_ptr = B_ptr;
  args->C_idx = C_idx;
  args->C_ptr = C_ptr;
  args->C_idx_orig=C_idx_orig;
  args->C_ptr_orig=C_ptr_orig;

  args->C_inter =  C_inter;
  args->m = m;
  args->k = k;
  args->ptid = ptid;
  args->num_cores = num_cores;

  return args;
}

void *pthread_kernel(void *args)
{
  // guarentee one thread goes to each core, by preventing any threads
  // from finishing early

  pthread_barrier_wait(&start_barrier);

  // call the spmd kernel
  Kern_Args *a = (Kern_Args *)args;

  kernel(a->A_val, a->B_val, a->C_val,a->C_inter, a->A_idx, a->A_ptr, a->B_idx, a->B_ptr,a->C_ptr,a->C_idx, a->C_idx_orig,a->C_ptr_orig ,a->m, a->k, a->ptid, a->num_cores);

  // reset scratchpad config
  // SET_PREFETCH_MASK(0, 0, &start_barrier);

  // if (a->ptid == 0)
  // {
  //   stats_off();
  // }

  return NULL;
}
