main(a,b)
{
	auto x,y,z;
	x = 10;
	y = x+20;
	extrn printf;
	extrn malloc;
	auto p = malloc(8);
	*p = 100;
	printf("hello world %d %d %d %d\n",y++,x+y,foo(x+2),*p);
	while(x++<15)printf("%d\n",foo(x));
}

foo(a) return a+2;