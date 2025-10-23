#include<Runner.h>






Runner::Runner(std::string target_lang,const std::string& path) : path(path)
{
	if(target_lang=="x86_64")
	{
		target=Target::X86_64;
	}
	else if(target_lang=="Mips")
	{
		target=Target::MIPS;
	}
	else 
	{
		std::cerr << "Unsupported Target Language " << target_lang << " Please provide valid target for the compiler\n";
		exit(EXIT_FAILURE);
	}
}
void Runner::compile(const std::vector<Op>& ops)
{
	std::string assembly_sourcecode;
	if(target==Target::MIPS)
	{
		Generator_Mips generator(ops);
		assembly_sourcecode=generator.generate();
	}
	else if(target==Target::X86_64)
	{
		Generator_x86_64 generator(ops);
		assembly_sourcecode=generator.generate();
	}
	std::string command = "mkdir " + path+"/trash";//TODO: Use smth like mkdir_if_not_exist
	system(command.c_str());
	{
		std::ofstream outFile(path+ "/trash/output.asm");  
		outFile << assembly_sourcecode;
	}
}


void Runner::run()
{
	
	if(target==Target::MIPS)
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



