#include<Runner.h>

#define exec(command) system((command).c_str())
#define exec_c(command) system(command)



Runner::Runner(std::string target_lang,const std::string& path) : path(path)
{
	if(target_lang=="x86_64")
	{
		target=Target::X86_64;
	}
	else if(target_lang=="mips")
	{
		target=Target::MIPS;
	}
	else errorf("Unsupported Target Language {}. Please provide valid target for the compiler",target_lang);
}
void Runner::compile(const Compiler& compiler)
{
	std::string assembly_sourcecode;
	if(target==Target::MIPS)
	{
		Generator_Mips generator(compiler);
		assembly_sourcecode=generator.generate();
	}
	else if(target==Target::X86_64)
	{
		Generator_x86_64 generator(compiler);
		assembly_sourcecode=generator.generate();
	}
	exec("mkdir " + path+"/trash");
	{
		std::ofstream outFile(path+ "/trash/output.asm");  
		outFile << assembly_sourcecode;
	}
}


void Runner::run()
{
	
	if(target==Target::MIPS)
	{
		exec("java -jar assemblers/Mars4_5.jar sm " + path +"/trash/output.asm");
	}
	else 
	{
		exec("assemblers/fasm "+ path +"/trash/output.asm");
		exec("cc -no-pie " + path +"/trash/output.o -l:raylib/libraylib.so -o builds/output");
//		exec_c("LD_LIBRARY_PATH=\"/usr/lib/raylib\"  builds/output");
	}
}



