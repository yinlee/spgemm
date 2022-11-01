#include <stdio.h>

#include "pthread_launch.h"
#include "spgemm.h"
#include "spad.h"
#include "bind_defs.h"
#include "group_templates.h"
#include "util.h"

#include "data_structure.h"

typedef struct node
{
	// int row;
	int col;
	float val;
	struct node *next;
} Node;

void __attribute__((optimize("-fno-inline")))
// spmm_manycore(const float *A_val, const float *B_val, float *C_val, const int *A_idx, const int *A_ptr, int m, int k, int ptid, int num_cores)
spgemm_manycore(const float *A_val, const float *B_val, float *C_val, Entry *C_inter, const int *A_idx, const int *A_ptr, const int *B_idx, const int *B_prt, int m, int k, int ptid, int num_cores)
{

	// DTYPE *sp_c = (DTYPE *)getSpAddr(ptid, 0);

	// Node *intermediate_list = (Node*)malloc(sizeof(Node));
	// intermediate_list->col=-1;
	// intermediate_list->val=0.0f;
	// intermediate_list->next = NULL;

	//row-wise product SpGEMM
	// printf("ptid: %d\n",ptid);
	for (int a_i = ptid; a_i < m; a_i += num_cores)
	{

		if (A_ptr[a_i] != A_ptr[a_i + 1])
		{
			// printf("start row %d\n",a_i);


			Node *intermediate_list = (Node *)malloc(sizeof(Node));
			intermediate_list->col = -1;
			intermediate_list->val = 0.0f;
			intermediate_list->next = NULL;
			// Node * head= intermediate_list;
			int malloc_time = 0;
			for (int a_j = A_ptr[a_i]; a_j < A_ptr[a_i + 1]; a_j++)
			{
				// printf("start row %d\n",a_i);
				float a_val = A_val[a_j];
				int a_idx = A_idx[a_j];
				int b_i = a_idx;
				// #pragma GCC unroll(19)
				for (int b_j = B_prt[b_i]; b_j < B_prt[b_i + 1]; b_j++)
				{
					Node *head = intermediate_list;
					float b_val = B_val[b_j];
					int b_idx = B_idx[b_j];
					// sp_c[B_idx[b_j]]+= a_val*b_val
					float tmp_val = a_val * b_val;
					// printf()
					// while(1&&(malloc_time<600)){
					// if (a_i==2)
					// {
					// 	printf("idx:%d val:%f \n", b_idx, tmp_val);
					// }
					while (1)
					{
						// if (a_i==2)
						// {
						// 	printf(" cur idx %d", head->col);
						// }

						if (head->col == b_idx)
						{
							// if (a_i==2)
							// {
							// 	printf(" merged(),");
							// }
							head->val += tmp_val;
							break;
						}
						else if ((head->next == NULL) || (head->next->col > b_idx))
						{
							// printf("%d,insert,\n",++malloc_time);
							// if (a_i==2)
							// {
							// 	printf(" malloc(), ");
							// }
							++malloc_time;
							Node *insert_node = (Node *)malloc(sizeof(Node));
							insert_node->col = b_idx;
							insert_node->val = tmp_val;
							insert_node->next = head->next;
							head->next = insert_node;
							break;
						}
						else if (head->next->col <= b_idx)
						{
							
							// printf("move,%d %d %d\n",head->next->col,b_idx,head->next->col);
							// if (a_i==2)
							// {
							// 	printf(" move(), ");
							// }
							// if (a_i==2)
							// {
							// 	printf("cur idx %d , move", head->next->col);
							// }
							head = head->next;
							continue;
						}
						else
						{
							printf("ERROR\n");
							return;
						}
					}

				}
			}
			// printf("before allocate\n");
			// printf("%d,insert,\n",malloc_time);

			float *tmp_array = (float *)malloc(sizeof(float) * 5000);
			int *tmp_index = (int *)malloc(sizeof(int) * 5000);

			int tmp_length = 0;
			Node *tmp_head = intermediate_list;
			Node *tmp_head_2 = intermediate_list;
			// printf("start trans\n");

			// if(1){
			// 	printf("\n");
			// Node* print_node = intermediate_list;

			// while(print_node!=NULL){
			// 	printf("(%d,%f),", print_node->col,print_node->val);
			// 	print_node=print_node->next;
			// }
			// printf("\n");
			// }
			while (tmp_head->next != NULL)
			{
				if(tmp_head->next->col>m){
					printf("%d,%d,%f\n",a_i,tmp_head->next->col,tmp_head->next->val);
					exit(0);
				}
				// printf("after free\n");
				// tmp_array[tmp_length] = tmp_head->next->val;
				// tmp_index[tmp_length] = tmp_head->next->col;


				C_inter[a_i].sub_idx[tmp_length]=tmp_head->next->col;
				C_inter[a_i].sub_array[tmp_length]=tmp_head->next->val;

				// printf("%d,",tmp_index[tmp_length]);
				tmp_head_2 = tmp_head;
				tmp_head = tmp_head->next;
				tmp_length = tmp_length + 1;
				free(tmp_head_2);
				malloc_time--;
			}
			free(tmp_head);
			malloc_time--;
			// printf("%d,insert,\n",malloc_time);

			// printf("after free\n");


			// C_inter[a_i].sub_array = tmp_array;
			// C_inter[a_i].sub_idx = tmp_index;


			C_inter[a_i].length = tmp_length;
			if (tmp_length > k)
			{
				printf("error*********************************%d\n", tmp_length);
			}
		}

		// #pragma GCC unroll(19)

		// for(int c_idx=0; c_idx<k; c_idx++){
		//   FSTORE_NOACK(sp_c[c_idx], C_val + i*k + c_idx, 0);
		//   sp_c[c_idx] = 0;
		// }
	}
}
// {

//   DTYPE *sp_c = (DTYPE *)getSpAddr(ptid, 0);

//   //row-wise product SpMM
//   for(int i=ptid; i<m; i += num_cores){
//     for(int j=A_ptr[i]; j<A_ptr[i+1]; j++){
//       float a_val = A_val[j];
//       int a_idx = A_idx[j];
//       #pragma GCC unroll(19)
//       for(int c_idx=0; c_idx<k; c_idx ++){
//         float b_val = B_val[a_idx * k + c_idx];
//         sp_c[c_idx] += a_val * b_val;
//       }
//     }

//     #pragma GCC unroll(19)
//     for(int c_idx=0; c_idx<k; c_idx++){
//       FSTORE_NOACK(sp_c[c_idx], C_val + i*k + c_idx, 0);
//       sp_c[c_idx] = 0;
//     }
//   }
// }
