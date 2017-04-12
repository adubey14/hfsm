#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <fstream>

namespace STATEMACHINE_UTILS {

	class LogStream : public std::ofstream
	{
	public:

		LogStream();
		~LogStream();
		void flushit();

		// friend put-to operators
		friend LogStream& operator<<( LogStream& s, const char * r);
		friend LogStream& operator<<( LogStream& s, const std::string& r);
		friend LogStream& operator<<( LogStream& s, const int i);
		friend LogStream& operator<<( LogStream& s, const double i);
		friend LogStream& operator<<( LogStream& s, const unsigned long l);
		std::string  m_buff;
	};

	class Globals
	{
	private:
		Globals():outputName(){}
		static Globals* _pinstance;
		~Globals(){}

	public :

		static void TerminateInstance();
		static Globals* Instance();
		std::string outputName;
		LogStream reg;

	};

}//namespace STATEMACHINE_UTILS 

#endif