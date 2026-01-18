#include"tokenizer.b"


IrGen;
compiler;
vars;
vars_count;
extrns_arr;



ir_count;
FUNCALL;
AUTOASSIGN;
arg_count;
VAR;
LIT;


ir_init()
{
    FUNCALL     = ir_count++;
    AUTOASSIGN  = ir_count++;





	VAR = arg_count++;
	LIT = arg_count++;
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
	printf("%s\n",msg);
	exit(1);
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

tok_name(tok) return tok.1.0;


find_var(name)
{
	auto siz = size(vars);
	auto ptr = vars.0;
	for(auto i = 0;i<siz;i++)
	{
		auto cur = ptr[i];
		if(!strcmp(cur.0,name)) return cur.1;
	}
	return -1;
}


extrn atoll;

compile_primary_expression()
{
	auto tok = consume();
	if(tok.0 == INTLIT)return {LIT,atoll(tok.1.0)};
	else if(tok.0 == IDENTIFIER)return {VAR,find_var(tok.1.0)};
	else error("UNREACHABLE\n");
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
			auto name = tok_name(consume());
			if(find_var(name) != -1 )
			{
				error("variable already declared %s\n");
			}
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
			auto name = tok_name(consume());
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
		auto name = tok_name(consume());
		auto idx = find_var(name);
		if(idx == -1)
		{
			printf("Undeclared variable %s\n",name);
			for(auto i=-5;i<5;i++)debug(peek(i));
			exit(1);
		}
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
		auto func = tok_name(consume());
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
		auto name = tok_name(consume());
		auto ops = alloc(24);
		try_consume_error(OPEN_PAREN,"Expected (");
		while(try_peek(IDENTIFIER))
		{
			push_back(vars,{tok_name(consume()),vars_count++});
			try_consume(COMMA);
		}
		auto curr = vars_count;
		try_consume_error(CLOSE_PAREN,"Expected )");
		try_consume_error(OPEN_CURLY,"Expected {");
		comp_func(ops);
		push_back(compiler,{name,ops,curr,vars_count});
		vars_count = 0;
	}
	else
	{
		printf("Global statements not done\n");
		debug(peek());
		exit(1);
	}
}



IrGenerate(tokens)
{
	ir_init();
	IrGen = alloc(16);
	IrGen.0 = tokens;
	vars = alloc(24);
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
	if(arg.0 == LIT) printf("Literal(%d)",arg.1);
	else if(arg.0 == VAR) printf("AutoVar(%d)",arg.1);
	else error("UNREACHABLES\n");
}

dbg_op(op)
{
	auto type = op.0;
	if(type == AUTOASSIGN)
	{
		printf("Auto Assign(%d,",op.1);
		dbg_arg(op.2);
		printf(")\n");
	}
	else if(type == FUNCALL)
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
	for(auto i=0;i<size;i++)printf("%s\n",base[i]);
}