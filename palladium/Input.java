import java.util.Scanner;

public class Input {
  private static Scanner __palladium_scanner__ = new Scanner(System.in);

  private static String __palladium_input__(String prompt) {
    System.out.print(prompt);
    return __palladium_scanner__.nextLine();
  }

  public static void main(String[] args) {
    String name = __palladium_input__("What is your name? ");
    __palladium_scanner__.close();
  }
}
