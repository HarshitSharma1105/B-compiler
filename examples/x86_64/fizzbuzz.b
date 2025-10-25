main()
{
    extrn printf;
    auto x=4;
    if(x<5)
    {
        auto e=-21;
        printf("hi from first if %d\n",(e+20)*10);
    }
    if(x>3){
        auto y=4;
        printf("hello from if %d\n",y);
        x--;
    }
    if(x-4){
        extrn exit;
        exit(69);
    }
    else{
        printf("in the else block\n");
    }
    printf("hello \n");
}