#include"tokenizer.b"




main(argc,argv)
{
	if(argc < 2)
	{
		printf("Incorrect usage\n");
		exit(1);
	}
	auto fd = open(argv[1],0);
	auto siz = 1024;
	auto buff = malloc(siz);
	read(fd,buff,siz);
	auto tokens = tokenize(buff);
	auto base = *tokens,len = *(tokens+8);
	for(auto i=0;i<len;i++)
	{
		debug(base[i]);
	}

}
