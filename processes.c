/*
* Created by Oscar Avellan
*/

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "processes.h"
#include "list.h"


/******************************************************************************/

void change_process_time(List pr_running){
  Process pr;

  if(pr_running->data){
    pr = (Process)((Data)((List)pr_running->data)->data)->process;
    pr->time_remaining++;
    pr->job_time--;
  }
}

/******************************************************************************/

bool add_process_rrq(List *rrq,List temp){

  if(!(*rrq)){
  	*rrq = push(*rrq,temp);
  }

  else if(!(*rrq)->data){
    insert(temp,rrq);
    Data d_rem = pop(rrq);

    free(d_rem);
    return true;
  }
  else{
    List ptr = find(&ptr_eq1,temp,*rrq);

    if(!ptr){
      insert(temp,rrq);
      return true;
    }
  }

    return false;
}

/******************************************************************************/

int mem_usage(List *memory){

  double mem_free = 0;
  double mem_used  = 0;

  while(*memory){
    if( ((Data)(*memory)->data)->process){
      mem_used += ((Process)((Data)(*memory)->data)->process)->memory_size;
    }
    else if(((Data)(*memory)->data)->mem_avail){
      mem_free += *((int*)((Data)(*memory)->data)->mem_avail);
    }
    memory = &(*memory)->next;
  }

  return ceil((100 * mem_used)/(mem_used+mem_free));

}

/******************************************************************************/

void checking_process_time(int *longest_time, int *index, int *process_id, Process p, int i, int *process_time){

  if(*process_time > *longest_time ){
    *longest_time = *process_time;
    *index = i;
    *process_id = p->process_id;
  }
  else if(*process_time == *longest_time && p->process_id < *process_id){
    *index = i;
    *process_id = p->process_id;
  }

}

/******************************************************************************/

void setting_process_to_run(List *rrq, List *pr_running){

  /* Removing an element from the RRQ */
  if(len(*rrq) == 1 && (*rrq)->data){
    (*pr_running)->data = (*rrq)->data;
    (*rrq)->data = NULL;
  }
  else if (len(*rrq) > 1){
    if( (*rrq)->data ){
      (*pr_running)->data = pop(rrq);
    }else{
      Data d_rem = pop(rrq);
      (*pr_running)->data = pop(rrq);

      free(d_rem);
    }
  }
  else{
    (*pr_running)->data = NULL;
  }

  /* Setting the quantum in the process */
  if((*pr_running)->data){
    Process p = ((Data)((List)(*pr_running)->data)->data)->process;
    p->time_remaining = 0;
  }
}

/******************************************************************************/

List longest_in_memory(List *memory, int clock){
  int longest_time = 0, time_in_mem;
  Process p_temp;
  List temp_rrq;

  while((*memory)){
    p_temp = ((Data)(*memory)->data)->process;

    if(p_temp){
      time_in_mem = clock - p_temp->swapped_in_time;

      if(time_in_mem > longest_time){
        temp_rrq = *memory;
        longest_time = time_in_mem;
      }
    }
    memory = &(*memory)->next;
  }

  return temp_rrq;
}

/******************************************************************************/

void *print_rrq(void *acc, void *data){
  /* Just because I'm getting warnings */
  acc = NULL;

  if(data){
    Data d = (Data)((List)data)->data;
    Process p = ((Process)d->process);

    printf("Time created = %d ",p->time_created);
    printf("Process ID = %d ",p->process_id);
    printf("Memory Size = %d ",p->memory_size);
    printf("Job Time = %d \n",p->job_time);
  }else{
    printf("NULL\n");
  }
  return NULL;
}

/******************************************************************************/

void *print_free_list(void *acc, void *data){
  /* Just because I'm getting warnings */
  acc = NULL;

  Data d = (Data)(((List)data)->data);
  printf("free_list : %d\n",*((int*)d->mem_avail));
  return NULL;
}

/******************************************************************************/

void *print_nodes_information(void *acc, void *data){
  /* Just because I'm getting warnings */
  acc = NULL;

  Data d = (Data)data;
  if(d->process){
    printf("Time created = %d ",((Process)d->process)->time_created);
    printf("Process ID = %d ",((Process)d->process)->process_id);
    printf("Memory Size = %d ",((Process)d->process)->memory_size);
    printf("Job Time = %d \n",((Process)d->process)->job_time);
  }
  else{
    printf("memory = %d\n", *((Data)data)->mem_avail);
  }

  return NULL;
}

/******************************************************************************/

void delete_from_rrq(List temp_rrq, List *rrq){
  List t = find(&ptr_eq1,temp_rrq,*rrq);

  if(t){
    Data d_dummy = del(&ptr_eq1,temp_rrq,rrq);
    assert(d_dummy);
  }
}

/******************************************************************************/

void delete_from_p_running(List *p_run, List temp_rrq){

  if((*p_run)->data == temp_rrq){
      (*p_run)->data = NULL;
    }
}

/******************************************************************************/

bool ptr_eq1(void *aim, void *vertex) {

  if ( aim == vertex){
    return true;
  }
  else{
    return false;
  }
}

/******************************************************************************/

bool ptr_eq2(void *aim, void *dat) {
  List l = dat;
  Data d = l->data;
  int *search = d->mem_avail;

  if (*search >= *(int*)aim ){
    return true;
  }
  else{
    return false;
  }
}

/******************************************************************************/

void delete_from_fl(List head, List *free_list, Data d2){

  List rem_mem = find(&ptr_eq1,d2,head);
  List rem_fl = find(&ptr_eq1,rem_mem,*free_list);

  if(rem_fl){

  	free( ((Data)rem_mem->data)->mem_avail );
  	free( rem_mem->data );

    List removed = del(&ptr_eq1,rem_mem,free_list);
    assert(removed);

  }

  List ptr = del(&ptr_eq1,d2,&head);
  assert(ptr);

}

/******************************************************************************/

void merge_spaces(List head,List *memory, List *free_list){
  int *sp1,*sp2;
  List *temp, f_node, f_fl;
  memory = &head;

  while((*memory)){
  	sp1 = ((Data)((*memory)->data))->mem_avail;
  	Data d1 = (Data)(*memory)->data;

    if((*memory)->next){
      temp = &(*memory)->next;
      sp2 = ((Data)((*temp)->data))->mem_avail;
      Data d2 = (Data)(*temp)->data;

      /* There are two spaces in memory that are free */
      if(sp1 && sp2){

        f_node = find(&ptr_eq1,d1,head);
        f_fl = find(&ptr_eq1,f_node,*free_list);

        *sp1 += *sp2;
        delete_from_fl(head,free_list,d2);

        /* The space does not have a link to the free_list */
        if(!f_fl){
          *free_list = push(*free_list,f_node);
        }

        memory = &head;
      }
      /* There is a space in memory */
      else if(sp1){
        f_node = find(&ptr_eq1,d1,head);
        f_fl = find(&ptr_eq1,f_node,*free_list);

        /* The space does not have a link to the free_list */
        if(!f_fl){
          *free_list = push(*free_list,f_node);
          memory = &head;
        }

        memory = &(*memory)->next;
      }
      else{
        memory = &(*memory)->next;
      }

    }
      else{
      memory = &(*memory)->next;
    }
  }
}

/******************************************************************************/

void setting_in_memory(List temp,Process p_swap,List *memory,List *free_list,int clock){
      int memory_available, space_remaining;
      int *spr = malloc(sizeof(int));
      assert(spr);
      Data d1, d2;
      List temp2, temp3;

      memory_available = *(((Data)((List)temp->data)->data)->mem_avail);

      space_remaining = memory_available - p_swap->memory_size;
      *spr = space_remaining;

      /* Setting the space in memory to the new process */
      d1 = ((List)temp->data)->data;
      d1->mem_avail = NULL;
      d1->process = p_swap;

      p_swap->swapped_in_time = clock;

      /* Splitting the memory for the new process if the space remaining is not 0 */
      if(*spr != 0){
      	temp2 = find(&ptr_eq1,d1,*memory);

     	 d2 = malloc( sizeof(*d2) );
      	assert(d2);
      	d2->process = NULL;

      	d2->mem_avail = malloc(sizeof(int));
      	*(d2->mem_avail) = space_remaining;

      	temp3 = temp2->next;
      	temp2->next = NULL;
      	insert(d2,&temp2);

      	if(temp3!=NULL){
        	append(temp3,&temp2);
      	}

      	/* Finding a space in free_list where the process fits */
      	temp->data = find(&ptr_eq1,d2,*memory);
      }
      else{

      	List delete = del(&ptr_eq1,temp->data,free_list);
      	assert(delete);
      }

      printf("time %d, %d loaded, ",clock,p_swap->process_id);
      printf("numprocesses=%d, numholes=%d, ",len(*memory)-len(*free_list),len(*free_list));
      printf("memusage=%d%%\n",mem_usage(memory));

      free(spr);
}

/******************************************************************************/

void organise_free_list(List *memory, List *free_list){

  int *sp_fl,*sp_m;

    while(*memory){

      Data d_m = (*memory)->data;

      if(!(*free_list)){

      }
      else if(d_m->mem_avail){
      	Data d_fl =  ((List)(*free_list)->data)->data;
        sp_fl = d_fl->mem_avail;

        sp_m = d_m->mem_avail;

        /* Check if the space in memory is also the first one in free_list*/
        if(*sp_m != *sp_fl){

          List sw_node = find(&ptr_eq1,*memory,*free_list);
          assert(sw_node);
          swap_nodes(*free_list,sw_node);

          if((*free_list)->next){
            free_list = &(*free_list)->next;
            d_fl = ((List)(*free_list)->data)->data;
          }
          else{
            memory = &(*memory)->next;
            break;
          }
        }
        else{
          free_list = &(*free_list)->next;
        }

      }
      memory = &(*memory)->next;
    }
}

/******************************************************************************/
