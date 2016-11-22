#ifndef __TIMER_H
#define __TIMER_H
#include "Noncopyable.h"

#ifdef __linux__
#include <sys/time.h>
#include <sys/types.h>
#elif _WIN32
#include <GL/freeglut.h>
#endif
class Timer :Noncopyable{
	public:
		
#ifdef __linux__
	static int64_t getTimeOfDay(){
		struct timeval tv;
			::gettimeofday(&tv,nullptr);
			return static_cast<int64_t>(tv.tv_sec)*1000000
				+(tv.tv_usec);
	}
#elif _WIN32
		using int64_t = long long;
	static long long getTimeOfDay() {
		return 1000*(long long)glutGet(GLUT_ELAPSED_TIME);
	}
#endif

	Timer (){
        //the clock is stopped in the beginning
        time_elapsed_ = 0;
        last_tic_ = getTimeOfDay();
        running_ = false;
    }
    Timer (Timer && rhs):
        last_tic_(rhs.last_tic_),
        time_elapsed_(rhs.time_elapsed_),
        running_(rhs.running_)
    {
    }
    void start (){
        if (running_)return;
        running_ = true;
        last_tic_ = getTimeOfDay();
    }
    void stop(){
        if(!running_)return;
        running_ = false;
        int64_t now = getTimeOfDay();
        time_elapsed_ += (now - last_tic_);
        last_tic_ = now;
    }
    bool isRunning(){
        return running_;
    }
    void changeStat(){
        if (isRunning()){
            stop();
        }
        else start();
    }

    //return time in usec
    //notice this function returns the time of
    //total time of running,
    //not the time since object is created.
    int64_t runningTime(){
        if (running_){
            return getTimeOfDay()-last_tic_ + time_elapsed_;
        }
        else {
            return time_elapsed_;
        }
    }
	private:
	int64_t last_tic_;
    int64_t time_elapsed_;
    bool running_;
};
#endif //__TIMER_H
