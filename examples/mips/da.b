pushback(ptr,val)
{
	extrn malloc,putint,pnl;
	auto siz = *(ptr+4);
	auto cap = *(ptr+8);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = malloc(4*cap);
		auto new = *ptr;
		auto i = 0;
		while( i < siz)
		{
			*(new+4*i)=*(prev+4*i);
			i++;
		}
	}
	auto base = *ptr;
	*(base+4*(siz++))=val;
	*(ptr + 8) = cap;
	*(ptr + 4)  = siz;
	putint(*ptr);
	putint(*(ptr+4));
	putint(*(ptr+8));
	pnl();
}
print(ptr)
{
	auto siz = *(ptr+4);
	auto base = *ptr;
	auto i =0;
	while(i < siz)
	{
		putint(*(base+4*i));
		i++;
	}
	pnl();
}

main(){
	extrn memset;
	auto da = malloc(12);
	*da=0;
	*(da+4)=0;
	*(da+8)=0;
	pushback(da,10);
	pushback(da,20);
	pushback(da,40);
	print(da);
}
