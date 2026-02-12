#include"lib.b"

b 200;

extrn malloc,printf;
main(a,b)
{
	auto x,y,z;
	x = 10;
	y = x+20;
	auto p = malloc(8);
	*p = 100;
	*p++;
	printf("hello world %d %d %d %d %d\n",y++,x+y,foo(x+2),*p,200);
	b=10;
	while(x++<15)printf("%d\n",foo(x+b));
	printf("bar results = %d\n",bar(20));
}

foo(a) return a+2;