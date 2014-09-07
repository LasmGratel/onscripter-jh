/* -*- C++ -*-
 *
 *  Parallel.h
 *
 *  Copyright (C) 2014 jh10001 <jh10001@live.cn>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef USE_PARALLEL
#ifndef __PARALLEL_H__
#define __PARALLEL_H__
#include <assert.h>
#include "SDL_cpuinfo.h"
#include "SDL_thread.h"
#include "SDL_timer.h"
#include "Queue_ts.h"

namespace parallel{
  template<typename Body>
  void For(const int first, const int last, const int step, const Body &body, const int scale = -1){
    assert(step > 0);
    if (last > first) {
      // Above "else" avoids "potential divide by zero" warning on some platforms
      int range = last - first;
#ifdef ANDROID
      static const int MINSCALE = 65536;
#else
      static const int MINSCALE = 4096;
#endif
      struct ThreadData {
        int lr[2];
        const Body *body;
      };
      static int cpuCount = SDL_GetCPUCount();
      int nthread = cpuCount;
      if(scale > 0){
        nthread = scale / MINSCALE + 1;
        if(nthread > cpuCount) nthread = cpuCount;
      } else if (range < nthread) {
        nthread = range;
      }
      SDL_Thread **thread = new SDL_Thread*[nthread - 1];
      ThreadData *td = new ThreadData[nthread];
      int ssize = range / nthread;
      int lend = last;
      int i = nthread;
      while (i > 1) {
        int lstart = lend - ssize;
        td[i - 1] = { { lstart, lend }, &body };
        thread[i - 2] = SDL_CreateThread([](void* ptr) {
            ThreadData &td = *((ThreadData*)ptr);
            for (int i = td.lr[0]; i < td.lr[1]; ++i) {
              (*td.body)(i);
            }
            return 0;
          }, "ParrallelFor", (void*)&td[i - 1]);
        lend = lstart;
        --i;
      }
      td[0] = { { first, lend }, &body };
      for (int i = td[0].lr[0]; i < td[0].lr[1]; ++i) {
        (*td[0].body)(i);
      }
      for (int i = 0; i < nthread - 1; ++i) {
        SDL_WaitThread(thread[i], NULL);
      }
      delete[] thread;
      delete[] td;
    }
  }
  template<typename Body>
  class LazySpawn{
    utils::Queue<Body> queue;
    SDL_Thread *thread = nullptr;
    void startLazyThread(){
      thread = SDL_CreateThread([](void *ptr){
          auto queue = (utils::Queue<Body>*)ptr;
          for(;;){
            Body *pb = queue->pop();
            while(pb != nullptr){
              (*pb)();
              delete pb;
              pb = queue->pop();
            }
            SDL_Delay(30);
          }
          return 0;
        },"ParallelLazySpawn",(void*)&queue);
      SDL_DetachThread(thread);
    }
  public:
    void push(const Body &body){
      queue.push(body);
      if(thread == nullptr) startLazyThread();
    }
  };
}

#endif //__PARALLEL_H__
#endif
