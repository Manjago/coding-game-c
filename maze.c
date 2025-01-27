#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int width;
int height;

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

enum constraints { max_width = 35, max_height = 35, max_players = 10 };

struct point {
  int x, y;
};

typedef struct point Point;

bool point_equals(const struct point a, const struct point b) {
  return a.x == b.x && a.y == b.y;
}

Point point_up(const Point point) {
  Point result = point;
  result.y = up_index(result.y);
  return result;
}

Point point_down(const Point point) {
  Point result = point;
  result.y = down_index(result.y);
  return result;
}

Point point_left(const Point point) {
  Point result = point;
  result.x = left_index(result.x);
  return result;
}

Point point_right(const Point point) {
  Point result = point;
  result.x = right_index(result.x);
  return result;
}

enum cell_type { unknown = 0, wall = 1, space = 2, explored = 4 };

typedef enum cell_type CellType;

CellType from_char(const char raw_status) {
  assert(raw_status == '#' || raw_status == '_');
  return (raw_status == '#') ? wall + explored : space + explored;
}

CellType grid[max_height][max_width] = {0};

void setCellType(Point point, CellType value) {
  grid[point.y][point.x] = grid[point.y][point.x] | value;
}

struct game_state {
  Point monsters[max_players - 1];
  int monsters_count;
  Point explorer;
};

typedef struct game_state GameState;

int index_of_monster(const GameState *game_state, const Point point) {
  for (int i = 0; i < game_state->monsters_count; ++i) {
    const struct point monster = game_state->monsters[i];
    if (point_equals(monster, point)) {
      return i;
    }
  }
  return -1;
}

void dump_grid(const GameState *game_state) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      const Point point = {j, i};
      const int monster_index = index_of_monster(game_state, point);
      if (monster_index + 1) {
        fprintf(stderr, "%d", monster_index);
      } else if (point_equals(game_state->explorer, point)) {
        fprintf(stderr, "%c", '@');
      } else {
        const CellType cell_type = grid[i][j];
        if (cell_type == unknown) {
          fprintf(stderr, "%c", '?');
        } else if (cell_type & wall) {
          fprintf(stderr, "%c", '#');
        } else if (cell_type & explored) {
          fprintf(stderr, "%c", '.');
        } else { // unexplored, but space
          fprintf(stderr, "%c", ',');
        }
      }
    }
    fprintf(stderr, "\n");
  }
}

enum move_type {
  move_right = 'A',
  move_wait = 'B',
  move_up = 'C',
  move_down = 'D',
  move_left = 'E'
};

typedef enum move_type MoveType;

void decode_move(const MoveType mt) {
  switch (mt) {
  case move_right:
    fprintf(stderr, "RIGHT\n");
    break;
  case move_wait:
    fprintf(stderr, "WAIT\n");
    break;
  case move_up:
    fprintf(stderr, "UP\n");
    break;
  case move_down:
    fprintf(stderr, "DOWN\n");
    break;
  case move_left:
    fprintf(stderr, "LEFT\n");
    break;
  }
}

MoveType do_move(const GameState *game_state) {
  dump_grid(game_state);
  return move_up;
}

int main() {
  scanf("%d", &width);
  assert(width > 0 && width <= max_width);
  scanf("%d", &height);
  assert(height > 0 && height <= max_height);
  int players_count;
  scanf("%d", &players_count);
  assert(players_count > 0 && players_count <= max_players);
  fgetc(stdin);

  unsigned int seed = (unsigned int)time(NULL);
  fprintf(stderr, "ver 1.0.0, seed = %u\n", seed);
  srand(seed);

  fprintf(stderr, "width %d, height %d, players count %d\n", width, height,
          players_count);

  // game loop
  int turn_num = 0;
  while (++turn_num) {
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

    fprintf(stderr, "up:%c, right:%c, down:%c, left:%c\n", up_status[0],
            right_status[0], down_status[0], left_status[0]);

    GameState game_state;
    game_state.monsters_count = players_count - 1;

    for (int i = 0; i < players_count; i++) {
      int x;
      int y;
      scanf("%d%d", &x, &y);
      fgetc(stdin);
      x = x % width;
      y = y % height;
      fprintf(stderr, "%d: %d %d\n", i, x, y);
      const Point pos = {x, y};
      setCellType(pos, space);
      if (i == players_count - 1) {
        game_state.explorer = pos;
        setCellType(point_up(pos), from_char(up_status[0]));
        setCellType(point_down(pos), from_char(down_status[0]));
        setCellType(point_right(pos), from_char(right_status[0]));
        setCellType(point_left(pos), from_char(left_status[0]));
        setCellType(pos, explored);
      } else {
        game_state.monsters[i] = pos;
      }
    }

    const char move = do_move(&game_state);
    fprintf(stderr, "turn %d, move ", turn_num);
    decode_move(move);
    printf("%c\n", move);
  }

  return 0;
}