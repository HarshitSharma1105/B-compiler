main()
{
    extrn InitWindow,WindowShouldClose,BeginDrawing,EndDrawing,ClearBackground;
    extrn SetTargetFPS,DrawRectangle;
    auto a=800,b=600,c="hello from B";
    InitWindow(a*2,b+1000,c);
    SetTargetFPS(60);
    auto x=200,y=200;
    auto dx=2,dy=2;
    while(!WindowShouldClose())
    {
        x=x+dx;
        y=y+dy;
        BeginDrawing();
        ClearBackground(0);
        DrawRectangle(x,y,100,100,4278190335);
        EndDrawing();
    }
    return 2-2;
}