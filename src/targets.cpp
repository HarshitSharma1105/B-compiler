#include"generator.cpp"


enum Targets{
	X86_64,
	MIPS
};



class Runner{
public:
	Runner(std::string target_lang,const std::string& path) : target(target),path(path)
	{
		if(target_lang=="x86_64")
		{
			target=Targets::X86_64;
		}
		else if(target_lang=="Mips")
		{
			target=Targets::MIPS;
		}
		else 
		{
			std::cerr << "unsupported Target Language . Please provide valid target for the compiler\n";
			exit(EXIT_FAILURE);
		}
	}
	void compile(const std::vector<Op>& ops)
	{
		if(target==Targets::MIPS)
		{
			Generator_Mips generator(ops);
			std::string assembly_sourcecode=generator.generate();
			std::string command = "mkdir " + path+"/trash";//TODO: Use smth like mkdir_if_not_exist
			system(command.c_str());
			{
				std::ofstream outFile(path+ "/trash/output.asm");  
				outFile << assembly_sourcecode;
			}
			
		}
		else 
		{
			Generator_x86_64 generator(ops);
			std::string assembly_sourcecode=generator.generate();
			std::string command = "mkdir " + path+"/trash";//TODO: Use smth like mkdir_if_not_exist
			system(command.c_str());
			{
				std::ofstream outFile(path+ "/trash/output.asm");  
				outFile << assembly_sourcecode;
			}
		}
	}


	void run()
	{
		
		if(target==Targets::MIPS)
		{
			command="java -jar assemblers/Mars4_5.jar sm " + path +"/trash/output.asm";
    		system(command.c_str());
		}
		else 
		{
			command = "assemblers/fasm " + path + "/trash/output.asm";
			system(command.c_str());
			command = "cc -no-pie " + path +"/trash/output.o -l:raylib/libraylib.so -o "+"builds/output";
			system(command.c_str());
			command = "LD_LIBRARY_PATH=\"/usr/lib/raylib\"    builds/output";
			system(command.c_str());
		}
	}

private:
	Targets target;
	std::string path,command;
};



