readbyte(ptr,idx)
{
    auto rem = idx % 4;
    idx = idx - rem;
    auto val = *(ptr + idx);
    rem = rem * 8;
    return (val & (255 << rem)) >> rem;
}


writebyte(ptr,ch,idx)
{
    auto rem = idx % 4;
    idx = idx - rem;
    rem = rem * 8;
    *(ptr + idx) = *(ptr + idx) & (~ (255 << rem)) | (ch << rem);
}