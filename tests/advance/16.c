
char a = 2;
char *b = &a;
int c = 3;
// char arr[3] = {1, 2, 3};
// int arr2[3] = {4, 5, 6};

int main() {
  return (*b + c) * 2;  // 10
}