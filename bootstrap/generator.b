#include"tokenizer.b"

gen;

consume()
{
	auto ptr = *(gen[0]);
	auto idx = gen[1];
	auto tok = ptr[idx];
	gen[1] = idx + 1;
	return tok;
}

tryconsumeerror(tokentype,msg)
{
	auto ptr = *(gen[0]);
	auto idx = gen[1];
	auto token = ptr[idx];
	if(tokentype == token[0])
	{
		gen[1] = idx + 1;
		return token;
	}
	printf("%s\n",msg);
	exit(1);
}

tryconsume(tokentype)
{
	auto ptr = *(gen[0]);
	auto idx = gen[1];
	auto token = ptr[idx];
	if(tokentype == token[0])
	{
		gen[1] = idx + 1;
		return true;
	}
	return false;
}


peek(off)
{
	auto ptr = *(gen[0]);
	auto idx = gen[1];
	return ptr[idx+off];
}	

trypeek(tokentype,off)
{
	auto tok = peek(off);
	return tokentype == tok[0];
}


findvar(da,name)
{
	auto siz = *(da+8);
	auto ptr = *da;
	for(auto i = 0;i<siz;i++)
	{
		auto cur = ptr[i];
		if(!strcmp(cur[0],name)) return cur[1];
	}
	return -1;
}


compfunc(src)
{
	while(!tryconsume(CLOSECURLY))
	{
		if(trypeek(AUTO,0))
		{
			consume();
			if(trypeek(IDENTIFIER,0))
			{
				auto ident = consume();
				auto curr = gen[3];
				auto variable = alloc(16);
				variable[0] = *(ident[1]);
				variable[1] = curr++;
				gen[3] = curr;
				pushback(gen[2],variable);
				tryconsumeerror(SEMICOLON,"Expected ;");
				formatstr(src,"    sub rsp,8");
			}
			else 
			{
				printf("Expected identifier\n");
			}
		}
		else if(trypeek(IDENTIFIER,0))
		{
			auto lhs = consume();
			auto idx = findvar(gen[2],*(lhs[1]));
			if(idx == -1)
			{
				printf("Undeclared variable %s\n",*(lhs[1]));
				exit(1);
			}
			tryconsumeerror(ASSIGN,"Expected =");
			auto rhs = consume();
			if(rhs[0] == INTLIT)
			{
				formatstr(src,"    mov rax,%s",*(rhs[1]));
			}
			else if(rhs[0] == IDENTIFIER)
			{
				auto i2 = findvar(gen[2],*(rhs[1]));
				if(i2 == -1)
				{
					printf("Undeclared variable %s",*(rhs[1]));
					exit(1);
				}
				formatstr(src,"    mov rax,[rbp-%d]",8*(i2+1));
			}
			else 
			{
				printf("Not supported\n");
				exit(1);
			}
			formatstr(src,"    mov QWORD [rbp-%d],rax",8*(idx+1));
			tryconsumeerror(SEMICOLON,"Expected ;");
		}
		else if(tryconsume(EXTRN))
		{
			auto name = tryconsumeerror(IDENTIFIER,"Expected identifier");
			tryconsumeerror(SEMICOLON,"Expected ;");
			formatstr(src,"    extrn %s",*(name[1]));
		}
		else if(trypeek(FUNCTION,0))
		{
			auto func = consume();
			tryconsumeerror(OPENPAREN,"Expected (");
			auto rhs = consume();
			if(rhs[0] == INTLIT)
			{
				formatstr(src,"    mov rax,%s",*(rhs[1]));
			}
			else if(rhs[0] == IDENTIFIER)
			{
				auto i2 = findvar(gen[2],*(rhs[1]));
				if(i2 == -1)
				{
					printf("Undeclared variable %s",*(rhs[1]));
					exit(1);
				}
				formatstr(src,"    mov rax,[rbp-%d]",8*(i2+1));
			}
			else 
			{
				printf("Not supported\n");
				exit(1);
			}
			tryconsumeerror(CLOSEPAREN,"Expected )");
			formatstr(src,"    mov rdi,rax");
			formatstr(src,"    call %s",*(func[1]));
			tryconsumeerror(SEMICOLON,"Expected ;");
		}
		else 
		{
			printf("Unexpected statement\n");
		}
	}
}

generate(tokens)
{
	auto str = alloc(24);
	formatstr(str,"format ELF64");
	formatstr(str,"section \".text\" executable");
	gen = alloc(32);
	gen[0] = tokens;
	gen[2] = alloc(24);
	auto len = *(tokens+8);
	while(gen[1] < len)
	{
		if(trypeek(FUNCTION,0))
		{
			auto tok = consume();
			auto name =  tok[1];
			tryconsumeerror(OPENPAREN,"Expected (");
			tryconsumeerror(CLOSEPAREN,"Expected )");
			tryconsumeerror(OPENCURLY,"Expected {");
			formatstr(str,"public %s\n %s:",*name,*name);
			formatstr(str,"    push rbp");
			formatstr(str,"    mov rbp,rsp");
			compfunc(str);
			formatstr(str,"    mov rsp,rbp");
			formatstr(str,"    pop rbp");
			formatstr(str,"    xor rax,rax");
			formatstr(str,"    ret");
		}
		else
		{
			printf("expected identifier %s\n",*str);
			exit(1);
		}
	}
	return str;
}
