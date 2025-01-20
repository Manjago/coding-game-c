#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

enum constraints { max_players = 10 };

struct point {
  int x, y;
};

struct point players[max_players];

/*
width, height
35 28
24 19
13 29
29 29
*/

int index_of_player(int players_count, int x, int y) {
  for (int i = 0; i < players_count; ++i) {
    const struct point player = players[i];
    if (player.x == x && player.y == y) {
      return i;
    }
  }
  return -1;
}

void dump_grid(int players_count, int width, int height) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      const int player_index = index_of_player(players_count, j, i);
      if (player_index + 1) {
        fprintf(stderr, "%d", player_index);
      } else {
        fprintf(stderr, "%c", '.');
      }
    }
    fprintf(stderr, "\n");
  }
}

int main() {
  int width;
  scanf("%d", &width);
  int height;
  scanf("%d", &height);
  int players_count;
  assert(players_count >= 0 && players_count <= 10);
  scanf("%d", &players_count);
  fgetc(stdin);

  fprintf(stderr, "ver 1.2.0\n");
  fprintf(stderr, "width %d, height %d, players count %d\n", width, height,
          players_count);

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

    fprintf(stderr, "%c %c %c %c\n", first_input[0], second_input[0],
            third_input[0], fourth_input[0]);

    for (int i = 0; i < players_count; i++) {
      int x;
      int y;
      scanf("%d%d", &x, &y);
      fgetc(stdin);
      fprintf(stderr, "%d: %d %d\n", i, x, y);
      const struct point pos = {x, y};
      players[i] = pos;
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    dump_grid(players_count, width, height);
    printf("A\n");
  }

  return 0;
}