/*
*	Created by Oscar Avellan
*/

#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "list.h"

typedef struct process* Process;
typedef struct data_t* Data;

struct process{
	int time_created;
	int process_id;
	int memory_size;
	int job_time;
	bool exit_status;
	int swapped_in_time;
	int swapped_out_time;
	int time_remaining;
};

struct data_t{
	Process process;
	int *mem_avail;
};

/* Increase the quantum time and decrease the job time in the process */
void change_process_time(List pr_running);

/* Checks the time of a process from the time in disk or swapped out time */
void checking_process_time(int *longest_time, int *index, int *process_id, Process p, int i, int *process_time);

/* Adds a process to the process running List from the RRQ */
void setting_process_to_run(List *rrq, List *pr_running);

/* Add process to RRQ */
bool add_process_rrq(List *rrq,List temp);

/* Returns longest process sitting in memory */
List longest_in_memory(List *memory, int clock);

/* Sets the new process in memory and prints the required output */
void setting_in_memory(List temp,Process p_swap,List *memory,List *free_list,int clock);

/* Delete process from RRQ */
void delete_from_rrq(List temp_rrq, List *rrq);

/* Set the list process running to NULL */
void delete_from_p_running(List *p_run, List temp_rrq);

/* Delete empty space from free_list */
void delete_from_fl(List head, List *free_list, Data d2);

void merge_spaces(List head, List *memory, List *free_list);

/* Returns the percentage of memory being used */
int mem_usage(List *memory);

/* Display processes in RRQ */
void *print_rrq(void *acc, void *data);

/* Print nodes in memory */
void *print_nodes_information(void *acc, void *data);

/* Print nodes in free_list */
void *print_free_list(void *acc, void *data);

/* Checks if both variables are poiniting to the same space */
bool ptr_eq1(void *aim, void *vertex);

/**/
bool ptr_eq2(void *aim, void *dat);

/* Makes sure that the free_list is organised from higher addresses to lower addresses */
void organise_free_list(List *memory, List *free_list);

#endif
