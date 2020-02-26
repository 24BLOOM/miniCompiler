

int fibonacci(int n,int b)
{
    if(n==1||n==2)
        return 1;
    else
        return fibonacci() + fibonacci(n-2);
}

int test(int a){
   // return 1.0;
}
void test2(int a){
   // return 1.0;
}

int main()
{
    float a;
    int b[4];
    int c;
    c = 1/0;
    a();
    a[1]=1;
    b[1.0]= 2;
    b['a'] = 'a';
    b[4]=1;
    1=2;
    test(a);
    fibonacci();
   
    return 0;
}

