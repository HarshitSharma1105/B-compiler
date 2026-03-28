fib(n)
{
    if(n<2) return n;
    else return fib(n-1)+fib(n-2);
}



main()
{
   extrn printf;
   auto n=10,i=0;
   while(i<n)
   {
        printf("%d ",(fib(i++)));
   }
   printf("\n");
}