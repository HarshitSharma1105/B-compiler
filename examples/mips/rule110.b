main()
{
    extrn malloc,puts,pnl;
    auto n = 100;
    auto board = malloc(4 * n);
    board[(4,n-1)] = 1;
    auto i = 0;
    while (i < n)
    {
        auto j = 0;
        while(j < n)
        {
            if(board[(4,j)]) puts("*");
            else puts(" ");
            j++;
        }
        pnl();

        auto pattern = (board[(4,0)] << 1) | board[(4,1)]; 
        j = 1;
        while (j < n - 1)
        {
            pattern = ((pattern << 1) & 7 ) | board[(4,j+1)];
            board[(4,j)] = (110 >> pattern) & 1;
            j++;
        }
        i++;
    }
}