public class Input {
  public int x;

  public Input(int x) {
    this.x = x;
  }

  public int square() {
    return x * x;
  }

  public static void main(String[] args) {
    Input input = new Input(5);
    System.out.println(input.square());
  }
}