#include"ir.b"



asm_str;
regs;


generate_arg(arg)
{
	switch(arg.0)
	{
		case LIT:			format_str(asm_str,"    mov r15,%d",arg.1);
		case VAR: 			format_str(asm_str,"    mov r15,[rbp-%d]",8*(arg.1+1));
		case DATA_OFFSET : 	format_str(asm_str,"    mov r15,data_%d",arg.1);
		case FUNC_RESULT :  format_str(asm_str,"	mov r15,rax");
		case NO_ARG	     :  {}
		default: error("UNREACHABLE");
	}
}



generate_op(op)
{
	switch(op.0)
	{
		case FUNCALL:
		{
			auto args = (op+16);
			auto size = args.1;
			for(auto i=0;i<size;i++)
			{
				generate_arg(op.2[i]);
				format_str(asm_str,"	mov %s,r15",regs[i]);
			}
			format_str(asm_str,"	xor rax,rax");
			format_str(asm_str,"	call %s",op.1);
		}
		case BINOP :
		{
			generate_arg(op.3);
			format_str(asm_str,"	mov r14,r15");
			generate_arg(op.2);
			switch(op.4)
			{
				case ASSIGN: format_str(asm_str,"	 mov r15,r14");
				case ADD :   format_str(asm_str,"    add r15,r14");
				case SUB :   format_str(asm_str,"    sub r15,r14");
				case MULT:   format_str(asm_str,"	 imul r15,r14");
				default :    error("UNKNOWN BINOP");
			}
			format_str(asm_str,"	mov QWORD [rbp-%d],r15",8*(op.1+1));
		}
		case UNOP:
		{
			generate_arg(op.2);
			format_str(asm_str,"	mov r14,r15");
			switch(op.3)
			{
				case SUB:format_str(asm_str,"    xor r15,r15\n    sub r15,r14");
				default: error("UNREACHABLE UNOP");
			}
			format_str(asm_str,"	mov QWORD [rbp-%d],r15",8*(op.1+1));
		}
		case RETURNVALUE:
		{
			generate_arg(op.1);
			format_str(asm_str,"	mov rax,r15");
			format_str(asm_str,"	mov rsp,rbp");
			format_str(asm_str,"	pop rbp");
			format_str(asm_str,"	ret"); 
		}
		default : error("UNREACHABLE");
	}
}


generate_func(func)
{
    auto size = func.1.1;
    auto ops = func.1.0;
    for(auto i = 0;i<size;i++)generate_op(ops[i]);
}

generate_func_prologue(func)
{
	auto num_args = func.2,alloc_vars = func.3;
	if(num_args>6)error("Too many args");
	if(alloc_vars%2)alloc_vars++;
	format_str(asm_str,"public %s",func.0);
	format_str(asm_str,"%s:",func.0);
	format_str(asm_str,"	push rbp");
	format_str(asm_str,"	mov rbp,rsp");
	format_str(asm_str,"	sub rsp,%d",8*alloc_vars);
	for(auto i=0;i<num_args;i++) format_str(asm_str,"    mov [rbp-%d],%s",8*(i+1),regs[i]);
}

generate_func_epilogue()
{
	format_str(asm_str,"	mov rsp,rbp");
	format_str(asm_str,"	pop rbp");
	format_str(asm_str,"	xor rax,rax");
	format_str(asm_str,"	ret"); 
}


generate_extrns()
{
	auto size = extrns_arr.1;
	auto base = extrns_arr.0;
	for(auto i=0;i<size;i++)format_str(asm_str,"	extrn %s",base[i]);
}

generate_data_seg()
{
	format_str(asm_str,"section \"data\" writeable");
	auto count = 0,idx = 0,size=data_string.1;
	while(idx<size)
	{
		format_str_2(asm_str,"data_%d db ",count++);
		while(read_byte(data_string.0,idx)!=10)push_char(asm_str,read_byte(data_string.0,idx++));
		idx++;
		push_char(asm_str,10);
	}
}

generate()
{
	regs = {"rdi","rsi","rdx","rcx","r8","r9"};
	asm_str = alloc(24);
	format_str(asm_str,"format ELF64");
	format_str(asm_str,"section \"text\" executable");
	auto size = compiler.1;
	auto funcs = compiler.0;
	for(auto i=0;i<size;i++)
	{
		generate_func_prologue(funcs[i]);
		generate_func(funcs[i]);
		generate_func_epilogue();
	}
	generate_extrns();
	generate_data_seg();
}