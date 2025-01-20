#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/resource.h>

/*

_ # _ _
0 1 2 3

1 right
3 left


A - right
B - wait?
C - wait?
D - wait?
E - left
*/

enum constraints {
  max_width = 35,
  max_height = 35,
  max_players = 10
 };

struct point {
  int x, y;
};

struct point players[max_players];
int seen[max_height][max_width] = {0};

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
      } else if (seen[i][j]) {
        fprintf(stderr, "%c", '.');
      } else {
        fprintf(stderr, "%c", '?');
      }
    }
    fprintf(stderr, "\n");
  }
}

int main() {
  struct rlimit rl;
  if (getrlimit(RLIMIT_STACK, &rl) == 0) {
    fprintf(stderr, "Stack size: %ld bytes\n", rl.rlim_cur);
  } else {
    perror("getrlimit");
  }

  int width;
  scanf("%d", &width);
  assert(width > 0 && width <= max_width);
  int height;
  scanf("%d", &height);
  assert(height > 0 && height <= max_height);
  int players_count;
  scanf("%d", &players_count);
  assert(players_count > 0 && players_count <= max_players);
  fgetc(stdin);

  fprintf(stderr, "ver 1.3.2\n");
  fprintf(stderr, "width %d, height %d, players count %d\n", width, height,
          players_count);

  // game loop
  int turn_num = 0;
  while (++turn_num) {
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
      seen[y][x] = 1;
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    dump_grid(players_count, width, height);
    char move;
    if (turn_num == 0) {
      move = 'E';
    } else {
      move = 'A';
    }
    fprintf(stderr, "turn %d, move %c\n", turn_num, move);
    printf("%c\n", move);
  }

  return 0;
}