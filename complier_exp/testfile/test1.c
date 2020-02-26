

int fibonacci(int n)
{
    if(n==1||n==2)
        return 1;
    else
        return fibonacci(n-1) + fibonacci(n-2);
}


int main()
{
   
    int i,j,k;
    i = read();

    if(i < 1){
        k = fibonacci(2);
    }else
    {
        k= fibonacci(5);
    }
    write(k);

   
    
    return 0;
}

