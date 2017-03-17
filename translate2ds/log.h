#pragma once
#include <fstream>
#include <ostream>
#include <iostream>
namespace sp
{
	class Log
	{
	public:
		Log(const char* file_name = "log.txt"): _out(file_name)
		{

		}
		~Log(void)
		{
			_out.close();
		}

		template<class T>
		Log& operator << (const T& obj)
		{
			std::cout << obj;

			_out << obj;
			return *this;
		}

	private:

		std::ofstream	_out;
	};

}

extern sp::Log	g_Log;
