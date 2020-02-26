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
    
    int ch[5];
    
    for(j=1;j<=5;j++)
    {
        k=fibonacci(j);
        ch[j-1] = fibonacci(j);
        write(ch[j-1]);
       
    }
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



