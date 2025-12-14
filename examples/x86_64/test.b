readbyte(ptr,i)
{
    auto j = i - i%4;
    i = i%4;
    return (*ptr & 255);
}



func(ptr)
{
    extrn printf;
    while(readbyte(ptr)) printf("%c",readbyte(ptr++));
}



main()
{
    asm("   mov rsp,rbp");
    asm("   pop rbp");
    asm("   xor rax,rax");
    asm("   ret");
    auto str = "hello world\n";
    func(str); // comments done?
    // yup done!
}