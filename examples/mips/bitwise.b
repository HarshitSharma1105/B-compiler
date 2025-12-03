main()
{
    extrn putint;
    auto c = 1 << 3;
    auto d = 1024 >> 8;
    auto e = d | c;
    auto f = c & e;
    putint(c);
    putint(d);
    putint(e);
    putint(f);
}