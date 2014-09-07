#ifdef USE_PARALLEL
#ifndef __QUEUE_TS_H__
#define __QUEUE_TS_H__
#include "SDL_atomic.h"

namespace utils{
  template<typename T>
    struct Node {
      T node;
      Node<T> *next = nullptr;
    };
  
  template<typename T>
    class Queue {
    Node<T> *front = nullptr, *rear = nullptr;
    SDL_SpinLock lock;
  public:
    bool empty(){
      SDL_AtomicLock(&lock);
      bool ret = (front == nullptr);
      SDL_AtomicUnlock(&lock);
      return ret;
    }
    void push(const T& value){
      SDL_AtomicLock(&lock);
      if (front == nullptr) {
        front = rear = new Node<T>();
        front->node = value;
      } else {
        Node<T> *prear = rear;
        rear = new Node<T>();
        rear->node = value;
        prear->next = rear;
      }
      SDL_AtomicUnlock(&lock);
    }
    T* pop(){
      SDL_AtomicLock(&lock);
      if (front != nullptr) {
        Node<T> *pfront = front;
        front = pfront->next;
        if (front == nullptr) rear = nullptr;
        auto ret = new T(pfront->node);
        delete pfront;
        SDL_AtomicUnlock(&lock);
        return ret;
      } else {
        SDL_AtomicUnlock(&lock);
        return nullptr;
      }
    }
    ~Queue(){
      auto pfront = pop();
	  while (pfront != nullptr) {
        delete pfront;
        pfront = pop();
      }
    }
  };
}

#endif //__QUEUE_TH_H__
#endif
