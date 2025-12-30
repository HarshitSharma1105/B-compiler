#include"generator.b"

extrn lseek,write,system;

siz(fd)
{
	auto siz = lseek(fd,0,2);
	lseek(fd,0,0);
	return siz;
}

main(argc,argv)
{
	if(argc < 2)
	{
		printf("Incorrect usage\n");
		exit(1);
	}
	auto fd = open(argv[1],0);
	auto siz = siz(fd);
	auto buff = alloc(siz+1);
	read(fd,buff,siz);
	//printf("%s\n",buff);
	auto tokens = tokenize(buff);
	/*
	auto base = *tokens,len = *(tokens+8);
	for(auto i=0;i<len;i++)
	{
		debug(base[i]);
	}
	*/
	auto str = generate(tokens);
	//printf("%s",*str);
	auto file = open("output.asm",64 | 1,420);
	write(file,*str,*(str+8));
	system("fasm output.asm");
	system("cc -no-pie output.o -o output");
	system("./output");
	system("rm output*");
}
