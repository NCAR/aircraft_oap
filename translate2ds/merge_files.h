#pragma once
#include <string>
#include <fstream>
#include "common.h"
#include "log.h"
namespace sp
{

	void	AppendFile(std::ifstream& in, std::ofstream& out)
	{
		enum
		{
			BYTES_PER_READ = 1000,
		};

		byte buffer[BYTES_PER_READ];

		while(true)
		{
			in.read(reinterpret_cast<char*>(buffer), BYTES_PER_READ);
			std::streamsize count =  in.gcount();
			out.write(reinterpret_cast<char*>(buffer),count);

			if(count != BYTES_PER_READ)
				break;
		}
	}

	bool	MergeFiles(const std::string& file1, const std::string& file2, const std::string& new_file_name)
	{
		std::ifstream tf1(file1.c_str(), std::ios::binary);
		std::ifstream tf2(file2.c_str(), std::ios::binary);

		std::ofstream file(new_file_name.c_str(),std::ios::binary);

                std::cout << "  Merging into : " << new_file_name << std::endl;

		if(!tf1.is_open() || !tf2.is_open() || !file.is_open())
		{
			g_Log << "Merge files failed with " << file1 <<", "<< file2 <<", "<< new_file_name<<"\n";
			return false;
		}

		AppendFile(tf1,file);
		AppendFile(tf2,file);

		return true;

	}
}
