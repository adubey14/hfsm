#ifndef SM_UTILS_H
#define SM_UTILS_H

#ifndef WIN32
#include <sys/time.h>
#else
#pragma warning(disable:4244)
#include <SYS\Timeb.h>
#endif
#include <assert.h>



namespace STATEMACHINE_UTILS 
{

	struct timespec {
        long tv_sec;
        long tv_nsec;
	};

	#define SYSTEM_TIMESPEC struct timespec 
	typedef long long  SYSTEM_TIME_TYPE;

	#define NANO 1000000000					//!<9 zeros
	#define MILLI 1000						//!<3 zeros
	#define MICRO 1000000					//!<6 zeros
	#define MAX_TIME_OUT (SYSTEM_TIME_TYPE)315569260*NANO  //!<100 years;
	#define INFINITE_TIME_VALUE (SYSTEM_TIME_TYPE)-1


	inline SYSTEM_TIMESPEC currentTime(){
		SYSTEM_TIMESPEC tv;	
#ifndef WIN32
		clock_gettime(CLOCK_REALTIME , &tv);	
#else
		struct _timeb currSysTime;
		long long  nanosecs, secs;

		const long long NANOSEC_PER_MILLISEC = 1000000;
		const long long NANOSEC_PER_SEC = 1000000000;

		/* get current system time and add millisecs */
		_ftime_s(&currSysTime);
		secs = (long long)(currSysTime.time) ;
		nanosecs = ((long long) (currSysTime.millitm)) * NANOSEC_PER_MILLISEC;
		if (nanosecs >= NANOSEC_PER_SEC)
		{
			secs++;
			nanosecs -= NANOSEC_PER_SEC;
		}
		else if (nanosecs < 0)
		{
			secs--;
			nanosecs += NANOSEC_PER_SEC;
		}

		tv.tv_nsec = (long)nanosecs;
		tv.tv_sec = (long)secs;
#endif
		return tv;	
	}

	inline SYSTEM_TIME_TYPE convertTimeSpecToNano(const SYSTEM_TIMESPEC& tv)
	{
		SYSTEM_TIME_TYPE time = (SYSTEM_TIME_TYPE)tv.tv_sec* NANO+(SYSTEM_TIME_TYPE)tv.tv_nsec;
		return time;
	}

	inline long double currentTimeInSeconds(){
		SYSTEM_TIMESPEC tv=currentTime();	
		long double time =(long double)tv.tv_sec + (long double)tv.tv_nsec/NANO ;
		return time;	
	}

	inline SYSTEM_TIME_TYPE currentTimeInNanoSeconds(){
		SYSTEM_TIMESPEC tv=currentTime();	
		SYSTEM_TIME_TYPE time = (SYSTEM_TIME_TYPE)tv.tv_sec* NANO+(SYSTEM_TIME_TYPE)tv.tv_nsec;
		return time;	
	}


	inline SYSTEM_TIMESPEC addTime(const SYSTEM_TIMESPEC& first, const SYSTEM_TIMESPEC& second){
		SYSTEM_TIMESPEC tv;
		long long nsecs=first.tv_nsec+second.tv_nsec;
		long long secs=first.tv_sec+second.tv_sec;
		while (nsecs >= NANO){
			secs=secs+1;
			nsecs=nsecs-NANO;
		}
		tv.tv_nsec=nsecs;
		tv.tv_sec=secs;
		return tv;		
	}

	

	

	inline SYSTEM_TIMESPEC operator + (const SYSTEM_TIMESPEC& first, const SYSTEM_TIMESPEC& second){
		return addTime(first,second);
	}

	inline SYSTEM_TIMESPEC addTimeNano(const SYSTEM_TIMESPEC& first, const SYSTEM_TIME_TYPE& nanoseconds)
	{

		assert(nanoseconds<=(SYSTEM_TIME_TYPE)MAX_TIME_OUT);
		if (nanoseconds == INFINITE_TIME_VALUE)
		{	
			SYSTEM_TIMESPEC tv;
			long long nsecs= first.tv_nsec + (SYSTEM_TIME_TYPE)MAX_TIME_OUT;
			long long secs = nsecs/(long long)NANO;
			secs += first.tv_sec;
			nsecs = nsecs%(long long)NANO;
			tv.tv_sec=secs;
			tv.tv_nsec=nsecs;
			return tv;	

		}
		else
		{		
			SYSTEM_TIMESPEC tv;
			long long nsecs= first.tv_nsec + nanoseconds;
			long long secs = nsecs/(long long)NANO;
			secs += first.tv_sec;
			nsecs = nsecs%(long long)NANO;
			tv.tv_sec=secs;
			tv.tv_nsec=nsecs;
			return tv;		
		}
	}
	inline SYSTEM_TIMESPEC operator +(const SYSTEM_TIMESPEC& first, const SYSTEM_TIME_TYPE& nanoseconds)
	{
		return addTimeNano(first,nanoseconds);

	}
	inline SYSTEM_TIMESPEC addTimeMilli(const SYSTEM_TIMESPEC& first, long& milliseconds){
		//it will call the ont with long long nanoseconds, but it's cleaner than calling something from the code in form wcet * 1000000;
		long long nanoseconds=milliseconds*MICRO;
		return addTimeNano(first,nanoseconds);
	}





	inline void stringTrim(std::string& str,std::string delims){		
		std::string::size_type pos = str.find_last_not_of(delims);
		if(pos != 	std::string::npos) {
			str.erase(pos + 1);
			pos = str.find_first_not_of(delims);
			if(pos != 	std::string::npos) str.erase(0, pos);
		}
		else str.erase(str.begin(), str.end());

	}

	inline void stringTokenize(const std::string& str,
		std::vector<std::string>& tokens,
		std::string delimiters)
	{
		// Skip delimiters at beginning.
		std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		std::string::size_type pos = str.find_first_of(delimiters, lastPos);

		while (	std::string::npos != pos || 	std::string::npos != lastPos)
		{
			// Found a token, add it to the vector.
			std::string substring=str.substr(lastPos, pos - lastPos);
			stringTrim(substring," \t\r\n\0");
			tokens.push_back(substring);
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

	inline SYSTEM_TIME_TYPE convertSecsToNano(double DURATION){
		long double DURATION_d =DURATION;
		DURATION_d=DURATION_d*1e9; ///Converting to NanoSeconds
		char buffer [100];
		_snprintf_s(buffer,100,"%Lf",DURATION_d);
		std::string bufferstr(buffer);
		std::string outputval;
		std::vector<std::string> tempbufferparts;
		stringTokenize(bufferstr,tempbufferparts,".");
		if(tempbufferparts.size()==0)
			outputval=bufferstr;
		else
			outputval=tempbufferparts[0];
		SYSTEM_TIME_TYPE temp_dur =_atoi64(outputval.c_str());

		return temp_dur;		
	}


	inline bool compareTime(const SYSTEM_TIMESPEC& first, const SYSTEM_TIMESPEC& second)
	{
		if (first.tv_sec < second.tv_sec)
			return true;

		if (first.tv_sec > second.tv_sec)
			return false;

		if (first.tv_nsec <= second.tv_nsec)
			return true;

		return false;

	}


	inline bool compareTimeLess(const SYSTEM_TIMESPEC& first, const SYSTEM_TIMESPEC& second)
	{
		if (first.tv_sec < second.tv_sec)
			return true;

		if (first.tv_sec > second.tv_sec)
			return false;

		if (first.tv_nsec < second.tv_nsec)
			return true;

		return false;

	}

	struct SYSTEM_TIMESPEC_COMP : public std::binary_function <SYSTEM_TIMESPEC, SYSTEM_TIMESPEC, bool> 
	{
	public:
		bool operator()(SYSTEM_TIMESPEC d1, SYSTEM_TIMESPEC d2) const
		{
			return STATEMACHINE_UTILS::compareTimeLess(d1,d2) ;
		}

	};

} // namespace STATEMACHINE_UTILS 

#endif//SM_UTILS_H