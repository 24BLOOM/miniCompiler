

int fibonacci(int n)
{
    if(n==1||n==2)
        return 1;
    else
        return fibonacci(n-1) + fibonacci(n-2);
}


int main()
{
    // int ch[5];
    int i,j,k;
    //int i;
    int ch[5];
    //i=read();
    //k = 2;
    //ch[k-1] = 2;  //不对数组访问是否越界进行检查
    // ch[5] = 1;  //数组访问越界
    // ch['a'] = 2; //下标类型错误
    // // for(j=1;j<=5;j++)
    // {
    //     //k=fibonacci(j);
    //     ch[j-1] = fibonacci(j);
    //     write(fibonacci(j));
    // }
    for(j=1;j<=5;j++)
    {
        k=fibonacci(j);
        ch[j-1] = fibonacci(j);
        write(ch[j-1]);
        //write(k);
    }
   
    //fibonacci();
    //fibonacci();
    //k++;
    //write(ch[1]);
    //write(i);
    return 0;
}

