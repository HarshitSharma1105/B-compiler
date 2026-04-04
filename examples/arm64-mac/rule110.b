extrn malloc,printf;

main()
{
    auto n = 70;
    auto board = malloc(8 * n);
    *(board + 8*(n-1)) = 1;
    auto i = 0;
    while (i < n)
    {
        auto j = 0;
        while(j < n)
        {
            if(board[j]) printf("*");
            else printf(" ");
            j++;
        }
        printf("\n");

        auto pattern = (board[0] << 1) | board[1]; 
        j = 1;
        while (j < n - 1)
        {
            pattern = ((pattern << 1) & 7 ) | board[j+1];
            board[j] = (110 >> pattern) & 1;
            j++;
        }
        i++;
    }
}