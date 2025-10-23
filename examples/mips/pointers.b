main()
{
    extrn malloc,putint,puts;
    auto p=malloc(40);
    *p=20;
    *((p+4))=37;
    putint(*p);
    putint(*(p+4));
    pnl();
    auto i=10;
    while(i>0){
        puts("writing ");
        putint(*(p+4*(10-i))=i);
        pnl();
        i--;
    }
    while(i<10)
    {
        puts("reading ");
        putint(*(p+4*i));
        pnl();
        i++;
    }
    return *p;
}