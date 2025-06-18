public class Test {
public static void main(String[] args)
{
float x = 1f;
float y = 1f;
while (true) {
{
float z = x + y;
x = y;
y = z;
System.out.println(z);
if (
z >= 144f
)
{
break;
}
}
}
}
}