main(){
    extrn printf;
    auto a=30,b,c=a-100;
    a=a+10;
    b=5*(c-20)+65;
    c=-b++ + (-a);
    auto f=c++ + ++c;
    printf("%d,%d,%d\n",(a+10)*20,(b+30),-c);
    foo(a++);
    foo(a);;;;;;;
    {
        auto e=foo(3)*10-20;
        printf("hi from scope %d %d\n",++a,e);
    }
    printf("%d %d\n",foo(11),bar(10));
    return 0;
}
foo(a)
{
    printf("hello from foo %d\n",a);
    return a*a;
}

bar(a)
{
    auto d=a*a;
    printf("hello from bar %d\n",d);
    return a++;
}