#include"tokenizer.b"


IrGen;
compiler;
vars;
vars_count;
extrns_arr;
data_count;
data_string;


ir_count;
FUNCALL;
AUTOASSIGN;
arg_count;
VAR;
LIT;
DATA_OFFSET;

ir_init()
{
    FUNCALL     = ir_count++;
    AUTOASSIGN  = ir_count++;





	VAR = arg_count++;
	LIT = arg_count++;
	DATA_OFFSET = arg_count++;
}

consume()
{
	auto ptr = IrGen.0.0;
	auto idx = IrGen.1++;
	return ptr[idx];
}

try_consume_error(tokentype,msg)
{
	auto ptr = IrGen.0.0;
	auto idx = IrGen.1++;
	auto token = ptr[idx];
	if(tokentype == token.0) return token;
	error(msg);
}

try_consume(tokentype)
{
	auto ptr = IrGen.0.0;
	auto idx = IrGen.1;
	auto token = ptr[idx];
	if(tokentype == token.0)
	{
		consume();
		return true;
	}
	return false;
}


peek(off=0)
{
	auto ptr = IrGen.0.0;
	auto idx = IrGen.1;
	return ptr[idx+off];
}	

try_peek(tokentype,off=0)
{
	auto tok = peek(off);
	return tokentype == tok.0;
}



find_var(name)
{
	auto siz = size(vars);
	auto ptr = vars.0;
	for(auto i = 0;i<siz;i++)
	{
		if(!strcmp(ptr[i].0,name)) return ptr[i];
	}
	return {NULL,-1};
}


extrn atoll;

compile_primary_expression()
{
	auto tok = consume();
	switch(tok.0)
	{
		case INTLIT: return {LIT,atoll(tok.1.0)};
		case IDENTIFIER:return {VAR,find_var(tok.1.0).1};
		case STRING_LIT:
		{
			auto str = tok.1.0;
			auto size = size(tok.1);
			for(auto i=0;i<size;i++)
			{
				if(read_byte(str,i)!='\')format_str_2(data_string,"%d",read_byte(str,i));
				else 
				{
					i++;
					if(read_byte(str,i)=='n')format_str_2(data_string,"10");
					else error("Unknown escape character");
					i++;
				}
				push_char(data_string,',');
			}
			push_char(data_string,'0');
			push_char(data_string,10);
			return {DATA_OFFSET,data_count++};
		}
		default : error("UNREACHABLE\n");
	}
}

compile_expression()
{
	return compile_primary_expression();
}


auto_vardec(ops)
{
	if(try_consume(AUTO))
	{
		while(try_peek(IDENTIFIER))
		{
			auto name = consume().1.0;
			if(find_var(name).1 != -1 ) error("variable already declared %s\n",name);

			push_back(vars,{name,vars_count++});
			try_consume(COMMA);
		}
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}

extrn_decl(ops)
{
	if(try_consume(EXTERN))
	{
		while(try_peek(IDENTIFIER))
		{
			auto name = consume().1.0;
			push_back(extrns_arr,name);	
			try_consume(COMMA);
		}
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}


auto_assign(ops)
{
	if(try_peek(IDENTIFIER))
	{
		auto name = consume().1.0;
		auto idx = find_var(name).1;
		if(idx == -1) error("Undeclared variable %s\n",name);
		try_consume_error(ASSIGN,"Expected =");
		push_back(ops,{AUTOASSIGN,idx,compile_expression()});
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}


comp_block(ops)
{
	if(auto_vardec(ops)) return 1;
	else if(auto_assign(ops)) return 1;
	else if(extrn_decl(ops)) return 1;
	else if(try_peek(FUNCTION))
	{
		auto func = consume().1.0;
		try_consume_error(OPEN_PAREN,"Expected (");
		auto args = alloc(24);
		while(try_consume(CLOSE_PAREN) == false)
		{
			auto arg = compile_expression();
			push_back(args,arg);
			try_consume(COMMA);
		}
		try_consume_error(SEMICOLON,"Expected ;");
		push_back(ops,{FUNCALL,func,args.0,args.1,args.2});
	}
	else 
	{
		printf("Unexpected statement\n");
	}
}

comp_func(ops)
{
	while(try_consume(CLOSE_CURLY) == false)
	{
		comp_block(ops);
	}
}


compile_prog()
{
	if(try_peek(FUNCTION))
	{
		auto name = consume().1.0;
		auto ops = alloc(24);
		try_consume_error(OPEN_PAREN,"Expected (");
		while(try_peek(IDENTIFIER))
		{
			push_back(vars,{consume().1.0,vars_count++});
			try_consume(COMMA);
		}
		auto curr = vars_count;
		try_consume_error(CLOSE_PAREN,"Expected )");
		try_consume_error(OPEN_CURLY,"Expected {");
		comp_func(ops);
		push_back(compiler,{name,ops,curr,vars_count});
		vars_count = 0;
	}
	else error("Global statements not done");
}



IrGenerate(tokens)
{
	ir_init();
	IrGen = alloc(16);
	IrGen.0 = tokens;
	vars = alloc(24);
	data_string = alloc(24);
    compiler = alloc(24);
	extrns_arr = alloc(24);
	auto len = size(tokens);
	while(IrGen.1 < len)
	{
		compile_prog();
	}
}


dbg_arg(arg)
{
	switch(arg.0)
	{
		case LIT: printf("Literal(%d)",arg.1);
		case VAR: printf("AutoVar(%d)",arg.1);
		case DATA_OFFSET: printf("DataOffset[%d]",arg.1);
		default : error("UNREACHABLE\n");
	}
}

dbg_op(op)
{
	switch(op.0)
	{
		case AUTOASSIGN:
		{
			printf("Auto Assign(%d,",op.1);
			dbg_arg(op.2);
			printf(")\n");
		}
		case FUNCALL:
		{
			printf("Function call %s(",op[1]);
			auto args = (op+16);
			auto size = size(args);
			for(auto i=0;i<size;i++)
			{
				dbg_arg(op[2][i]);
				if(i != size-1)printf(",");
			}
			printf(")\n");
		}
		default : error("UNREACHABLE");
	}
}


dbg_func(func)
{
    auto name = func.0;
    printf("Function %s(%d,%d):\n",name,func.2,func.3);
    auto size = size(func.1);
    auto ops = func.1.0;
    for(auto i = 0;i<size;i++)dbg_op(ops[i]);
    printf("Function %s end\n",name);
}

dbg_compiler(compiler)
{
    auto size = size(compiler);
    auto funcs = compiler.0;
    for(auto i=0;i<size;i++)dbg_func(funcs[i]);
	size = size(extrns_arr);
	auto base = extrns_arr.0;
	for(auto i=0;i<size;i++)printf("extrn %s\n",base[i]);
}