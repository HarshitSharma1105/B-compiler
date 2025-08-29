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
        e=2;
        printf("%d\n",e++);
    }
}
foo(a)
{
    printf("%d\n",a);
}