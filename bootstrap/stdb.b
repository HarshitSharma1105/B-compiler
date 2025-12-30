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
		*ptr = alloc(8*cap);
		auto new = *ptr;
		for(auto i = 0; i < siz;i++)
		{
			new[i]=prev[i];
		}
		free(prev);
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
}




pushchar(ptr,ch)
{
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = alloc(cap+1);
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

extrn sprintf;

pushstr(ptr,str)
{
	for(auto i = 0;readbyte(str,i);i++)pushchar(ptr,readbyte(str,i));
}

formatstr(src,fmt,x1,x2,x3,x4)
{
	auto temp = alloc(100);
	sprintf(temp,fmt,x1,x2,x3,x4);
	pushstr(src,temp);
	pushchar(src,10);
	free(temp);
}
