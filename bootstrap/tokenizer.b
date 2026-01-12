#include"stdb.b"

extrn isdigit,isspace,isalnum;
extrn strlen,strcmp;


tok_count;


OPEN_PAREN;
CLOSE_PAREN;
OPEN_CURLY;
CLOSE_CURLY;
INTLIT;
IDENTIFIER;
AUTO;
EXTERN;
ASSIGN;
SEMICOLON;
FUNCTION;
COMMA;





tok_init()
{
	OPEN_PAREN  = tok_count++;
	CLOSE_PAREN = tok_count++;
	OPEN_CURLY  = tok_count++;
	CLOSE_CURLY = tok_count++;
	INTLIT      = tok_count++;
	AUTO        = tok_count++;
	EXTERN      = tok_count++;
	IDENTIFIER  = tok_count++;
	ASSIGN      = tok_count++;
	SEMICOLON   = tok_count++;
	FUNCTION    = tok_count++;
	COMMA       = tok_count++;
}

tokenize(src)
{
	tok_init();
	auto tokens = alloc(24);
	auto idx = 0;
	auto len = strlen(src);
	while(idx < len)
	{
		auto tok = alloc(16);
		auto ch = read_byte(src,idx);
		if(isspace(ch))idx++;
		else if(ch == 10)idx++;
		else if(ch == '(')
		{
			tok.0 = OPEN_PAREN;
			auto last = *(back(tokens));
			if(last.0 == IDENTIFIER) last.0 = FUNCTION;
			push_back(tokens,tok);
			idx++;
		}
		else if(ch == ')')
		{
			tok.0 = CLOSE_PAREN;
			push_back(tokens,tok);
			idx++;
		}
		else if(ch == '{')
		{
			tok.0 = OPEN_CURLY;
			push_back(tokens,tok);
			idx++;
		}
		else if(ch == '}')
		{
			tok.0 = CLOSE_CURLY;
			push_back(tokens,tok);
			idx++;
		}
		else if (ch == ',')
		{
			tok.0 = COMMA;
			push_back(tokens,tok);
			idx++;
		}
		else if(isdigit(ch))
		{
			auto buff = alloc(24);
							
			while(isdigit(read_byte(src,idx)))push_char(buff,read_byte(src,idx++));
			tok.0 = INTLIT;
			tok.1 = buff;
			push_back(tokens,tok);
		}
		else if(isalnum(ch))
		{
			auto buff = alloc(24);
			tok.1 = buff;
			while(isalnum(read_byte(src,idx)))push_char(buff,read_byte(src,idx++));
			if(!strcmp(*buff,"auto"))tok.0 = AUTO;
			else if(!strcmp(*buff,"extrn"))tok.0 = EXTERN;
			else tok.0 = IDENTIFIER;
			push_back(tokens,tok);
		}
		else if(ch == '=')
		{
			tok.0 = ASSIGN;
			push_back(tokens,tok);
			idx++;
		}
		else if(ch == ';')
		{
			tok.0 = SEMICOLON;
			push_back(tokens,tok);
			idx++;
		}
		else 
		{
			printf("Unrecognized token: ");
			for(auto i = 0;i<10;i++)printf("%c",read_byte(src,idx+i));
			printf("\n");
			exit(1);
		}
	}
	return tokens;
}

debug(token)
{
	auto type = token.0;
	if(type == OPEN_PAREN)printf("open-curly (\n");
	else if(type == CLOSE_PAREN)printf("close-curly )\n");
	else if(type == OPEN_CURLY)printf("open-brace {\n");
	else if(type == CLOSE_CURLY)printf("close-brace }\n");
	else if(type == INTLIT)printf("Int-lit %s\n",token.1.0);
	else if(type == AUTO)printf("auto\n");
	else if(type == IDENTIFIER)printf("Identifier %s\n",token.1.0);
	else if(type == FUNCTION)printf("Function %s\n",token.1.0);
	else if(type == EXTERN )printf("Extrn\n");
	else if(type == ASSIGN)printf("assignment =\n");
	else if(type == SEMICOLON)printf("semicolon ;\n");
	else if(type == COMMA)printf("comma ,\n");
}


debug_tokens(tokens)
{
	auto base = *tokens,len = *(tokens+8);
	for(auto i=0;i<len;i++)
	{
		debug(base[i]);
	}
}