

int main() {
  struct {
    int a;
    int b;
  } c;
  c.a = 1;
  c.b = 9;
  return c.a + c.b;
}