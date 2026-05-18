public class Test {
public static int fib(int n)
{
if (
n <= 1
)
{
return n;
}
return fib(n - 1) + fib(n - 2);
}
public static int factorial(int n)
{
if (
n <= 1
)
{
return 1;
}
return n * factorial(n - 1);
}
public static void main(String[] args)
{
System.out.println(fib(10));
System.out.println(factorial(5));
}
}