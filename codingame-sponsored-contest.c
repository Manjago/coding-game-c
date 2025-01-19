#include <stdbool.h>
#include <stdio.h>

int main() {
  int first_init_input;
  scanf("%d", &first_init_input);
  int second_init_input;
  scanf("%d", &second_init_input);
  int third_init_input;
  scanf("%d", &third_init_input);
  fgetc(stdin);

  fprintf(stderr, "ver 1.0.0\n");
  fprintf(stderr, "%d %d %d\n", first_init_input, second_init_input, third_init_input);

  // game loop
  while (1) {
    char first_input[2];
    scanf("%[^\n]", first_input);
    fgetc(stdin);
    char second_input[2];
    scanf("%[^\n]", second_input);
    fgetc(stdin);
    char third_input[2];
    scanf("%[^\n]", third_input);
    fgetc(stdin);
    char fourth_input[2];
    scanf("%[^\n]", fourth_input);

    fprintf(stderr, "%c %c %c %c\n", first_input[0],
            second_input[0], third_input[0], fourth_input[0]);

    for (int i = 0; i < third_init_input; i++) {
      int fifth_input;
      int sixth_input;
      scanf("%d%d", &fifth_input, &sixth_input);
      fgetc(stdin);
      fprintf(stderr, "%d: %d %d\n", i, fifth_input, sixth_input);
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    printf("A\n");
  }

  return 0;
}