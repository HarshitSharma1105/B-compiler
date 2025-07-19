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
    a=79;
    foo(a);
    foo(10);
}


foo(a){
    putchar(a);
}