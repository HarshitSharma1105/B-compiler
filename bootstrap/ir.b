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
globals;
arrays;


ir_count;
FUNCALL;
BINOP;
UNOP;
RETURNVALUE;
LABEL;
JMP;
JMPIFZERO;
STORE;
ASSEMBLY;

arg_count;
VAR;
LIT;
DATA_OFFSET;
FUNC_RESULT;
REF;
NO_ARG;


STORAGE_AUTO;
STORAGE_GLOBAL;
STORAGE_ARRAY;
storage_count;

precedences;

ir_init()
{
	precedences = alloc(24); 
	push_back(precedences,{{ASSIGN},1});
	push_back(precedences,{{LESS,GREATER,EQUALS,NOT_EQUALS},4});
	push_back(precedences,{{ADD,SUB},2});
	push_back(precedences,{{DIV,MULT,REMAINDER},3});


    FUNCALL     = ir_count++;
	BINOP		= ir_count++;
	UNOP		= ir_count++;
	RETURNVALUE = ir_count++;
	JMP			= ir_count++;
	JMPIFZERO	= ir_count++;
	LABEL		= ir_count++;
	STORE		= ir_count++;
	ASSEMBLY	= ir_count++;

	VAR 		= arg_count++;
	LIT 		= arg_count++;
	DATA_OFFSET = arg_count++;
	FUNC_RESULT = arg_count++;
	REF			= arg_count++;
	NO_ARG		= arg_count++;

	STORAGE_AUTO=storage_count++;
	STORAGE_GLOBAL=storage_count++;
	STORAGE_ARRAY = storage_count++;

	globals = alloc(24);
	arrays	= alloc(24);
}

dbg_arg(arg)
{
	switch(arg.0)
	{
		case LIT: printf("Literal(%d)",arg.1);
		case VAR: 
		{
			switch(arg.3)
			{
				case STORAGE_AUTO:printf("AutoVar(%d)",arg.1);
				case STORAGE_GLOBAL:printf("Global %s",arg.2);
			}
		}
		case DATA_OFFSET: printf("DataOffset[%d]",arg.1);
		case FUNC_RESULT: printf("FuncResult(%s)",arg.1);
		case REF:		  printf("Ref(%d)",arg.1);
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
			dbg_arg(op.1);
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
			printf("Branch Label (%d)(",op.2);
			dbg_arg(op.1);
			printf(")\n");
		}
		case STORE:
		{
			printf("Store ");
			dbg_arg(op.2);
			printf(" at AutoVar(%d)\n",op.1);
		}
		case ASSEMBLY:
		{
			printf("Assembly %s\n",op.1);
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


peek(off=0)
{
	auto ptr = IrGen.0.0;
	auto idx = IrGen.1;
	return ptr[idx+off];
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
		if(!strcmp(ptr[i].2,name)) return ptr[i];
	}
	return {VAR,-1,NULL,STORAGE_AUTO};
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
			auto var = find_var(tok.1.0);
			if(var.1==-1)error("Undeclared variable %s",tok.1.0);
			return var;
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
			push_back(ops,{BINOP,{VAR,vars_count,NULL,STORAGE_AUTO},{NO_ARG},{FUNC_RESULT,func},ASSIGN});
			return {VAR,vars_count++,NULL,STORAGE_AUTO};
		}
		case MULT:
		{
			auto arg = compile_primary_expression(ops);
			push_back(ops,{BINOP,{VAR,vars_count,NULL,STORAGE_AUTO},{NO_ARG},arg,ASSIGN});
			return {REF,vars_count++};
		}
		case SUB:
		{
			auto arg = compile_primary_expression(ops);
			push_back(ops,{UNOP,vars_count,arg,SUB});
			return {VAR,vars_count++,NULL,STORAGE_AUTO};
		}
		case NOT:
		{
			auto arg = compile_primary_expression(ops);
			push_back(ops,{UNOP,vars_count,arg,NOT});
			return {VAR,vars_count++,NULL,STORAGE_AUTO};		
		}
		case OPEN_CURLY:
		{
			auto args = alloc(24);
			while(try_peek(CLOSE_CURLY)==false)
			{
				push_back(args,compile_expression(0,ops));
				try_consume(COMMA);
			}
			try_consume(CLOSE_CURLY);
			push_back(ops,{FUNCALL,"alloc",{{LIT,8*args.1}},1,1});
			auto ptr  = {VAR,vars_count++,NULL,STORAGE_AUTO};
			auto temp = {VAR,vars_count++,NULL,STORAGE_AUTO};
			push_back(ops,{BINOP,ptr,{NO_ARG},{FUNC_RESULT,"alloc"},ASSIGN});
			for(auto i = 0;i<args.1;i++)
			{
				push_back(ops,{BINOP,temp,ptr,{LIT,8*i},ADD});
				push_back(ops,{STORE,temp.1,args.0[i]});
			}
			return ptr;
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
			auto var = find_var(ident.1.0);
			if(var.1==-1)error("Undeclared variable %s",ident.1.0);
			push_back(ops,{BINOP,var,var,{LIT,1},ADD});
			return var;
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
	while(try_peek_vec({{DOT,OPEN_SQUARE},2}))
	{
		if(try_consume(DOT))
		{
			auto idx = compile_primary_expression(ops);
			temp = {VAR,vars_count++,NULL,STORAGE_AUTO};
			push_back(ops,{BINOP,temp,{LIT,8},idx,MULT});
    		push_back(ops,{BINOP,temp,ret,temp,ADD});
            ret = {REF,temp.1};
		}
		else if(try_consume(OPEN_SQUARE))
        {
            auto idx = compile_expression(0,ops);
            try_consume_error(CLOSE_SQUARE,"Expected closing ']'");
            temp = {VAR,vars_count++,NULL,STORAGE_AUTO};
        	push_back(ops,{BINOP,temp,{LIT,8},idx,MULT});
        	push_back(ops,{BINOP,temp,ret,temp,ADD});
            ret = {REF,temp.1};
        }
	}
	if(try_peek_vec({{INCR,DECR},2}))
	{
		auto type = consume().0;
		switch(ret.0)
		{
			case VAR:
			{
				temp = {VAR,vars_count++,NULL,STORAGE_AUTO};
				push_back(ops,{BINOP,temp,{NO_ARG},ret,ASSIGN});
				push_back(ops,{BINOP,ret,temp,{LIT,1},conv(type)});
				ret = temp;
			}
			case REF:
			{
				auto curr = ret.1;
				auto new_curr = vars_count++;
				push_back(ops,{BINOP,{VAR,new_curr,NULL,STORAGE_AUTO},{NO_ARG},ret,ASSIGN});
				temp = {VAR,vars_count++,NULL,STORAGE_AUTO};
				push_back(ops,{BINOP,temp,ret,{LIT,1},conv(type)});
				push_back(ops,{STORE,curr,temp});
				ret = {VAR,new_curr,NULL,STORAGE_AUTO};
			}
		}
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
			switch(lhs.0)
			{
				case VAR:push_back(ops,{BINOP,lhs,{NO_ARG},rhs,type});
				case REF:push_back(ops,{STORE,lhs.1,rhs});
				default: error("Assigning to non assignable values");
			}
		}
		else
		{
			rhs = compile_expression(prec+1,ops);
			auto new_lhs = {VAR,vars_count++,NULL,STORAGE_AUTO};
			push_back(ops,{BINOP,new_lhs,lhs,rhs,type});
			lhs = new_lhs;
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
			push_back(vars,{VAR,vars_count++,name,STORAGE_AUTO});
			compile_expression(0,ops);
			try_consume(COMMA);
		}
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}

extrn_decl()
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
compile_branch(ops)
{
	if(try_consume(IF))
	{
		push_back(ops,{LABEL,labels_count++});
		auto curr_vars = vars_count;
		auto vars_size = vars.1;
		auto arg = compile_expression(0,ops);
		auto jmp_idx = ops.1;
		push_back(ops,{JMPIFZERO,arg,0});
		comp_block(ops);
		auto if_jmp = ops.1;
		push_back(ops,{JMP,labels_count});
		push_back(ops,{LABEL,labels_count});
		ops.0[jmp_idx].2 = labels_count++;
		max_vars_count = max(vars_count,max_vars_count);
		resize(vars,vars_size);
		vars_count = curr_vars;
		if(try_consume(ELSE))
		{
			curr_vars = vars_count;
			vars_size = vars.1;
			comp_block(ops);
			push_back(ops,{LABEL,labels_count});
			ops.0[if_jmp].1 = labels_count++;
			max_vars_count = max(vars_count,max_vars_count);
			resize(vars,vars_size);
			vars_count = curr_vars;
		}
		return true;
	}
	return false;
}

compile_asm(ops)
{
	if(try_consume(ASM))
	{
		try_consume_error(OPEN_PAREN,"Expected )");
		auto code = try_consume_error(STRING_LIT,"Expected string lit").1.0;
		push_back(ops,{ASSEMBLY,code});
		try_consume_error(CLOSE_PAREN,"Expected (");
		try_consume_error(SEMICOLON,"Expected ;");
		return true;
	}
	return false;
}

comp_func_body(ops)
{
	if(try_consume(SEMICOLON)) return 1;
	else if(auto_vardec(ops)) return 1;
	else if(extrn_decl()) return 1;
	else if(compile_return(ops)) return 1;
	else if(compile_scope(ops)) return 1;
	else if(compile_while(ops))return 1;
	else if(compile_branch(ops))return 1;
	else if(compile_asm(ops))return 1;
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
	auto asm_only = false;
	if(try_peek(ATTRIB))
	{
		auto attr = consume().1.0;
		if(strcmp(attr,"asm"))error("Unknown Attribute");
		asm_only = true;
	}
	if(try_peek(FUNCTION))
	{
		auto name = consume().1.0;
		auto ops = alloc(24);
		try_consume_error(OPEN_PAREN,"Expected (");
		auto vars_size = vars.1;
		while(try_peek(IDENTIFIER))
		{
			push_back(vars,{VAR,vars_count++,consume().1.0,STORAGE_AUTO});
			try_consume(COMMA);
		}
		auto curr = vars_count;
		try_consume_error(CLOSE_PAREN,"Expected )23");
		comp_block(ops);
		max_vars_count = max(vars_count,max_vars_count);
		vars_count = 0;
		resize(vars,vars_size);
		push_back(compiler,{name,ops,curr,max_vars_count,asm_only});
		max_vars_count = 0;
	}
	else if(try_peek(EXTERN))
	{
		extrn_decl();
	}
	else if(try_peek(IDENTIFIER))
	{
		auto name = consume().1.0;
		if(try_consume(OPEN_SQUARE))
		{
			push_back(vars,{VAR,arrays.1,name,STORAGE_ARRAY});
			auto size = atoll(try_consume_error(INTLIT,"Only constant integer sizes supported for now").1.0);
			try_consume_error(CLOSE_SQUARE,"Expected ]");
			push_back(arrays,{name,size});
		}
		else
		{
			auto val = 0;
			if(try_peek(INTLIT))val = atoll(consume().1.0);
			push_back(vars,{VAR,globals.1,name,STORAGE_GLOBAL});
			push_back(globals,{name,val});
		}
		try_consume_error(SEMICOLON,"Expected ;");
	}
	else if(try_consume(DECL))
	{
		try_consume(IDENTIFIER);
		try_consume_error(SEMICOLON,"Expected ;");
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


