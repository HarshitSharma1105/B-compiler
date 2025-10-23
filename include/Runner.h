#pragma once


#include<Generator-Mips.h>
#include<Generator-x86_64.h>


enum Target{
	X86_64,
	MIPS
};



class Runner
{
public:
	Runner(std::string target_lang,const std::string& path);
	void compile(const std::vector<Op>& ops);
	void run();

private:
	Target target;
	std::string path,command;
};



