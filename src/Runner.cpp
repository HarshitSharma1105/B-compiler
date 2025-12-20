#include<Runner.h>

#define exec(command) system((command).c_str())
#define exec_c(command) system(command)

void debug(const Compiler& compiler)
{
	for(auto func:compiler.functions)
    {
		std::cout << "Function " <<  func.function_name << '(' << func.num_args << ',' << func.max_vars_count << "):\n";
        debug(func.function_body); 
		std::cout << "Function " << func.function_name << " end\n";
    }
	std::cout << "data:\n" << compiler.data_section << '\n';
}


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



