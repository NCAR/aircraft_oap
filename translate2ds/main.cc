#include <sys/stat.h>
#include <locale>

#include "File.h"
#include "2DS.h"
#include "2DS_reverse.h"
#include "3VCPI.h"
#include "UCAR_Writer.h"
#include "CommandLine.h"
#include "directory.h"

sp::Log	g_Log("program.log");


struct DoF2DS
{
	sp::Options*	options;

	void operator () (const std::string& file_name) const
	{
		sp::Device3VCPI	device(*options);
		sp::File	file(file_name);
		sp::File	file_hk(file_name+"HK");

		if (file.is_open() == false) { return; }

		g_Log << "Generating NCAR OAP .2d from Fast2DS file \"" << file_name << "\" of size "
			<< file.MegaBytes() << " MB.\n";

		std::string outfile = file_name;
		outfile.erase(0, outfile.find("base"));
		outfile.erase(outfile.end() - 5, outfile.end()); //remove .F2DS

		sp::UCAR_Writer writer(outfile, *options, sp::HORIZONTAL_2DS, sp::VERTICAL_2DS,
					"F2DS", "10", "128", "_2H", "_2V");
/*
		if (file_hk.is_open())
		{
			std::cout << "Processing housekeeping file.\n";
			device.ProcessHK(file_hk, writer);
		}
*/
		device.ProcessData(file, writer);
	}
};

struct Do3VCPI
{
	sp::Options*	options;

	void operator () (const std::string& file_name) const
	{
		sp::Device3VCPI	device(*options);
		sp::File	file(file_name);
		sp::File	file_hk(file_name+"HK");

		if (file.is_open() == false) { return; }

		g_Log << "Generating NCAR OAP .2d from 3V-CPI/2DS file \"" << file_name << "\" of size "
			<< file.MegaBytes() << " MB.\n";

		std::string outfile = file_name;
		outfile.erase(0, outfile.find("base"));
		outfile.erase(outfile.end() - 7, outfile.end()); //remove .2dscpi

		sp::UCAR_Writer writer(outfile, *options, sp::HORIZONTAL_3VCPI, sp::VERTICAL_3VCPI,
					"3V-CPI", "10", "128", "_3H", "_3V");
/*
		if (file_hk.is_open())
		{
			std::cout << "Processing housekeeping file.\n";
			device.ProcessHK(file_hk, writer);
		}
*/
		device.ProcessData(file, writer);
	}
};

struct DoHVPS
{
	sp::Options*	options;

	void operator () (const std::string& file_name) const
	{
		sp::Device2DS	device;
		sp::File	file(file_name);

		if (file.is_open() == false) { return; }

		g_Log	<< "Generating NCAR OAP .2d from HVPS file \"" << file_name << "\" of size "
			<< file.MegaBytes() << " MB.\n";

		std::string outfile = file_name;
		outfile.erase(0, outfile.find("base"));
		outfile.erase(outfile.end() - 5, outfile.end()); // remove .hvps

		sp::UCAR_Writer writer(outfile, *options, 0, sp::HVPS,
					"HVPS", "150", "128", "", "_H1");
		device.Process(file, writer);
	}
};

struct Do2DS
{
	sp::Options*	options;

	void operator () (const std::string& file_name) const
	{
// I wonder if the real issue is we were trying to process Fast2DS data in here, and that is same
// format as 3V-CPI.  I've since added a DoF2DS().  --cjw 6/2022

		//sp::Device2DS	device;
		sp::Device3VCPI	device(*options);
		sp::File	file(file_name);

		if (file.is_open() == false) { return; }

		g_Log	<< "Generating NCAR OAP .2d from 2DS file \"" << file_name << "\" of size "
			<< file.MegaBytes() << " MB.\n";

		std::string outfile = file_name;
		outfile.erase(0, outfile.find("base"));
		outfile.erase(outfile.end() - 4, outfile.end()); // remove .2ds

		// The 2DS processor doesn't correctly process 2DS data, but the 2DSCPI
		// processor seems to process the 2DS data correctly, so just use that
		// routine for this data.
		sp::UCAR_Writer writer(outfile, *options, sp::HORIZONTAL_2DS, sp::VERTICAL_2DS,
					"2DS", "10", "128", "_2H", "_2V");
		device.ProcessData(file, writer);
		// This is the call to the 2DS processor.
		//sp::UCAR_Writer writer(outfile, *options, sp::HORIZONTAL_2DS, sp::VERTICAL_2DS,
		//			"2DS", "10", "128", "_2H", "_2V");
		//device.Process(file, writer);
	}
};

struct Do2DS_Reverse
{
	sp::Options*	options;

	void operator () (const std::string& file_name) const
	{
		sp::Device2DS_reverse	device(*options);
		sp::File		file(file_name);

		if (file.is_open() == false) { return; }

		g_Log	<< "Generating Reverse .2DS from file \"" << file_name << "\" of size "
			<< file.MegaBytes() << " MB's\n";

		std::string outfile = file_name;
		outfile.erase(0, outfile.find("base"));
		outfile.erase(outfile.end() - 4, outfile.end());
		outfile += ".2ds";

		std::string file_full_path = options->OutputDir + "/" + outfile.c_str();
		std::ofstream	writer(file_full_path.c_str(), std::ios::binary);

		device.Process(file, writer);
	}
};


struct ProcessFile
{
	sp::Options*	options;

	bool	contains(const std::string& fName, const char* match)const
	{
		std::string copy(match);
		if(fName.length() < copy.length())
			return false;
std::cout << "In=" << fName << ", " << match <<'\n';
		std::transform(fName.end()-strlen(match), fName.end(), copy.begin(), ::tolower);
std::cout << "Copy=" << copy << ", " << match <<'\n';

		return copy.find(match) != std::string::npos;
	}

	void operator () (const std::string& file_name) const
	{
		if (contains(file_name, ".2dscpi") && file_name.find("base") != std::string::npos)
		{
			Do3VCPI doit = {options};
			doit(file_name);
		}
		else if(contains(file_name, ".f2ds") && file_name.find("base") != std::string::npos)
		{
			DoF2DS doit = {options};
			doit(file_name);
		}
		else if(contains(file_name, ".2ds") && file_name.find("base") != std::string::npos)
		{
			Do2DS doit = {options};
			doit(file_name);
		}
		else if(contains(file_name, ".hvps") && file_name.find("base") != std::string::npos)
		{
			DoHVPS doit = {options};
			doit(file_name);
		}
		else if(contains(file_name, "2d"))
		{
			Do2DS_Reverse doit = {options};
			doit(file_name);
		}
	}
};


/**
 * -d 2ds -o Spec2d  -date_end 2011 10 31 22 3
 * -d Spec2d -o 2ds_dup -program SpecTest -platform spec -date_end 2013 10 31 22 3
 * -d 3v-cpi -asciiart -o Spec2d -date_end 2013 10 31 22 25 -minparticle 100 -maxparticle 10000
 */
int main(int argc, const char* argv[])
{
	g_Log << "Starting up\n";

	sp::CommandLine cl(argc, argv);
	sp::MakeDirectory("temp");

	{
		cl.for_each_file(ProcessFile());
		cl.for_each_directory(ProcessFile());
	}
	sp::DeleteDirectory("temp");

	g_Log <<"Shutting down\n";
}
