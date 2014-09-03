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
#include "SDL_cpuinfo.h"
#include "SDL_thread.h"

namespace parallel{
  template<class T> class For {
#ifdef ANDROID
    static const int MINSCALE = 256;
#else
    static const int MINSCALE = 64;
#endif
    struct ThreadData {
      int lr[2];
      void(*func)(int, const T*);
      const T *data;
    };
  public:
    void run(void(*func)(int i, const T*), int start, int end, const T *data, int scale = -1) {
      static int cpuCount = SDL_GetCPUCount();
      int nthread = cpuCount;
      int range = end - start;
      if (range < nthread * MINSCALE) {
        nthread = range / MINSCALE + 1;
      }
      SDL_Thread **thread = new SDL_Thread*[nthread - 1];
      ThreadData *td = new ThreadData[nthread];
      int ssize = range / nthread;
      int lend = end;
      int i = nthread;
      while (i > 1) {
        int lstart = lend - ssize;
        td[i - 1] = { { lstart, lend }, func, data };
        thread[i - 2] = SDL_CreateThread([](void* ptr) {
            ThreadData &td = *((ThreadData*)ptr);
            for (int i = td.lr[0]; i < td.lr[1]; ++i) {
              td.func(i, td.data);
            }
            return 0;
          }, "ParrallelFor", (void*)&td[i - 1]);
        lend = lstart;
        --i;
      }
	  td[0] = { { start, lend }, func, data };
	  for (int i = td[0].lr[0]; i < td[0].lr[1]; ++i) {
		td[0].func(i, td[0].data);
	  }
	  for (int i = 0; i < nthread - 1; ++i) {
	    SDL_WaitThread(thread[i], NULL);
	  }
      delete[] thread;
      delete[] td;
    }
  };
  template<class T> class Spawn {
    struct ThreadData {
      void(*func)(const T*);
      const T *data;
    };
  public:
    void run(void(*func)(const T*), const T *data) {
      ThreadData *td = new ThreadData();
      *td = { func, new T(*data) };
      SDL_Thread *thread = SDL_CreateThread([](void *ptr){
          ThreadData *ptd = (ThreadData*) ptr;
          ptd->func(ptd->data);
          delete ptd->data;
          delete ptd;
          return 0;
        },"ParrallelSpawn",(void*)td);
      SDL_DetachThread(thread);
    }
  };
}

#endif //__PARALLEL_H__
#endif
