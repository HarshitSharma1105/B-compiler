[[asm]]
read_byte(ptr,idx)
{
	asm("	movzx eax,BYTE [rdi+rsi]");
	asm("	ret");
}
[[asm]]
write_byte(ptr,ch,idx)
{
	asm("	mov BYTE [rdi+rdx], sil");
	asm("	ret");
}

extrn sprintf,printf,exit;
extrn read,open,write;


error(msg,x1=0,x2=0,x3=0,x4=0,x5=0) 
{
	printf(msg,x1,x2,x3,x4,x5);
	printf("\n");
	exit(1);
}



alloc_size;
arena[20480];

alloc(size)
{
	if(size+alloc_size > 20480) error("Ran out of arena memory . Please change capacity of arena\n");
	auto ptr = arena + alloc_size;
	alloc_size = alloc_size + size;
	return ptr;
}

push_back(ptr,val)
{
	auto siz = ptr.1;
	auto cap = ptr.2;
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto new = alloc(8*cap);
		for(auto i = 0; i < siz;i++) new[i] = ptr.0[i];
		ptr.0 = new;
	}
	ptr.0[siz++] = val;
	ptr.2 = cap;
	ptr.1= siz;
}


size(ptr) return ptr.1;

back(ptr)
{
	auto siz = size(ptr);
	if(siz>0)return (*ptr + 8*(siz - 1));
	else return 0;
}

resize(ptr,siz)
{
	ptr.1 = siz;
	auto cap = ptr.2;
	while(cap<siz) cap = 2*cap+1;
	auto new = alloc(8*cap);
	for(auto i = 0; i < siz;i++) new[i] = ptr.0[i];
	ptr.0 = new;
	ptr.2 = cap;
}




push_char(ptr,ch)
{
	auto siz = ptr.1;
	auto cap = ptr.2;
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto new = alloc(cap+1);
		for(auto i = 0;i < siz;i++)write_byte(new,read_byte(ptr.0,i),i);
		ptr.0 = new;
	}
	write_byte(ptr.0,ch,siz++);
	ptr.2 = cap;
	ptr.1  = siz;
}


push_str(ptr,str)
{
	for(auto i = 0;read_byte(str,i);i++)push_char(ptr,read_byte(str,i));
}


temp[100];
format_str(src,fmt,x1=0,x2=0,x3=0,x4=0)
{
	sprintf(temp,fmt,x1,x2,x3,x4);
	push_str(src,temp);
	push_char(src,10);
}
