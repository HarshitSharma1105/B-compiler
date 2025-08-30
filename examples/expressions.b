main(){
    extrn printf;
    auto a,b,c;
    a=20+10;
    c=a-100;
    b=5*(c-20)+65;
    c=-b++ + (-a);
    printf("%d,%d,%d\n",(a+10)*20,(b+30),-c);
    foo(a++);
    foo(a);
    {
        auto e;
        e=foo(3)*10-20;
        bar(e);
        printf("hi from scope %d\n",e++);
    }
}
foo(a)
{
    printf("hello from foo %d\n",a);
    return a*a;
}

bar(a)
{
    auto d;
    d=a*a;
    printf("hello from bar %d\n",d);
}