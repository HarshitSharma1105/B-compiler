main(argc,argv)
{
    extrn  printf,puts,malloc,scanf;
    printf("%d\n",argc);
    auto i=0;
    while(i!=argc)
    {
        printf("%s ",*(argv+8*i));
        i++;
    }
    printf("\n");
    auto n;
    scanf("%ld",&n);
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