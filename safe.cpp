#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <vector>

#define MAX 1000

std::size_t thread_count;
sem_t *sems;
FILE* input_file;

void* tokenize(void* rank){
  std::size_t id = (std::size_t)rank;
  int32_t count;
  int32_t next = (id + 1) % thread_count;
  char *fg_rv;
  char my_line[MAX];
  char *my_string;

  sem_wait(&sems[id]);
  fg_rv = fgets(my_line, MAX, input_file);
  sem_post(&sems[next]);
  while (fg_rv != NULL) {
    printf("Thread %ld > my line = %s\n", id, my_line);
    count = 0;
    my_string = strtok(my_line, " \t\n");
    while ( my_string != NULL ) {
      ++count;
      printf("Thread %ld > string %d = %s\n", id, count, my_string);
      my_string = strtok(NULL, " \t\n");
    }
    sem_wait(&sems[id]);
    fg_rv = fgets(my_line, MAX, input_file);
    sem_post(&sems[next]);
  }
  return 0;
}

/*
Pease porridge hot.
Pease porridge cold.
Pease porridge in the pot
Nine days old.
 
*/

int main(){
  std::cin>>thread_count;
  std::vector<pthread_t> threads;
  sems = new sem_t[thread_count];
  input_file = fopen("input.txt", "r");
  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_t thread_id;
    sem_init(&sems[i], 0, i % 2);
    pthread_create(&thread_id, NULL, tokenize, (void*)i);
    threads.push_back(thread_id);
  }
  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_join(threads[i], NULL);
    sem_destroy(&sems[i]);
  }
  fclose(input_file);
  delete[] sems;
  return 0;
}