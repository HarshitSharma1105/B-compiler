readbyte(ptr,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    return (*(ptr + idx) & (255 << rem)) >> rem;
}


writebyte(ptr,ch,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    *(ptr + idx) = *(ptr + idx) & (~ (255 << rem)) | (ch << rem);
}

extrn malloc,memset,free;

alloc(bytes)
{
	auto ptr = malloc(bytes);
	memset(ptr,0,bytes);
	return ptr;
}

pushback(ptr,val)
{
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = malloc(8*cap);
		auto new = *ptr;
		auto i = 0;
		while(i < siz)
		{
			new[i]=prev[i++];
		}
		free(prev);
	}
	auto base = *ptr;
	base[siz++]=val;
	*(ptr + 16) = cap;
	*(ptr + 8)  = siz;
}



pushstr(ptr,ch)
{
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = malloc(cap);
		auto new = *ptr;
		auto i = 0;
		while(i < siz)
		{
			writebyte(new,readbyte(prev,i),i);
			i++;
		}
		free(prev);
	}
	auto base = *ptr;
	writebyte(base,ch,siz++);
	*(ptr + 16) = cap;
	*(ptr + 8)  = siz;
}
