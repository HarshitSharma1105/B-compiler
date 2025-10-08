main(){
    extrn printf;
    auto x=0;
    while(x < 10){
        printf("%d %d\n",foo(x),bar(x++));
    }
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