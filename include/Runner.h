#pragma once


#include<Generator-Mips.h>
#include<Generator-x86_64.h>


enum Target{
	X86_64,
	MIPS
};


void debug(const Compiler& compiler);

class Runner
{
public:
	Runner(std::string target_lang,const std::string& path);
	void compile(const Compiler& compiler);
	void run();

private:
	Target target;
	std::string path,command;
};



