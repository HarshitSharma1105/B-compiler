main(){
    extrn putchar;
    auto a,b;
    a=72;
    putchar(a);
    b=69;
    putchar(b);
    putchar(76);
    a=79;
    foo(a);
    foo(10);
}


foo(a){
    putchar(a);
}