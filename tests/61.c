int main() {
  int* p;
  alloc4(&p, 1, 3, 5, 10);
  int** q = &p;
  return *(*q + 3);
}