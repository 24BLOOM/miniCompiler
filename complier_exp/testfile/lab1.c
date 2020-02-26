
struct tree { // 测试结构体
	char c;
	float f;
	int i;
};

int f(int a) {  // 测试函数定义
	struct tree t1, t2; // 测试结构体变量的定义
	char a = 'a';  // 测试函数char类型
	int i = 5; // 测试int类型定义和初始化
	float j = 5.5; 
	int b,c;
	b = 1||1;
	t1.a = 1; // 测试结构体访问
	a[1] = 2; // 测试数据访问
	i++; // 测试自增运算
	i--; // 测试自减运算
	
	for(j=0;j<=5;j++)
    {
        //k=fibonacci(j);
        //ch[j-1] = fibonacci(j);
        //write(ch[j]);
    }


	f(1,2,3); // 测试函数调用
	if (i < 1) // 测试比价运算
		//if (b >= 2) /*测试嵌套if语句*/
			//b = 2 || 1; // 测试逻辑运算
		//else 
			//b = 3 + 1; // 测试算术运算
		b = 1;
	else // 测试就近匹配原则
		b = 3;
	
    // //f

	
	return 0; // 测试返回语句
}



