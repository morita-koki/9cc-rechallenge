int main() {
  // clang-format off
  int a[][3][3] = {{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
                   {{10, 11, 12}, {13, 14, 15},},
                   {{19, 20, 21}, {22, 23, 24},}};
  return a[2][1][2];  // 24
}