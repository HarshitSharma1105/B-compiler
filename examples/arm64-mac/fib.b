main()
{
    extrn printf;
    auto a=0,b=1;
    printf("%d\n",a);
    while(a<100000)
    {
        auto c=a+b;
        printf("%d\n",a);
        a=b;
        b=c;
    }
}
