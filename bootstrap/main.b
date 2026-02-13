#include"generator.b"

extrn lseek,system;

siz(fd)
{
	auto siz = lseek(fd,0,2);
	lseek(fd,0,0);
	return siz;
}

read_file(path)
{
	auto fd = open(path,0);
	auto siz = siz(fd);
	auto buff = alloc(siz+1);
	read(fd,buff,siz);
	return buff;
}


preprocess(path,buff)
{
	extrn realpath,dirname;
	auto final_code = alloc(24);
	auto parent_path = string_from_cstr(dirname(realpath(path,NULL)));
	auto rel_path = alloc(24);
	auto idx = 0;
	while(read_byte(buff,idx)=='#')
	{
		idx++;
		while(read_byte(buff,idx) != '"')
		{
			if(!isspace(read_byte(buff,idx)))push_char(rel_path,read_byte(buff,idx));
			idx++;
		}
		if(strcmp(rel_path.0,"include"))error("Wrong declaration");
		rel_path.1 = rel_path.2 = 0;
		idx++;
		push_char(rel_path,'/');
		while(read_byte(buff,idx)!='"')push_char(rel_path,read_byte(buff,idx++));
		idx++;
		auto temp_path = string_from_cstr(parent_path.0);
		push_str(temp_path,rel_path.0);
		push_str(final_code,preprocess(temp_path.0,read_file(temp_path.0)));
		while(isspace(read_byte(buff,idx)))idx++;
	} 
	while(read_byte(buff,idx))push_char(final_code,read_byte(buff,idx++));
	return final_code.0;
}


main(argc,argv)
{
	if(argc < 2) error("Incorrect Usage");
	auto path = argv[1];

	auto buff = read_file(path);
	auto code = preprocess(path,buff);

	auto tokens = tokenize(code);

	IrGenerate(tokens);

	generate();

	auto file = open("output.asm",64 | 1,420);
	write(file,asm_str.0,asm_str.1);

	system("fasm output.asm");
	system("cc -no-pie output.o -o output");
	system("./output");
	system("rm output*");
} 
