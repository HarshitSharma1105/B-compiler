bar(z)
{
    return z+10;
}
[[asm]]
read_byte(ptr,idx)
{
	asm("	movzx eax,BYTE [rdi+rsi]");
	asm("	ret");
}
[[asm]]
write_byte(ptr,ch,idx)
{
	asm("	mov BYTE [rdi+rdx], sil");
	asm("	ret");
}