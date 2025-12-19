readbyte(ptr,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    return (*(ptr + idx) & (255 << rem)) >> rem;
}


writebyte(ptr,ch,idx)
{
    auto rem = idx % 8;
    idx = idx - rem;
    rem = rem * 8;
    *(ptr + idx) = *(ptr + idx) & (~ (255 << rem)) | (ch << rem);
}
