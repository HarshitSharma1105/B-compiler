main(){
    extrn putchar;
    auto a,b;
    a=72;
    putchar(a);
    b=69;
    a=b;
    putchar(a);
    putchar(76);
    b=76;
    putchar(b);
    a=34;
    b=a+21;
    foo(a+b-10);
    foo(10);
}

foo(a){
    putchar(a);
}