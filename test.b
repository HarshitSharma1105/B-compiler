main(a,b)
{
	auto x,y,z;
	x = 10;
	y = 68;
	extrn printf;
	printf("hello world %c %d %d\n",y++,x+y,foo(x+2));
}

foo(a) return a+2;