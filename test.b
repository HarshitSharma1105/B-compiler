#include"lib.b"

b 200;
arrays[200];
alloc(size) return malloc(size);
foo(a) 
{
	if(a>100) return 10;
	else if(a>17) return a+2;
	else return 2;
}
extrn malloc,printf;
main(a,b)
{
	auto x,y,z;
	x = 10;
	y = x+20;
	auto p = malloc(8);
	*p = 100;
	*p++;
	printf("hello world %d %d %d %d\n",y++,x+y,foo(x+2),*p);
	b=10;
	arrays[20] = 40; 
	while(x++<20)printf("%d\n",foo(x));
	auto arr = {1,2,3,4};
	auto i = 0;
	while(i<4)printf("%d\n",arr[i++]);
	printf("bar results = %d\n",bar(20)+arrays[20]);
}

