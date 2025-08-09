main(){
    extrn printf;
    auto a,b,c;
    a=20+10;
    c=a-100;
    b=5*(c-20)+65;
    c=-b-10;
    printf("%d,%d,%d\n",a,-b,c);
}