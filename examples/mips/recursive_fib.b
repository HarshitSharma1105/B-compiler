fib(n)
{
    if(n<2) return n;
    return fib(n-1)+fib(n-2);
}



main()
{
   extrn putint;
   auto n=10,i=0;
   while(i<n)
   {
        putint(fib(i++));
   }
}