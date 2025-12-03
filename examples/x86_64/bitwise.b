main()
{
    extrn printf;
    auto c = 1 << 3;
    auto d = 1024 >> 8;
    auto e = d | c;
    auto f = c & e;
    printf("%d %d %d %d\n",c,d,e,f);
}