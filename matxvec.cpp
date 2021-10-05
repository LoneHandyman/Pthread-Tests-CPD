#include <pthread.h>
#include <iostream>
#include <vector>
#include <chrono>

std::vector<std::vector<double>> A;
double *y, *vec;
std::size_t n, m;
std::size_t thread_count;
std::vector<pthread_t> threads;

void* matxvec(void* rank){
  std::size_t local_n = n / thread_count;
  std::size_t begin = (std::size_t)rank * local_n;
  for(std::size_t i = begin; i < begin + local_n; ++i){
    y[i] = 0;
    for(std::size_t j = 0; j < m; ++j)
      y[i] += A[i][j] * vec[j];
  }
  return 0;
}

int main(){
  std::cin >> thread_count >> n >> m;
  y = new double[n];
  vec = new double[m];

  for(std::size_t i = 0; i < m; ++i){
    vec[i] = (double)(rand() % 100);
  }
  for(std::size_t i = 0; i < n; ++i){
    std::vector<double> row;
    for(std::size_t j = 0; j < m; ++j){
      row.push_back((double)(rand() % 100));
    }
    A.push_back(row);
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();
  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, matxvec, (void*)i);
    threads.push_back(thread_id);
  }

  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_join(threads[i], NULL);
  }
  end = std::chrono::high_resolution_clock::now();
  int64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Time: " << ((double)duration)/1e+9 << " s.";
  delete[] y;
  delete[] vec;
  pthread_exit(NULL);
  return 0;
}