#include"generator.b"

extrn lseek,system;

siz(fd)
{
	auto siz = lseek(fd,0,2);
	lseek(fd,0,0);
	return siz;
}

main(argc,argv)
{
	if(argc < 2) error("Incorrect Usage");
	auto fd = open(argv[1],0);

	auto siz = siz(fd);

	auto buff = alloc(siz+1);

	read(fd,buff,siz);

	auto tokens = tokenize(buff);

	IrGenerate(tokens);

	generate();

	auto file = open("output.asm",64 | 1,420);
	write(file,asm_str.0,asm_str.1);

	system("fasm output.asm");
	system("cc -no-pie output.o -o output");
	system("./output");
	system("rm output*");
} 
