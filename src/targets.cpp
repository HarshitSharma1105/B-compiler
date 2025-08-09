#include"generator.cpp"


enum Targets{
	X86_64,
	MIPS
};



class Runner{
public:
	Runner(Targets target) : target(target){}
	void Compile(){
		
	}


private:
	Targets target;

};



