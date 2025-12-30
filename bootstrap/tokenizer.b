#include"stdb.b"

extrn printf,puts;
extrn read,open,exit;
extrn isdigit,isspace,isalnum;
extrn strlen,strcmp;


count;


OPENPAREN;
CLOSEPAREN;
OPENCURLY;
CLOSECURLY;
INTLIT;
IDENTIFIER;
AUTO;
EXTRN;
ASSIGN;
SEMICOLON;
FUNCTION;


init()
{
	OPENPAREN  = count++;
	CLOSEPAREN = count++;
	OPENCURLY  = count++;
	CLOSECURLY = count++;
	INTLIT     = count++;
	AUTO       = count++;
	EXTRN      = count++;
	IDENTIFIER = count++;
	ASSIGN     = count++;
	SEMICOLON  = count++;
	FUNCTION   = count++;
}

tokenize(src)
{
	init();
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
			tok[0] = OPENPAREN;
			auto last = *(back(tokens));
			if(last[0] == IDENTIFIER) last[0] = FUNCTION;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == ')')
		{
			auto tok = alloc(16);
			tok[0] = CLOSEPAREN;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == '{')
		{
			auto tok = alloc(16);
			tok[0] = OPENCURLY;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == '}')
		{
			auto tok = alloc(16);
			tok[0] = CLOSECURLY;
			pushback(tokens,tok);
			idx++;
		}
		else if(isdigit(ch))
		{
			auto tok = alloc(16);
			auto buff = alloc(24);
							
			while(isdigit(readbyte(src,idx)))pushchar(buff,readbyte(src,idx++));				
			tok[0] = INTLIT;
			tok[1] = buff;
			pushback(tokens,tok);
		}
		else if(isalnum(ch))
		{
			auto tok  = alloc(16);
			auto buff = alloc(24);
			tok[1] = buff;
			while(isalnum(readbyte(src,idx)))pushchar(buff,readbyte(src,idx++));
			if(!strcmp(*buff,"auto"))tok[0] = AUTO;
			else if(!strcmp(*buff,"extrn"))tok[0] = EXTRN;
			else tok[0] = IDENTIFIER;
			pushback(tokens,tok);
		}
		else if(ch == '=')
		{
			auto tok = alloc(16);
			tok[0] = ASSIGN;
			pushback(tokens,tok);
			idx++;
		}
		else if(ch == ';')
		{
			auto tok = alloc(16);
			tok[0] = SEMICOLON;
			pushback(tokens,tok);
			idx++;
		}
		else 
		{
			printf("Unrecognized token: ");
			for(auto i = 0;i<10;i++)printf("%c",readbyte(src,idx+i));
			printf("\n");
			exit(1);
		}
	}
	return tokens;
}

debug(token)
{
	auto type = token[0];
	if(type == OPENPAREN)printf("open-curly (\n");
	else if(type == CLOSEPAREN)printf("close-curly )\n");
	else if(type == OPENCURLY)printf("open-brace {\n");
	else if(type == CLOSECURLY)printf("close-brace }\n");
	else if(type == INTLIT)printf("Int-lit %s\n",*(token[1]));
	else if(type == AUTO)printf("auto\n");
	else if(type == IDENTIFIER)printf("Identifier %s\n",*(token[1]));
	else if(type == FUNCTION)printf("Function %s\n",*(token[1]));
	else if(type == EXTRN )printf("Extrn\n");
	else if(type == ASSIGN)printf("assignment =\n");
	else if(type == SEMICOLON)printf("semicolon ;\n");
}
