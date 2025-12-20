#include"stdb.b"

extrn malloc,memset,free;
extrn printf,puts,putchar,read,open,exit;
extrn isdigit,isspace,isalnum;
extrn strlen;

tokenize(src)
{
	auto tokens = alloc(24);
	auto idx = 0;
	auto len = strlen(src);
	while(idx < len)
	{
		auto ch = readbyte(src,idx);
		if(isspace(ch))idx++;
		else if(ch == 10)idx++;
		else if(ch == '(')
		{
			auto tok = alloc(16);
			tok[0] = 0;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == ')')
		{
			auto tok = alloc(16);
			tok[0] = 1;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == '{')
		{
			auto tok = alloc(16);
			tok[0] = 2;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == '}')
		{
			auto tok = alloc(16);
			tok[0] = 3;
			pushback(tokens,tok);
			idx++;
		}
		else if(isdigit(ch))
		{
			auto tok = alloc(16);
			auto buff = alloc(24);
							
			while(isdigit(readbyte(src,idx)))pushstr(buff,readbyte(src,idx++));				
			tok[0] = 4;
			tok[1] = buff;
			pushback(tokens,tok);
		}
		else if(isalnum(ch))
		{
			auto tok = alloc(16);
			auto buff = alloc(24);
			tok[0] = 5;
			tok[1] = buff;
			while(isalnum(readbyte(src,idx)))pushstr(buff,readbyte(src,idx++));
			pushback(tokens,tok);
		}
		else if(ch == '=')
		{
			auto tok = alloc(16);
			tok[0] = 6;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == ';')
		{
			auto tok = alloc(16);
			tok[0] = 7;
			pushback(tokens,tok);
			idx++;
		}
		else 
		{
			puts("Unrecognized token\n");
			for(auto i =0;i<10;i++)printf("%d ",readbyte(src,idx+i));
			exit(1);
		}
	}
	return tokens;
}

debug(token)
{
	auto type = token[0];
	if(type == 0)puts("(");
	else if(type == 1)puts(")");
	else if(type == 2)puts("{");
	else if(type == 3)puts("}");
	else if(type == 4)printf("int-lit %s\n",*(token[1]));
	else if(type == 5)printf("Identifier %s\n",*(token[1]));
	else if(type == 6)puts("=");
	else if(type == 7)puts(";");
}
