#if 0
struct timespec { long tv_sec; long tv_nsec; };    //header part
int clock_gettime(int, struct timespec *spec)      //C-file part
{
	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= 116444736000000000i64;  //1jan1601 to 1jan1970
	spec->tv_sec = wintime / 10000000i64;           //seconds
	spec->tv_nsec = wintime % 10000000i64 * 100;      //nano-seconds
	return 0;
}
#endif

#if 0
struct timespec { long tv_sec; long tv_nsec; };   //header part
#define exp7           10000000i64     //1E+7     //C-file part
#define exp9         1000000000i64     //1E+9
#define w2ux 116444736000000000i64     //1.jan1601 to 1.jan1970
void unix_time(struct timespec *spec)
{
	__int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	wintime -= w2ux;  spec->tv_sec = wintime / exp7;
	spec->tv_nsec = wintime % exp7 * 100;
}
int clock_gettime(int, timespec *spec)
{
	static  struct timespec startspec; static double ticks2nano;
	static __int64 startticks, tps = 0;    __int64 tmp, curticks;
	QueryPerformanceFrequency((LARGE_INTEGER*)&tmp); //some strange system can
	if (tps != tmp) {
		tps = tmp; //init ~~ONCE         //possibly change freq ?
		QueryPerformanceCounter((LARGE_INTEGER*)&startticks);
		unix_time(&startspec); ticks2nano = (double)exp9 / tps;
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&curticks); curticks -= startticks;
	spec->tv_sec = startspec.tv_sec + (curticks / tps);
	spec->tv_nsec = startspec.tv_nsec + (double)(curticks % tps) * ticks2nano;
	if (!(spec->tv_nsec < exp9)) { spec->tv_sec++; spec->tv_nsec -= exp9; }
	return 0;
}
#endif
