main()
{
    extrn malloc,printf;
    auto n = 30;
    auto board = malloc(8 * n);
    *(board + 8*(n-1)) = 1;
    auto i = 0;
    while (i < n)
    {
        auto j = 0;
        while(j < n)
        {
            if(*(board + 8 * j)) printf("*");
            else printf(" ");
            j++;
        }
        printf("\n");

        auto pattern = (*(board) << 1) | *(board + 8); 
        j = 1;
        while (j < n - 1)
        {
            pattern = ((pattern << 1) & 7 ) | *(board + 8*(j+1));
            *(board + 8*j) = (110 >> pattern) & 1;
            j++;
        }
        i++;
    }
}