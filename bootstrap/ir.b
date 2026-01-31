#include"tokenizer.b"


IrGen;
compiler;
vars;
vars_count;
max_vars_count;
extrns_arr;
data_count;
data_string;
labels_count;

ir_count;
FUNCALL;
BINOP;
UNOP;
RETURNVALUE;
LABEL;
JMP;
JMPIFZERO;


arg_count;
VAR;
LIT;
DATA_OFFSET;
FUNC_RESULT;
NO_ARG;

precedences;

ir_init()
{
	precedences = alloc(24); 
	push_back(precedences,{{ASSIGN},1});
	push_back(precedences,{{LESS},1});
	push_back(precedences,{{ADD,SUB},2});
	push_back(precedences,{{DIV,MULT,REMAINDER},3});


    FUNCALL     = ir_count++;
	BINOP		= ir_count++;
	UNOP		= ir_count++;
	RETURNVALUE = ir_count++;
	JMP			= ir_count++;
	JMPIFZERO	= ir_count++;
	LABEL		= ir_count++;


	VAR 		= arg_count++;
	LIT 		= arg_count++;
	DATA_OFFSET = arg_count++;
	FUNC_RESULT = arg_count++;
	NO_ARG		= arg_count++;
}

dbg_arg(arg)
{
	switch(arg.0)
	{
		case LIT: printf("Literal(%d)",arg.1);
		case VAR: printf("AutoVar(%d)",arg.1);
		case DATA_OFFSET: printf("DataOffset[%d]",arg.1);
		case FUNC_RESULT: printf("Func Result %s",arg.1);
		case NO_ARG		: printf("No Arg");
		default : error("UNREACHABLE");
	}
}

dbg_op(op)
{
	switch(op.0)
	{
		case FUNCALL:
		{
			printf("Function call %s(",op[1]);
			auto args = (op+16);
			auto size = args.1;
			for(auto i=0;i<size;i++)
			{
				dbg_arg(op.2[i]);
				if(i != size-1)printf(",");
			}
			printf(")\n");
		}
		case BINOP :
		{
			printf("BinOp AutoVar(%d)",op.1);
			printf(",lhs=");
			dbg_arg(op.2);
			printf(",rhs=");
			dbg_arg(op.3);
			printf(",type=");
			debug({op.4,NULL});
		}
		case UNOP:
		{
			printf("Unop Autovar(%d) Arg = ",op.1);
			dbg_arg(op.2);
			printf(",type =");
			debug({op.3,NULL});
		}
		case RETURNVALUE:
		{
			printf("Return Value ");
			dbg_arg(op.1);
			printf("\n");
		}
		case LABEL:
		{
			printf("Label (%d)\n",op.1);
		}
		case JMP:
		{
			printf("Branch Label (%d)\n",op.1);
		}
		case JMPIFZERO:
		{
			printf("Branch Label (%d)(",op.1);
			dbg_arg(op.2);
			printf(")\n");
		}
		default : error("UNREACHABLE");
	}
}


dbg_func(func)
{
    auto name = func.0;
    printf("Function %s(%d,%d):\n",name,func.2,func.3);
    auto size = func.1.1;
    auto ops = func.1.0;
    for(auto i = 0;i<size;i++)dbg_op(ops[i]);
    printf("Function %s end\n",name);
}

dbg_compiler(compiler)
{
    auto size = compiler.1;
    auto funcs = compiler.0;
    for(auto i=0;i<size;i++)dbg_func(funcs[i]);
	size = extrns_arr.1;
	auto base = extrns_arr.0;
	for(auto i=0;i<size;i++)printf("extrn %s\n",base[i]);
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
	auto type = peek(off).0;
	return tokentype == type;
}


try_peek_vec(tokentypes,off=0)
{
	auto type = peek(off).0;
	auto size = tokentypes.1;
	for(auto i=0;i<size;i++)
	{
		if(type == tokentypes.0[i])return true;
	}
	return false;
}


find_var(name)
{
	auto siz = vars.1;
	auto ptr = vars.0;
	for(auto i = 0;i<siz;i++)
	{
		if(!strcmp(ptr[i].0,name)) return ptr[i];
	}
	return {NULL,-1};
}


extrn atoll;

decl compile_expression; 


compile_primary_expression(ops)
{
	auto tok = consume();
	switch(tok.0)
	{
		case IDENTIFIER:
		{
			auto idx = find_var(tok.1.0).1;
			if(idx==-1)error("Undeclared variable %s",tok.1.0);
			return {VAR,idx};
		}
		case INTLIT: return {LIT,atoll(tok.1.0)};
		case STRING_LIT:
		{
			auto str = tok.1.0;
			auto size = tok.1.1;
			for(auto i=0;i<size;i++)
			{
				if(read_byte(str,i)!='\')format_str_2(data_string,"%d",read_byte(str,i));
				else 
				{
					i++;
					if(read_byte(str,i)=='n')format_str_2(data_string,"10");
					else if(read_byte(str,i)=='t')format_str_2(data_string,"9");
					else error("Unknown escape character");
					i++;
				}
				push_char(data_string,',');
			}
			push_char(data_string,'0');
			push_char(data_string,10);
			return {DATA_OFFSET,data_count++};
		}
		case FUNCTION:
		{
			auto func = tok.1.0;
			try_consume_error(OPEN_PAREN,"Expected (");
			auto args = alloc(24);
			while(try_consume(CLOSE_PAREN) == false)
			{
				auto arg = compile_expression(0,ops);
				push_back(args,arg);
				try_consume(COMMA);
			}
			push_back(ops,{FUNCALL,func,args.0,args.1,args.2});
			push_back(ops,{BINOP,vars_count,{NO_ARG},{FUNC_RESULT,func},ASSIGN});
			return {VAR,vars_count++};
		}
		case SUB:
		{
			auto arg = compile_primary_expression(ops);
			push_back(ops,{UNOP,vars_count,arg,SUB});
			return {VAR,vars_count++};
		}
		case OPEN_PAREN:
		{
			auto arg = compile_expression(0,ops);
			try_consume_error(CLOSE_PAREN,"Expected )");
			return arg;
		}
		case INCR:
		{
			auto ident = try_consume_error(IDENTIFIER,"Only identifier allowed after pre incr");
			auto idx = find_var(ident.1.0).1;
			if(idx==-1)error("Undeclared variable %s",ident.1.0);
			push_back(ops,{BINOP,idx,{VAR,idx},{LIT,1},ADD});
			return {VAR,idx};
		}
		default : error("UNREACHABLE");
	}
}

conv(type)
{
	switch(type)
	{
		case INCR:return ADD;
		case DECR:return SUB;
		default: error("UNREACHABLE");
	}
}

compile_prim_expression(ops)
{
	auto ret = compile_primary_expression(ops);
	auto temp;
	if(try_peek_vec({{INCR,DECR},2}))
	{
		if(ret.0 == VAR)
		{
			auto type = consume().0;
			temp = {VAR,vars_count++};
			push_back(ops,{BINOP,temp.1,{NO_ARG},ret,ASSIGN});
			push_back(ops,{BINOP,ret.1,temp,{LIT,1},conv(type)});
			ret = temp;
		}
		else error("TODO REFERENCES FOR FUTURE");
	}
	return ret;
}



compile_expression(prec,ops)
{
	if(prec==precedences.1)return compile_prim_expression(ops);
	auto lhs = compile_expression(prec+1,ops),rhs;
	while(try_peek_vec(precedences.0[prec]))
	{
		auto type = consume().0;
		if(prec==0)
		{
			rhs = compile_expression(0,ops);
			push_back(ops,{BINOP,lhs.1,{NO_ARG},rhs,type});
		}
		else
		{
			rhs = compile_expression(prec+1,ops);
			push_back(ops,{BINOP,vars_count,lhs,rhs,type});
			lhs = {VAR,vars_count++};
		}
	}
	return lhs;
}


auto_vardec(ops)
{
	if(try_consume(AUTO))
	{
		while(try_peek(IDENTIFIER))
		{
			auto name = peek().1.0;
			if(find_var(name).1 != -1 ) error("variable already declared %s",name);
			push_back(vars,{name,vars_count++});
			compile_expression(0,ops);
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

decl comp_block;
compile_while(ops)
{
	if(try_consume(WHILE))
	{
		push_back(ops,{LABEL,labels_count});
		auto start = labels_count++;
		auto curr_vars  = vars_count;
		auto vars_size = vars.1;
		auto arg = compile_expression(0,ops);
		auto curr = ops.1;
		push_back(ops,{JMPIFZERO,arg,0});
		comp_block(ops);
		push_back(ops,{JMP,start});
		push_back(ops,{LABEL,labels_count});
		ops.0[curr].2 = labels_count++;
		resize(vars,vars_size);
		max_vars_count = max(max_vars_count,vars_count);
		vars_count = curr_vars;
		return true;
	}
	return false;
}

decl comp_func_body;
compile_scope(ops)
{
	if(try_consume(OPEN_CURLY))
	{
		auto curr = vars_count;
		auto size = vars.1;
		while(!try_consume(CLOSE_CURLY))comp_func_body(ops);
		resize(vars,size);
		max_vars_count = max(max_vars_count,vars_count);
		vars_count = curr;
		return true;
	}
	return false;
}


compile_stmt(ops)
{
	compile_expression(0,ops);
	try_consume_error(SEMICOLON,"Expected ;");
}

compile_return(ops)
{
	if(try_consume(RETURN))
	{
		auto arg = compile_expression(0,ops);
		push_back(ops,{RETURNVALUE,arg});
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}


comp_func_body(ops)
{
	if(try_consume(SEMICOLON)) return 1;
	else if(auto_vardec(ops)) return 1;
	else if(extrn_decl(ops)) return 1;
	else if(compile_return(ops)) return 1;
	else if(compile_scope(ops)) return 1;
	else if(compile_while(ops))return 1;
	else compile_stmt(ops);
}

comp_block(ops)
{
	if(try_consume(OPEN_CURLY))
	{
		while(!try_consume(CLOSE_CURLY))comp_func_body(ops);
	}
	else comp_func_body(ops);
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
		comp_block(ops);
		push_back(compiler,{name,ops,curr,max_vars_count});
		vars_count = 0;
		max_vars_count = 0;
		resize(vars,0);
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
	auto len = tokens.1;
	while(IrGen.1 < len)
	{
		compile_prog();
	}
}


