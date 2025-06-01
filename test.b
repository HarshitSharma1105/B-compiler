foo(){
    auto d;
    d=69;
    putchar(d);
}


main(){
    extrn putchar,printf;
    auto a,b,c;
    b=69;
    a=b;
    foo();
}