pushback(ptr,val)
{
	extrn malloc,printf,free;
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		auto prev = *ptr;
		*ptr = malloc(4*cap);
		auto new = *ptr;
		auto i = 0;
		while(i < siz)
		{
			*(new+4*i)=*(prev+4*i);
			i++;
		}
		free(prev);
	}
	auto base = *ptr;
	*(base+4*(siz++))=val;
	*(ptr + 16) = cap;
	*(ptr + 8)  = siz;
	printf("%p %ld %ld\n",*ptr,*(ptr+8),*(ptr+16));
}
print(ptr)
{
	auto siz = *(ptr+8);
	auto base = *ptr;
	auto i = 0;
	while(i < siz)
	{
		printf("%d ",*(base+4*i));
		i++;
	}
	printf("\n");
}

main(){
	extrn memset;
	auto da = malloc(24);
	memset(da,0,24);
	pushback(da,10);
	pushback(da,20);
	pushback(da,40);
	pushback(da,100);
	pushback(da,102);
	print(da);
}
