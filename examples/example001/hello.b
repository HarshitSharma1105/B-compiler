main(){
    extrn putchar,printf;
    putchar(72);
    putchar(69);
    putchar(76);
    foo(76);
    putchar(79);
    foo(10);
}


foo(a){
    putchar(a);
}