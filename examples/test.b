#include "examples/main.b"
#include "examples/another.b"


main(){
    extrn putchar,printf;
    auto a,b,c;
    b=69;
    a=b;
    h();
    b=78;
    c(a,b);
    putchar(10);
    d(74);
}

h()
{
    putchar(72);
}

