#include <pthread.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <queue>

pthread_mutex_t entire_list_mutex;

template<typename key_t, typename value_t>
class LinkedList{
private:
  struct Node{
  public:
    std::shared_ptr<Node> next_;
    std::shared_ptr<value_t> data_;
    key_t key_;

    Node(){}
    Node(const key_t& key, const std::shared_ptr<value_t>& data, const std::shared_ptr<Node>& next):
                                                          key_(key), data_(data), next_(next){}
  };
  std::shared_ptr<Node> head;
public:
  LinkedList(){}

  ~LinkedList(){head.reset();}

  std::shared_ptr<value_t> find(const key_t& key){
    std::shared_ptr<Node>* place;
    bool found = false;
    for(place = &head; (*place != nullptr); place = &(*place)->next_){
      if((*place)->key_ == key){
        found = true;
        break;
      }
    }
    if(found)
      return (*place)->data_;
    return nullptr;
  }

  void insert(const key_t& key, const std::shared_ptr<value_t>& data){
    std::shared_ptr<Node>* place;
    for(place = &head; (*place != nullptr) && ((*place)->key_ <= key); place = &(*place)->next_);
    *place = std::make_shared<Node>(key, data, *place);
  }

  void remove(const key_t& key){
    std::shared_ptr<Node>* place, *prev;
    bool found = false;
    for(place = &head; (*place != nullptr); prev = &(*place), place = &(*place)->next_){
      if((*place)->key_ == key){
        found = true;
        break;
      }
    }
    if(found){
      if(*place != head)
        (*prev)->next_ = (*place)->next_;
      else
        head = head->next_;
    }
  }

  void print(){
    std::shared_ptr<Node> place;
    for(place = head; (place != nullptr); place = place->next_){
      std::cout << place->key_ << "->";
    }std::cout << "NULL\n";
  }
};

LinkedList<int32_t, void> demo;

enum Operation{MEMBER, INSERTION, DELETION};

std::vector<std::queue<Operation>> task_set;

void* test(void* rank){
  std::size_t idx = (std::size_t)rank;
  std::srand(unsigned(std::time(0)) + idx);
  while(!task_set[idx].empty()){
    switch(task_set[idx].front()){
      case Operation::MEMBER:
        pthread_mutex_lock(&entire_list_mutex);
        demo.find(rand() % 10000);
        pthread_mutex_unlock(&entire_list_mutex);
        break;
      case Operation::INSERTION:
        pthread_mutex_lock(&entire_list_mutex);
        demo.insert(rand() % 10000, nullptr);
        pthread_mutex_unlock(&entire_list_mutex);
        break;
      case Operation::DELETION:
        pthread_mutex_lock(&entire_list_mutex);
        demo.remove(rand() % 10000);
        pthread_mutex_unlock(&entire_list_mutex);
        break;
    }
    task_set[idx].pop();
  }
  return 0;
}

//./main <#_threads> <initial_keys> <#_ops> <% member> <% insert> <% delete>
//./main 4 1000 100000 99.9 0.05 0.05
//./main 4 1000 100000 80.0 10.0 10.0
int main(int32_t argc, char* argv[]){
  if(argc != 7)
    return 1;
  //PREPROCESS BEGIN
  std::srand(unsigned(std::time(0)));
  std::size_t thread_count = atoi(argv[1]), initial_keys = atoi(argv[2]);
  std::size_t ops = atoi(argv[3]);
  std::size_t member_ops = (atof(argv[4]) * (double)ops) / 100.0;
  std::size_t insert_ops = (atof(argv[5]) * (double)ops) / 100.0;
  std::size_t delete_ops = (atof(argv[6]) * (double)ops) / 100.0;

  std::vector<Operation> member_v(member_ops, Operation::MEMBER), 
                         insert_v(insert_ops, Operation::INSERTION), 
                         delete_v(delete_ops, Operation::DELETION), task_vector;
  task_vector.reserve(member_v.size() + insert_v.size() + delete_v.size());
  task_vector.insert(task_vector.end(), member_v.begin(), member_v.end());
  task_vector.insert(task_vector.end(), insert_v.begin(), insert_v.end());
  task_vector.insert(task_vector.end(), delete_v.begin(), delete_v.end());
  std::random_shuffle(task_vector.begin(), task_vector.end());

  for(std::size_t idx = 0; idx < task_vector.size();){
    std::queue<Operation> task_queue;
    for(std::size_t i = 0; i < ops / thread_count; ++i)
      task_queue.push(task_vector[idx++]);
    task_set.push_back(task_queue);
  }

  for(std::size_t i = 0; i < initial_keys; ++i)
    demo.insert(rand() % 10000, nullptr);
  pthread_mutex_init(&entire_list_mutex, NULL);
  std::vector<pthread_t> threads;
  //PREPROCESS END
  std::cout << "<Spam>: PREPROCESS FINISHED\n";
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();
  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, test, (void*)i);
    threads.push_back(thread_id);
  }
  for(std::size_t i = 0; i < thread_count; ++i){
    pthread_join(threads[i], NULL);
  }
  end = std::chrono::high_resolution_clock::now();
  int64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Time: " << ((double)duration)/1e+9 << " s.";

  pthread_mutex_destroy(&entire_list_mutex);
  pthread_exit(NULL);
  return 0;
}