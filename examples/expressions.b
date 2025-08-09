main(){
    extrn printf;
    auto a,b,c;
    a=20+10;
    c=a-100;
    b=c-2*5/2+65;
    c=-b-10;
    printf("%d,%d,%d\n",a,-b,c);
}