public class Test {
public static void main(String[] args)
{
int health = 100;
int enemyHealth = 60;
while (
health > 0 && enemyHealth > 0
)
{
System.out.println("Player Health: " + health);
System.out.println("Enemy Health: " + enemyHealth);
enemyHealth = enemyHealth - 15;
health = health - 10;
}
System.out.println("Battle Over!");
}
}