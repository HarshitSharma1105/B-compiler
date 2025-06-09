#include "examples/another.b"
#include "examples/main.b"


main(){
    extrn putchar,printf;
    auto a,b,c;
    b=69;
    a=b;
    h();
    c(65,99);
    putchar(10);
    d(74);
}

h()
{
    putchar(72);
}

