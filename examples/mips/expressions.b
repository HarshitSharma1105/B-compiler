foo(a)
{
    extrn puts,putint,pnl;
    puts("hello from foo ");
    putint(a);
    pnl();
    return a*a;
}

bar(a)
{
    extrn puts,putint,pnl;
    auto d=a*a;
    puts("hello from bar ");
    putint(d);
    pnl();
    return a++;
}

main(){
    auto a=30,b,c=a-100;
    a=a+10;
    b=5*(c-20)+65;
    c=-b++ + (-a);
    auto f=c++ + ++c;
    putint((a+10)*20);
    putint(b+30);
    putint(-c);
    pnl();
    foo(a++);
    foo(a);;;;;;;
    {
        auto e=foo(3)*10-20;
        puts("hi from scope ");
        putint(++a);
        putint(e);
        pnl();
    }
    putint(foo(11));
    pnl();
    putint(bar(10));
    pnl();
    return 0;
}