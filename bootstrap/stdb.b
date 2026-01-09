read_byte(ptr,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    return (*(ptr + idx) & (255 << rem)) >> rem;
}


write_byte(ptr,ch,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    *(ptr + idx) = *(ptr + idx) & (~ (255 << rem)) | (ch << rem);
}

extrn free,malloc,memset,calloc;
extrn sprintf,printf,exit;
extrn read,open,write;



arena_cap;
alloced;
curr_arena;
arena_list;
alloc_size;
arena;

alloc(size)
{
	if(size+alloc_size>arena_cap & alloced)
	{
		printf("Ran out of arena memory. Please change arena_cap initialization here\n");
		exit(1);
	}
	if(!alloced)
	{
		arena_cap = 20480;
		arena = malloc(arena_cap);
		memset(arena,0,arena_cap);
		alloced = true;
	}
	auto ptr = arena + alloc_size;
	alloc_size = alloc_size + size;
	return ptr;
}

error(msg,x1=0,x2=0,x3=0,x4=0,x5=0) 
{
	printf(msg,x1,x2,x3,x4,x5);
	printf("\n");
	exit(1);
}


push_back(ptr,val)
{
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = alloc(8*cap);
		auto new = *ptr;
		for(auto i = 0; i < siz;i++) new[i] = prev[i];
	}
	auto base   = *ptr;
	base[siz++] = val;
	*(ptr + 16) = cap;
	*(ptr + 8)  = siz;
}


size(ptr)
{
	return *(ptr+8);
}

back(ptr)
{
	auto siz = size(ptr);
	if(siz>0)return (*ptr + 8*(siz - 1));
	else return 0;
}

resize(ptr,siz)
{
	*(ptr+8) = siz;
	auto cap = *(ptr+16);
	while(cap<siz) cap = 2*cap+1;
	auto prev = *ptr;
	*ptr = alloc(8*cap);
	auto new = *ptr;
	for(auto i = 0; i < siz;i++) new[i] = prev[i];
	*(ptr+16) = cap;
}




push_char(ptr,ch)
{
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = alloc(cap+1);
		auto new = *ptr;
		for(auto i = 0;i < siz;i++)write_byte(new,read_byte(prev,i),i);
	}
	auto base = *ptr;
	write_byte(base,ch,siz++);
	*(ptr + 16) = cap;
	*(ptr + 8)  = siz;
}


push_str(ptr,str)
{
	for(auto i = 0;read_byte(str,i);i++)push_char(ptr,read_byte(str,i));
}

format_str(src,fmt,x1=0,x2=0,x3=0,x4=0)
{
	auto temp = calloc(100,1);
	sprintf(temp,fmt,x1,x2,x3,x4);
	push_str(src,temp);
	push_char(src,10);
	free(temp);
}
