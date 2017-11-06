/*
* Created by Oscar Avellan
*/

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>
#include        <assert.h>

#include        "processes.h"
#include        "list.h"

#define         SIZE_BUFFER 250
#define         NUM_PROCESSES 10
#define         MAX_NUMBER 10000

/* Extern declarations: */

extern  int     optind;
extern  char    *optarg;

/* Starts the process of swapping processes from disk to memory*/
void swap(int clock,List *memory, Process *process_array,List *free_list, List *rrq,List *p_run, char *algorithm, int p_in_main);

/* Copies a process from memory to main */
void swap_to_main(Process *pr, Process *p_array,int clock, int p_in_main);

/* Removes processes from memory */
void swap_out(List *free_list, List *memory, int mem_required, Process *p_array, int clock, List *rrq, List *p_run, int p_in_main);

/* Find the longest sitting process in disk */
Process longest_in_disk(Process *p_array, int clock, int p_in_main);

/* Schedules a process to run, update RRQ and pr_running */
void schedule(List *memory, List *rrq, List *pr_running);

/* Deletes process from memory that have ended their job time */
void process_terminated(List *memory, List *free_list, List *pr_running, Process *p_array, int clock, int p_in_main);

/* Used to call function find in list.c */
List first_fit(List free_list, Process process);

/* Find the first space in memory that fits */
bool first_fit_search(void *aim, void *data);

/* Find the biggest space in memory */
List worst_fit(List *free_list, Process process);

/* Find the best fitted space for the process */
List best_fit(List *free_list, Process process);

/* Returns a spot in the free_list according to one of the algorithms */
List find_spot(List *free_list, Process p_swap, char *algorithm);

/*******************************************************************************************************************************************/
/*******************************************************************************************************************************************/
int main(int argc, char** argv)
{
  FILE *fp;
  char *algorithm, *filename;
  int sizeMemory, quantum, input;
  char *tokens, *buffer;
  int processes_in_main = 0, sizeArray = NUM_PROCESSES;

  while ((input = getopt(argc, argv, "f:a:m:q:")) != EOF)
  {
    switch ( input )
    {
      case 'f':
        filename = optarg;
        break;

      case 'a':
        if(strcmp(optarg,"first") == 0){
          algorithm = optarg; }

        if(strcmp(optarg,"best") == 0){
          algorithm = optarg; }

         if(strcmp(optarg,"worst") == 0){
          algorithm = optarg; }
        break;

      case 'm':
        sizeMemory = atoi(optarg);
        break;

      case 'q':
        quantum = atoi(optarg);
        break;

      default:
        fprintf(stderr,"No arguments specified\n");
        break;
    }
  }

  /* Opening file and storing data in MAIN MEMORY (Array of pointers to processes)*/
  fp = fopen(filename,"r");
  Process *process_array = malloc(sizeof(Process) * NUM_PROCESSES);
  assert(process_array);
  int i = 0;

  if (fp == NULL) {
    fprintf(stderr, "Can't open input file\n");
    exit(1);
  }
  else{
     buffer = malloc(sizeof(char) * SIZE_BUFFER);

     while( (buffer = fgets(buffer,SIZE_BUFFER,fp) ) != NULL){

        tokens = strtok(buffer," ");

        if(processes_in_main == sizeArray){

          sizeArray = sizeArray*2;
          process_array = realloc(process_array, sizeof(Process)*(sizeArray));
          assert(process_array);

        }

        process_array[i] = malloc( sizeof( struct process ));
        assert(process_array[i]);

        process_array[i]->time_created = atoi(tokens);
        tokens = strtok(NULL," ");
        process_array[i]->process_id = atoi(tokens);
        tokens = strtok(NULL," ");
        process_array[i]->memory_size = atoi(tokens);
        tokens = strtok(NULL," ");
        process_array[i]->job_time = atoi(tokens);
        process_array[i]->exit_status = false;
        process_array[i]->swapped_in_time = -1;
        process_array[i]->swapped_out_time = -1;
        process_array[i]->time_remaining = -1;

        i++;
        processes_in_main++;
     }
  }

  /* Creating Memory for processes */
  List memory;
  Data d1 = malloc(sizeof(*d1));
  assert(d1);
  d1->process = NULL;
  d1->mem_avail = &sizeMemory;

  memory = push(NULL, d1);

  /* Creating free_list and assigning the memory list*/
  List free_list = push(NULL,memory);
  List rrq = push(NULL,NULL);
  List pr_running = push(NULL,NULL);
  Process pr;

  /* Creating cycle*/
  int processes_terminated = 0;
  int clock = 0;

  /* Starting Cycle */
  while(processes_terminated != processes_in_main){

    /* The memory is EMPTY*/
    if(len(memory) == 1){
      swap(clock,&memory,process_array,&free_list,&rrq,&pr_running,algorithm,processes_in_main);
      schedule(&memory,&rrq,&pr_running);
      change_process_time(pr_running);

    }
    else if(pr_running->data){
      pr = (Process)((Data)((List)pr_running->data)->data)->process;

      /* Process running time has terminated */
      if(pr->job_time == 0){
        pr->exit_status = true;
        processes_terminated++;

        process_terminated(&memory,&free_list,&pr_running,process_array,clock,processes_in_main);
        swap(clock,&memory,process_array,&free_list,&rrq,&pr_running,algorithm,processes_in_main);
        schedule(&memory,&rrq,&pr_running);

      }

      /* Process quantum has expired*/
      else if(pr->time_remaining == quantum){
        swap(clock,&memory,process_array,&free_list,&rrq,&pr_running,algorithm,processes_in_main);
        schedule(&memory,&rrq,&pr_running);

      }

      change_process_time(pr_running);
    }


    clock++;
  }
  printf("time %d, simulation finished.\n",clock-1);

  free(buffer);
  free_llist(memory);
  free_llist(free_list);
  free_llist(rrq);
  free_llist(pr_running);

  for(i = 0; i < sizeArray ; i++){
    free(process_array[i]);
  }

  free(process_array);

  return 0;
}

/*******************************************************************************************************************************************/

/*******************************************************************************************************************************************/

void process_terminated(List *memory, List *free_list, List *pr_running, Process *p_array, int clock, int p_in_main){

  int p_size;
  List head = *memory;

  Data d_temp = ((List)(*pr_running)->data)->data;
  Process p = d_temp->process;
  p_size = p->memory_size;

  /* Swap the process terminated to Main memory */
  swap_to_main(&p,p_array,clock,p_in_main);

  /* Setting the space in memory to its right values */
  d_temp->mem_avail = malloc(sizeof(int));
  assert(d_temp->mem_avail);
  *(d_temp->mem_avail) = p_size;
  d_temp->process = NULL;

  /* Setting List pr_running to NULL value */
  (*pr_running)->data = NULL;

  merge_spaces(head,memory,free_list);

  memory = &head;
  organise_free_list(memory,free_list);

}

/*************************************************************************************************************************************/
void schedule( List *memory, List *rrq, List *pr_running){

  List temp,temp2;

  while(*memory){
    temp = *memory;
    Data d1 = ((Data)temp->data);
    Process p = (Process)d1->process;

    /* Check if the space in memory has a Process */
    if(p){

      /* There is a process runnning */
      if((*pr_running)->data){
        temp2 = find(&ptr_eq1,temp,*pr_running);

        /* Don't add the process running to the RRQ yet */
        if(!temp2){
          if(add_process_rrq(rrq,temp)){
            break;
          }
        }
      }
      else{

        if(add_process_rrq(rrq,temp)){
          break;
        }
      }

    }

    memory = &(*memory)->next;
  }

  /* Adding running process to the end of RRQ */
  if((*pr_running)->data){
    insert((*pr_running)->data,rrq);
  }

  setting_process_to_run(rrq,pr_running);
}

/*************************************************************************************************************************************/

void swap(int clock,List *memory, Process *process_array,List *free_list,List *rrq,List *p_run, char *algorithm, int p_in_main){

  int mem_required;
  List temp = NULL;
  /* Check for longest process in disk */
  Process p_swap = longest_in_disk(process_array,clock,p_in_main);

  if(p_swap){

    temp = find_spot(free_list,p_swap,algorithm);

    /* No hole big enough for the process */
    if(temp == NULL){

      mem_required = p_swap->memory_size;

      /* Remove processes from memory */
      swap_out(free_list,memory, mem_required,process_array,clock,rrq,p_run,p_in_main);

      /* Find space in free_list*/
      temp = find_spot(free_list,p_swap,algorithm);

      setting_in_memory(temp,p_swap,memory,free_list,clock);

    }
    else{
      setting_in_memory(temp,p_swap,memory,free_list,clock);
    }
  }

}

/*************************************************************************************************************************************/
List find_spot(List *free_list, Process p_swap, char *algorithm){
  List temp = NULL;

  if(strcmp(algorithm,"first") == 0){
    temp = first_fit(*free_list,p_swap);
  }
  else if(strcmp(algorithm,"worst") == 0){
    temp = worst_fit(free_list,p_swap);
  }
  else if(strcmp(algorithm,"best") == 0){
    temp = best_fit(free_list,p_swap);
  }

  return temp;
}
/*************************************************************************************************************************************/


void swap_out(List *free_list, List *memory, int mem_required, Process *p_array, int clock, List *rrq, List *p_run, int p_in_main){

  int longest_time, *ptr, space_size= 0;
  int mem_available = 0;

  List head = *memory;
  List temp_rrq, hole;
  Data d_temp;

  while(mem_available < mem_required){
    longest_time = 0;

    /* Removing longest process sitting in memory */
    temp_rrq = longest_in_memory(memory,clock);
    d_temp = temp_rrq->data;
    space_size = ((Process)((Data)temp_rrq->data)->process)->memory_size;

    delete_from_rrq(temp_rrq,rrq);
    delete_from_p_running(p_run,temp_rrq);

    /* Swap to MAIN memory the necessary processes */
    swap_to_main(&(d_temp->process),p_array,clock,p_in_main);

    /*  Set the empty space in memory */
    d_temp->mem_avail = malloc(sizeof(int));
    assert(d_temp);
    *(d_temp->mem_avail) = space_size;
    d_temp->process = NULL;

    merge_spaces(head,memory,free_list);

    memory = &head;
    organise_free_list(memory,free_list);

    /* Look for hole big enough in free_list */
    ptr = &mem_required;
    hole = find(&ptr_eq2,ptr,*free_list);

    if(hole){
    mem_available = *(int*)(((Data)((List)hole->data)->data)->mem_avail);
    }

    memory = &head;
  }
}

/*********************************************************************************/

Process longest_in_disk(Process *p_array, int clock, int p_in_main){
  int i,process_time,longest_time = 0,index = -1,process_id = MAX_NUMBER;

  Process p_return;

  for(i = 0; i < p_in_main; i++){

    /* There is no process or process finish its execution time */
    if(p_array[i] == NULL || p_array[i]->exit_status){
      continue;
    }

    /* Process has been swapped in and out of memory */
    if(p_array[i]->swapped_out_time > 0){

      process_time = clock - p_array[i]->swapped_out_time;
      checking_process_time(&longest_time,&index,&process_id,p_array[i],i,&process_time);
    }

    /* Processes created at time 0 */
    else if(!p_array[i]->time_created && p_array[i]->process_id < process_id){
      index = i;
      process_id = p_array[i]->process_id;
      longest_time = 5000;
    }

    else{

      if(p_array[i]->time_created > 0){
        process_time = clock - p_array[i]->time_created;
        checking_process_time(&longest_time,&index,&process_id,p_array[i],i,&process_time);
      }

    }

  }

  if(index >= 0){

    if(p_array[index]->time_created == 0){
      p_array[index]->time_created = -1;
    }

    p_return = p_array[index];
    p_array[index] = NULL;
    return p_return;
  }

  return NULL;

}

/*********************************************************************************/

List first_fit(List free_list, Process process){

  int *pr_size;
  pr_size = &process->memory_size;

  return find(&first_fit_search,pr_size,free_list);
}

/*********************************************************************************/

bool first_fit_search(void *aim, void *data){
  List a = (List)data;

  int *target = ((Data)(a->data))->mem_avail;
  int *look = aim;


  if(*target >= *look){
    return true;
  }
  else{
    return false;
  }
}

/*********************************************************************************/

List worst_fit(List *free_list, Process process){
  int *pr_size, biggest_space = 0;
  pr_size = &process->memory_size;
  List temp = NULL;

  while(*free_list){

    if(*((int*)((Data)((List)(*free_list)->data)->data)->mem_avail) > biggest_space){
      biggest_space = *((int*)((Data)((List)(*free_list)->data)->data)->mem_avail);

      if(biggest_space >= *pr_size){
        temp = *free_list;
      }
    }
    free_list = &(*free_list)->next;
  }

  return temp;

}

/*********************************************************************************/

List best_fit(List *free_list, Process process){
  int *pr_size,*fl_node, biggest_space = MAX_NUMBER;
  pr_size = &process->memory_size;
  List temp = NULL;
  fl_node = ((Data)((List)(*free_list)->data)->data)->mem_avail;

  while(*free_list){
    fl_node = ((Data)((List)(*free_list)->data)->data)->mem_avail;

    if(*fl_node < biggest_space && *fl_node >= *pr_size){
      biggest_space = *fl_node;
      temp = *free_list;
    }

    free_list = &(*free_list)->next;
  }

  return temp;
}

/*********************************************************************************/

 void swap_to_main(Process *pr, Process *p_array,int clock ,int p_in_main){
    int i;
    for(i = 0; i < p_in_main; i++){
      if(!p_array[i]){
        p_array[i] = *pr;
        p_array[i]->swapped_out_time = clock;
        break;
      }
    }
 }
