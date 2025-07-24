main(){
    extrn putchar,printf;
    auto x,y,z;
    x="hello";
    z=2;
    y=103;
    putchar(y);
    printf("hello world\n");
    x=x+z;
    printf("%s",x);
    putchar(z+8);
}