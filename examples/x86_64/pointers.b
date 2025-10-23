main()
{
    extrn malloc,printf;
    auto p=malloc(80);
    *p=20;
    *((p+8))=37;
    printf("%d %d\n",*(p),*(p+8));
    auto i=10;
    while(i>0){
        printf("writing %d\n",*(p+8*(10-i))=i);
        i--;
    }
    while(i<10)
    {
        printf("reading %d\n",*(p+8*i));
        i++;
    }
    return *p;
}