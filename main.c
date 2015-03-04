#include <stdio.h>
#include "backtrace.h"

int func2(int a, int b);
int func1(int a, int b);
int func0(int a, int b);

int func2(int a, int b)
{
	int c = a * b;
	printf("%s: c = %d\n", __FUNCTION__, c);
	show_backtrace();
	return c;
}

int func1(int a, int b)
{
	int c = func2(a, b);
	printf("%s: c = %d\n", __FUNCTION__, c);
	return c;
}

int func0(int a, int b)
{
	int c = func1(a, b);
	printf("%s: c = %d\n", __FUNCTION__, c);
	return c;
}

int main()
{
	int a = 4, b = 5;
	int (*funcptr)(int, int) = func0;
	
	int c = func0(a, b);
	printf("%s: c = %d\n", __FUNCTION__, c);
	
	printf("funcptr's name = %s\n", addr_to_name((unsigned long)funcptr));
	return 0;
}
