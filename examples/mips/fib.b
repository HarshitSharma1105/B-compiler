main()
{
    extrn putint,pnl;
    auto a=0,b=1;
    while(a<100000)
    {
        auto c=a+b;
        putint(a);
        pnl();
        a=b;
        b=c;
    }
}