#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/resource.h>

/*

_ # _ _
0 1 2 3
u r d l

0 up
1 right
2 down
3 left


A - right
B - wait
C - up
D - down
E - left
*/

enum constraints { max_width = 35, max_height = 35, max_players = 10 };

enum maze_type {
  unknown = 0,
  free_cell = 1,
  wall = 2
 };

struct point {
  int x, y;
};

struct point players[max_players];
enum maze_type seen[max_height][max_width] = {0};

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
        switch (seen[i][j]) {
        case unknown:
          fprintf(stderr, "%c", '?');
          break;
        case free_cell:
          fprintf(stderr, "%c", '.');
          break;
        case wall:
          fprintf(stderr, "%c", '#');
          break;
        }
      }
    }
    fprintf(stderr, "\n");
  }
}

enum maze_type from_char(const char raw_status) {
  assert(raw_status == '#' || raw_status == '_');
  return (raw_status == '#') ? wall : free_cell;
}

int up_index(int height, int j) {
  int new_value = j - 1;
  if (new_value < 0) {
    new_value += height;
  }
  return new_value;
}

int down_index(int height, int j) {
  int new_value = j + 1;
  if (new_value >= height) {
    new_value -= height;
  }
  return new_value;
}

int left_index(int width, int i) {
  int new_value = i - 1;
  if (new_value < 0) {
    new_value += width;
  }
  return new_value;
}

int right_index(int width, int i) {
  int new_value = i + 1;
  if (new_value >= width) {
    new_value -= width;
  }
  return new_value;
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

  fprintf(stderr, "ver 1.3.3\n");
  fprintf(stderr, "width %d, height %d, players count %d\n", width, height,
          players_count);

  // game loop
  int turn_num = 0;
  while (++turn_num) {

    /*
    _ # _ _
    0 1 2 3
    u r d l

    0 up
    1 right
    2 down
    3 left

     */

    char up_status[2];
    scanf("%[^\n]", up_status);
    fgetc(stdin);
    char right_status[2];
    scanf("%[^\n]", right_status);
    fgetc(stdin);
    char down_status[2];
    scanf("%[^\n]", down_status);
    fgetc(stdin);
    char left_status[2];
    scanf("%[^\n]", left_status);

    fprintf(stderr, "u %c r %c d %c l %c\n", up_status[0], right_status[0],
            down_status[0], left_status[0]);

    for (int i = 0; i < players_count; i++) {
      int x;
      int y;
      scanf("%d%d", &x, &y);
      fgetc(stdin);
      fprintf(stderr, "%d: %d %d\n", i, x, y);
      const struct point pos = {x, y};
      players[i] = pos;
      seen[y][x] = free_cell;
      if (i == players_count - 1) {
         seen[up_index(height, y)][x] = from_char(up_status[0]);
         seen[down_index(height, y)][x] = from_char(down_status[0]);
         seen[y][right_index(width, x)] = from_char(right_status[0]);
         seen[y][left_index(width, x)] = from_char(left_status[0]);
      }
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    dump_grid(players_count, width, height);
    char move;
    if (turn_num == 0) {
      move = 'E';
    } else if (turn_num == 4) {
      move = 'D';
    } else if (turn_num >= 3) {
      move = 'C';
    } else {
      move = 'A';
    }
    fprintf(stderr, "turn %d, move %c\n", turn_num, move);
    printf("%c\n", move);
  }

  return 0;
}