main()
{
    extrn InitWindow,WindowShouldClose,BeginDrawing,EndDrawing,ClearBackground;
    auto a=800,b=600,c="hello from B";
    InitWindow(a,b,c);
    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(4294902015);
        EndDrawing();
    }
    return 2-2;
}