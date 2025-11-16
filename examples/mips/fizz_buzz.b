main()
{
    extrn  puts,getint,pnl;
    auto n=getint();
    if(n>7)
    {
        if(n>10) puts("first");
        else puts("second");
    }
    else if(n < 5){
        puts("third");
    }
    else {
        puts("none");
    }
    pnl();
}