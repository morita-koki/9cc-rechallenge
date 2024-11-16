int a[3];
int b[3] = {1, 2, 3};
char c[3];
char d[3] = {1, 2, 3};
char e[] = {1, 2, 3};

int main() {
  return b[0] + b[1] + b[2] + d[1] + e[1];  // 10
}
