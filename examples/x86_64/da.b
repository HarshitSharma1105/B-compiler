pushback(ptr,val)
{
	extrn malloc,printf;
	auto siz = *(ptr+8);
	auto cap = *(ptr+16);
	if(siz == cap)
	{
		cap = 2 * cap + 1;
		*ptr = malloc(4*cap);
	}
	*(ptr + 16) = cap;
	*(ptr + 8)  = ++siz;
	printf("%p %ld %ld\n",*ptr,*(ptr+8),*(ptr+16));
}


main(){
	extrn memset;
	auto da = malloc(24);
	memset(da,0,24);
	pushback(da,10);
	pushback(da,20);
	pushback(da,40);
}
