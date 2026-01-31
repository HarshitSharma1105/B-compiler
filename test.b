main(a,b)
{
	auto x,y,z;
	x = 10;
	y = 68;
	extrn printf;
	printf("hello world %c %d %d\n",y++,x+y,foo(x+2));
	while(x++<15)printf("%d\n",foo(x));
}

foo(a) return a+2;