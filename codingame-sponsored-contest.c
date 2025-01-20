#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

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

int width;
int height;

enum move_type {
  move_right = 'A',
  move_wait = 'B',
  move_up = 'C',
  move_down = 'D',
  move_left = 'E'
};

enum cell_type { unknown = 0, free_cell = 1, wall = 2 };

struct point {
  int x, y;
};

struct point players[max_players];
enum cell_type seen[max_height][max_width] = {0};

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

void dump_grid(int players_count) {
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

enum cell_type from_char(const char raw_status) {
  assert(raw_status == '#' || raw_status == '_');
  return (raw_status == '#') ? wall : free_cell;
}

int up_index(int j) {
  int new_value = j - 1;
  if (new_value < 0) {
    new_value += height;
  }
  return new_value;
}

int down_index(int j) {
  int new_value = j + 1;
  if (new_value >= height) {
    new_value -= height;
  }
  return new_value;
}

int left_index(int i) {
  int new_value = i - 1;
  if (new_value < 0) {
    new_value += width;
  }
  return new_value;
}

int right_index(int i) {
  int new_value = i + 1;
  if (new_value >= width) {
    new_value -= width;
  }
  return new_value;
}

enum cell_type up(int x, int y) { return seen[up_index(y)][x]; }

enum cell_type down(int x, int y) { return seen[down_index(y)][x]; }

enum cell_type left(int x, int y) { return seen[y][left_index(x)]; }
enum cell_type right(int x, int y) { return seen[y][right_index(x)]; }

enum move_type move0(int x, int y) {
  enum move_type moves[4];
  int index = 0;

  if (up(x, y) != wall) {
    moves[index++] = move_up;
  }

  if (down(x, y) != wall) {
    moves[index++] = move_down;
  }

  if (left(x, y) != wall) {
    moves[index++] = move_left;
  }

  if (right(x, y) != wall) {
    moves[index++] = move_right;
  }

  if (index == 0) {
    return move_wait;
  } else {
    const int selected_index = rand() % index;
    return moves[selected_index];
  }
}

int main() {
  struct rlimit rl;
  if (getrlimit(RLIMIT_STACK, &rl) == 0) {
    fprintf(stderr, "Stack size: %ld bytes\n", rl.rlim_cur);
  } else {
    perror("getrlimit");
  }

  scanf("%d", &width);
  assert(width > 0 && width <= max_width);
  scanf("%d", &height);
  assert(height > 0 && height <= max_height);
  int players_count;
  scanf("%d", &players_count);
  assert(players_count > 0 && players_count <= max_players);
  fgetc(stdin);

  unsigned int seed = (unsigned int)time(NULL);
  fprintf(stderr, "ver 1.4.0, seed = %u\n", seed);
  srand(seed);

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
      const int x_mod = x % width;
      const int y_mod = y % height;
      fprintf(stderr, "%d: %d %d -> %d %d\n", i, x, y, x_mod, y_mod);
      const struct point pos = {x_mod, y_mod};
      players[i] = pos;
      seen[y][x] = free_cell;
      if (i == players_count - 1) {
        seen[up_index(y)][x] = from_char(up_status[0]);
        seen[down_index(y)][x] = from_char(down_status[0]);
        seen[y][right_index(x)] = from_char(right_status[0]);
        seen[y][left_index(x)] = from_char(left_status[0]);
      }
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    dump_grid(players_count);
    const struct point me = players[players_count - 1];
    const char move = move0(me.x, me.y);
    fprintf(stderr, "turn %d, move %c\n", turn_num, move);
    printf("%c\n", move);
  }

  return 0;
}