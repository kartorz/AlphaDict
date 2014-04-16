#include "alphadict.h"
#include "string.h"
#include "Util.h"

#include <time.h>
#include <stdarg.h>
#include <wchar.h>

unsigned int Util::getTimeMS()
{
	static unsigned long long start_mstime = 0;
	unsigned long long now_mstime;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	now_mstime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
	if (start_mstime == 0) {
		start_mstime = now_mstime;
	}
	
	return(now_mstime - start_mstime);
}

/* This piece of code comes from an example of mbrtowc function of GNU libc. */
wchar_t Util::mbrtowc_r(char** mb)
{
	wchar_t wctmp[1];
	size_t len = strlen(*mb);
	size_t nbytes = mbrtowc(wctmp, *mb, len, NULL);
	if (nbytes > 0) {
		if (nbytes > (size_t) - 2)
			return 0;
		*mb += nbytes;
		return wctmp[0];
	}
}

/* This piece of code comes from an example of mbrtowc function of GNU libc. */
wchar_t* Util::mbstowcs(const char *mb)
{
	size_t len = strlen(mb);
	wchar_t *result = (wchar_t *)malloc((len+1)*sizeof(wchar_t));
	wchar_t *wcp = result;
	wchar_t tmp[1];
	mbstate_t state;
	size_t nbytes;
	memset(&state, '\0', sizeof(state));
	memset(result, '\0', (len+1)*sizeof(wchar_t));
	while ((nbytes = mbrtowc(tmp, mb, len, &state)) > 0)
	{
		if (nbytes >= (size_t) -2) {
		    g_log.e("mbstowcs: invalid input string\n");
			return NULL;
		}
		*wcp++ = tmp[0];
		len -= nbytes;
		mb += nbytes;
	}
	return result;
}

size_t ReadFile::operator()(FILE *f, void *ptr, size_t length)
{
	int rdbytes = 0;
	do {
		int bytes = length - rdbytes;
		rdbytes += fread(ptr, 1, bytes, f);			
	}while(rdbytes > 0 && rdbytes < length);
	return rdbytes;
}

void* ReadFile::operator()(FILE *f, size_t length)
{
	if (ptr != NULL)
		free(ptr);
	ptr = malloc(length);
	int rdbytes = 0;
	do {
		int bytes = length - rdbytes;
		rdbytes = fread(ptr, 1, bytes, f);			
	}while(rdbytes > 0 && rdbytes < length);
	return ptr;		
}

