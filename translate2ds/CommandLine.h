#pragma once
#include <string>
#include <vector>
#include "common.h"
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include "log.h"
#include "directory.h"
#undef UNICODE
#include "dirent.h"


namespace sp
{
	class CommandLine
	{
		enum State
		{
			FILES,
			DIRECTORY,
			DATE_START,
			DATE_END,
			OUT_DIR,
			PROJECT,
			PLATFORM,
			FLIGHT_NUMBER,
			SERIALNUMBER,
			MINPARTICLE,
			MAXPARTICLE,
			NONE
		};

	public:
		CommandLine(int args,const char* argv[])
		{
			g_Log <<args<<" command line options found\n";
			if(args <= 1) //the name of the program is the first argument, which we ignore
			{
				g_Log <<"No options specified in command line.  Please specify what you wish this program to do.\n";
				PrintCommandLineOptions();
			}

			State state = NONE;
			std::string op;
			for (int i = 0; i < args; ++i)
			{
				op = argv[i];
				g_Log <<"Option #"<<i+1<<" = " << op <<"\n";
				if(op == "-f")
				{
					state = FILES;
				}
				else if(op == "-d")
				{
					state = DIRECTORY;
				}
				else if(op == "-compress")
				{
					_options.Compressed = true;
				}
				else if(op == "-asciiart")
				{
					_options.Compressed = false;
					_options.ascii_art = true;
					g_Log <<"Generating ASCII art.  '_' represent clear spots while 'x' represents shaded.\n";
				}
				else if(op == "-date_start")
				{
					state = DATE_START;
					_dateSlot  = 0;
				}
				else if(op == "-date_end")
				{
					state = DATE_END;
					_dateSlot = 0;
				}
				else if(op == "-minparticle")
				{
					state = MINPARTICLE;
				}
				else if(op == "-maxparticle")
				{
					state = MAXPARTICLE;
				}
				else if(op == "-o")
				{
					state = OUT_DIR;
				}
				else if(op == "-project")
				{
					state = PROJECT;
				}
				else if(op == "-platform")
				{
					state = PLATFORM;
				}
				else if(op == "-flight")
				{
					state = FLIGHT_NUMBER;
				}
				else if(op == "-sn")
				{
					state = SERIALNUMBER;
				}
				else if(op == "help")
				{
					PrintCommandLineOptions();
				}
				else
				{
					ProcessArg(op, state);
				}
			}
		}

		~CommandLine()
		{
		}

		template<class Op>
		void for_each_file(Op op)
		{
			op.options  = &_options;
			std::for_each(_Files.begin(),_Files.end(), op);
		}

		template<class T>
		struct OpDir
		{
			T* _op;

			OpDir(T& o):_op(&o){}

			void operator () (std::string& dirName)
			{
				DIR *dir;
				struct dirent *ent;

				dir = opendir (dirName.c_str());
				if (dir != NULL)
				{
					std::vector<std::string> file_list;

					while ((ent = readdir (dir)) != NULL) 
					{
						std::string final_file = dirName + "/" + std::string(ent->d_name);
						file_list.push_back(final_file);
					}

					std::sort(file_list.begin(), file_list.end());

					std::vector<std::string>::iterator it;
					for (it = file_list.begin(); it != file_list.end(); ++it)
					{
						(*_op)(*it);
					}
					closedir (dir);
				} else {
					g_Log << "Could not open directory: " << dirName << "\n";
					perror ("");
				}

			}
		};
		template<class Op>
		void for_each_directory(Op op)
		{
			op.options  = &_options;
			OpDir<Op> dirDoer(op);
			std::for_each(_Directories.begin(),_Directories.end(), dirDoer);
		}


	private:

		void ProcessArg(const std::string& op, State state)
		{
			switch (state)
			{
			case FILES:{
				_Files.push_back(op);
				}break;
			case DIRECTORY:{
				_Directories.push_back(op);
				}break;
			case OUT_DIR:{
				_options.OutputDir = op;
				MakeDirectory(op);
				}break;
			case DATE_START:{
				ProcessDate(op, _options.StartTime);
				}break;
			case DATE_END:{
				ProcessDate(op, _options.EndTime);
				}break;
			case MINPARTICLE:{
				_options.MinParticle = atoi(op.c_str());
				}break;
			case MAXPARTICLE:{
				_options.MaxParticle = atoi(op.c_str());
				}break;
			case PROJECT:{
				_options.Project = op;
				}break;
			case PLATFORM:{
				_options.Platform = op;
				}break;
			case FLIGHT_NUMBER:{
				_options.FlightNumber = op;
				}break;
			case SERIALNUMBER:{
				_options.SerialNumber = op;
				}break;
			default:{

				}break;
			}
		}

		void ProcessDate(const std::string& date, TimeStamp16& stamp)
		{
			word val = atoi(date.c_str());
			switch (_dateSlot)
			{
				case 0: {stamp.wYear = val;} break;
				case 1: {stamp.wMonth = val;} break;
				case 2: {stamp.wDay = val;} break;
				case 3: {stamp.wHour = val;} break;
				case 4: {stamp.wMinute = val;} break;
				case 5: {stamp.wSecond = val;} break;
				case 6: {stamp.wMilliseconds = val;} break;
			}
			_dateSlot++;
		}

		typedef std::vector<std::string> Names;

		Names	_Files;
		Names	_Directories;
		Options	_options;

		int	_dateSlot;


		void PrintCommandLineOptions()
		{
			std::cout << "Spec Data Converter Options:\n\n" << "Process specific files:  <-f> <file name> <...>\n" <<
			 "Process Directory:  <-d> <directory name> <...>\n" <<
			 "Start Time:  <-start_time> <year> <month> <day> <hour> <minute> <millisecond>\n" <<
			  "End Time:  <-end_time> <year> <month> <day> <hour> <minute> <millisecond>\n" <<
			  "Set Output Directory:  <-o> <directory name>\n" <<
			  "Set Project: <-project> <name>\n" <<
			  "Set Platform: <-platform> <name>\n"<<
			  "Set Flight Number: <-flight> <number>\n"<<
			  "Set Serial Number: <-sn> <name>\n"<<
			  "Set Min Particle #: <-minparticle> <number>\n"<<
			  "Set Max Particle #: <-maxparticle> <number>\n"<<
			  "ASCII art debug: <-asciiart>\n";
		}
	};
}
