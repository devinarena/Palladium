
final class Main {
public static void main(String[] args)
{
int x = 1;
int y = 1;
while (true)
{
int z = x + y;
x = y;
y = z;
System.out.println(z);
if (
z >= 144
)
{
break;
}
}
}
}
