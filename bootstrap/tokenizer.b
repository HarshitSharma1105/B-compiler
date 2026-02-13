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
STRING_LIT;
ADD;
SUB;
MULT;
DIV;
REMAINDER;
RETURN;
INCR;
DECR;
WHILE;
GREATER;
LESS;
OPEN_SQUARE;
CLOSE_SQUARE;
DOT;
EQUALS;
NOT_EQUALS;
NOT;
IF;
ELSE;
ATTRIB;
DECL;
ASM;

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
	STRING_LIT  = tok_count++;
	ADD			= tok_count++;
	SUB			= tok_count++;
	MULT		= tok_count++;
	DIV			= tok_count++;
	REMAINDER	= tok_count++;
	RETURN 		= tok_count++;
	INCR		= tok_count++;
	DECR		= tok_count++;
	WHILE 		= tok_count++;
	OPEN_SQUARE	= tok_count++;
	CLOSE_SQUARE= tok_count++;
	DOT			= tok_count++;
	GREATER		= tok_count++;
	LESS		= tok_count++;
	EQUALS		= tok_count++;
	NOT_EQUALS	= tok_count++;
	NOT			= tok_count++;
	IF			= tok_count++;
	ELSE		= tok_count++;
	ATTRIB		= tok_count++;
	DECL		= tok_count++;
}

tokenize(src)
{
	tok_init();
	auto tokens = alloc(24);
	auto idx = 0;
	auto len = strlen(src);
	while(idx < len)
	{
		auto ch = read_byte(src,idx);
		if(isspace(ch))idx++;
		else if(ch == 10)idx++;
		else if(ch == '(')
		{
			auto last = *(back(tokens));
			if(last.0 == IDENTIFIER) last.0 = FUNCTION;
			push_back(tokens,{OPEN_PAREN,NULL});
			idx++;
		}
		else if(ch == ')')
		{
			push_back(tokens,{CLOSE_PAREN,NULL});
			idx++;
		}
		else if(ch == '{')
		{
			push_back(tokens,{OPEN_CURLY,NULL});
			idx++;
		}
		else if(ch == '}')
		{
			push_back(tokens,{CLOSE_CURLY,NULL});
			idx++;
		}
		else if(ch=='+')
		{
			if(read_byte(src,idx+1)=='+')
			{
				push_back(tokens,{INCR,NULL});
				idx++;
			}
			else push_back(tokens,{ADD,NULL});
			idx++;
		}
		else if(ch=='<')
		{
			push_back(tokens,{LESS,NULL});
			idx++;
		}
		else if(ch=='>')
		{
			push_back(tokens,{GREATER,NULL});
			idx++;
		}
		else if(ch=='-')
		{
			if(read_byte(src,idx+1)=='-')
			{
				push_back(tokens,{DECR,NULL});
				idx++;
			}
			else push_back(tokens,{SUB,NULL});
			idx++;
		}
		else if (ch == ',')
		{
			push_back(tokens,{COMMA,NULL});
			idx++;
		}
		else if(ch == '"')
		{
			idx++;
			auto buff = alloc(24);
			while(read_byte(src,idx)!='"')push_char(buff,read_byte(src,idx++));
			idx++;
			push_back(tokens,{STRING_LIT,buff});
		}
		else if(ch=='*')
		{
			push_back(tokens,{MULT,NULL});
			idx++;
		}
		else if(ch=='[')
		{
			if(read_byte(src,idx+1)=='[')
			{
				idx = idx + 2;
				auto buff = alloc(24);
				while(isalnum(read_byte(src,idx)))push_char(buff,read_byte(src,idx++));
				idx = idx + 2;
				push_back(tokens,{ATTRIB,buff});
			}
			else
			{
				push_back(tokens,{OPEN_SQUARE,NULL});
				idx++;
			}
		}
		else if(ch==']')
		{
			push_back(tokens,{CLOSE_SQUARE,NULL});
			idx++;
		}
		else if(ch=='.')
		{
			push_back(tokens,{DOT,NULL});
			idx++;
		}
		else if(ch=='/')
		{
			push_back(tokens,{DIV,NULL});
			idx++;
		}
		else if(ch=='%')
		{
			push_back(tokens,{REMAINDER,NULL});
			idx++;
		}
		else if(isdigit(ch))
		{
			auto buff = alloc(24);
			while(isdigit(read_byte(src,idx)))push_char(buff,read_byte(src,idx++));
			push_back(tokens,{INTLIT,buff});
		}
		else if(isalnum(ch) | ch == '_')
		{
			auto buff = alloc(24);
			while(isalnum(read_byte(src,idx)) | read_byte(src,idx) == '_')push_char(buff,read_byte(src,idx++));
			if(!strcmp(buff.0,"auto"))push_back(tokens,{AUTO,NULL});
			else if(!strcmp(buff.0,"extrn"))push_back(tokens,{EXTERN,NULL});
			else if(!strcmp(buff.0,"return"))push_back(tokens,{RETURN,NULL});
			else if(!strcmp(buff.0,"while"))push_back(tokens,{WHILE,NULL});
			else if(!strcmp(buff.0,"if"))push_back(tokens,{IF,NULL});
			else if(!strcmp(buff.0,"else"))push_back(tokens,{ELSE,NULL});
			else if(!strcmp(buff.0,"decl"))push_back(tokens,{DECL,NULL});
			else if(!strcmp(buff.0,"asm"))push_back(tokens,{ASM,NULL});
			else push_back(tokens,{IDENTIFIER,buff});
		}
		else if(ch == '=')
		{
			if(read_byte(src,idx+1)=='=')
			{
				push_back(tokens,{EQUALS,NULL});
				idx++;
			}
			else push_back(tokens,{ASSIGN,NULL});
			idx++;
		}
		else if(ch=='!')
		{
			if(read_byte(src,idx+1)=='=')
			{
				push_back(tokens,{NOT_EQUALS,NULL});
				idx++;
			}
			else push_back(tokens,{NOT,NULL});
			idx++;
		}
		else if(ch == ';')
		{
			push_back(tokens,{SEMICOLON,NULL});
			idx++;
		}
		else error("Unrecognized token %c",ch);
	}
	return tokens;
}

debug(token)
{
	auto type = token.0;
	switch(token.0)
	{
		case OPEN_PAREN:printf("open-paren (\n");
		case CLOSE_PAREN: printf("close-paren )\n");
		case OPEN_CURLY: printf("open-brace {\n");
		case CLOSE_CURLY: printf("close-brace }\n");
		case INTLIT: printf("Int-lit %s\n",token.1.0);
		case AUTO: printf("auto\n");
		case IDENTIFIER: printf("Identifier %s\n",token.1.0);
		case FUNCTION: printf("Function %s\n",token.1.0);
		case EXTERN: printf("Extrn\n");
		case ASSIGN: printf("assignment \n");
		case SEMICOLON: printf("semicolon ;\n");
		case COMMA: printf("comma ,\n");
		case STRING_LIT:  printf("string lit %s\n",token.1.0);
		case ADD : 		  printf("add\n");
		case SUB : 		  printf("sub\n");
		case INCR : 	  printf("Incr\n");
		case DECR : 	  printf("Decr\n");
		case WHILE:		  printf("While");
		case GREATER:	  printf("Greater\n");
		case LESS:		  printf("Less\n");
		case DOT:		  printf("Dot\n");
		case OPEN_SQUARE: printf("Open Square\n");
		case CLOSE_SQUARE:printf("Close Sqaure\n");
		default : printf("Unknown token\n");
	}
}


debug_tokens(tokens)
{
	auto base = tokens.0,len = tokens.1;
	for(auto i=0;i<len;i++)
	{
		debug(base[i]);
	}
}