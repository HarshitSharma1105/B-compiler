#include"ir.b"



asm_str;
regs;



set_up_regs()
{
	
	regs = alloc(48);
	regs[0] = "rdi";
	regs[1] = "rsi";
	regs[2] = "rdx";
	regs[3] = "rcx";
	regs[4] = "r8";
	regs[5] = "r9";
}

generate_arg(arg)
{
	if(arg[0] == LIT) format_str(asm_str,"	mov r15,%d",arg[1]);
	else if(arg[0] == VAR) format_str(asm_str,"	mov r15,[rbp-%d]",8*arg[1]);
	else error("UNREACHABLE\n");
}



generate_op(op)
{
    auto type = op[0];
    if(type == AUTOASSIGN)
	{
		generate_arg(op[2]);
		format_str(asm_str,"	mov QWORD[rbp-%d],r15",8*op[1]);
	}
	else if(type == FUNCALL)
	{
		auto args = (op+16);
		auto size = size(args);
		for(auto i=0;i<size;i++)
		{
			generate_arg(op[2][i]);
			format_str(asm_str,"	mov %s,r15",regs[i]);
		}
		format_str(asm_str,"	xor rax,rax");
		format_str(asm_str,"	call %s",op[1]);
	}
}


generate_func(func)
{
    auto name = func[0];
    auto size = size(func[1]);
    auto ops = *(func[1]);
    for(auto i = 0;i<size;i++)generate_op(ops[i]);
}

generate_func_prologue(func)
{
	auto num_args = func[2],alloc_vars = func[3];
	if(num_args>6)error("Too many args");
	if(alloc_vars%2)alloc_vars++;
	format_str(asm_str,"public %s",func[0]);
	format_str(asm_str,"%s:",func[0]);
	format_str(asm_str,"	push rbp");
    format_str(asm_str,"	mov rbp,rsp");
	format_str(asm_str,"	sub rsp,%d",8*alloc_vars);
}

generate_func_epilogue()
{
	format_str(asm_str,"	mov rsp,rbp");
	format_str(asm_str,"	pop rbp");
	format_str(asm_str,"	xor rax,rax");
	format_str(asm_str,"	ret");
}


generate()
{
	set_up_regs();
	asm_str = alloc(24);
	format_str(asm_str,"format ELF64");
	format_str(asm_str,"section \".text\" executable");
	auto size = size(compiler);
    auto funcs = compiler[0];
    for(auto i=0;i<size;i++)
	{
		generate_func_prologue(funcs[i]);
		generate_func(funcs[i]);
		generate_func_epilogue();
	}
	size = size(extrns_arr);
	auto base = extrns_arr[0];
	for(auto i=0;i<size;i++)format_str(asm_str,"	extrn %s",base[i]);
	return asm_str;
}