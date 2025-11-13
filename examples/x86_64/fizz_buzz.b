main()
{
    extrn  printf,malloc,scanf;
    auto p=malloc(8),n;
    scanf("%ld",p);
    n=*p;
    if(n>7)
    {
        if(n>10) printf("first\n");
        else printf("second\n");
    }
    else if(n < 5){
        printf("third\n");
    }
    else {
        printf("none\n");
    }
}