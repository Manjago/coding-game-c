
/*
why

up:_, right:#, down:_, left:#
0: 23 7 MOVED from 24 7
1: 4 7 MOVED from 3 7
2: 16 22 MOVED from 17 22
3: 16 17 STAY
4: 6 8 MOVED from 6 9
???????????????????????????????????
???????????????????????????????????
?######????????????????????????????
#.......???????????????????????????
#.####.#???????????????????????????
#.#??#.#???????????????????????????
#.#??#.###?????????????????????????
#..?1......????????????0???????????
#.#??#X##.#????????????????????????
#.####.##.###??????????????????????
#......##....#?????????????????????
?#####.??###.#?????????????????????
?????????###.#?????????????????????
????????#.....?????????????????????
????????#.###??????????????????????
????????#.#????????????????????????
????????#.#????????????????????????
????????#.#?????3??????????????????
????????#.#????????????????????????
????????#..????????????????????????
????????#.#????????????????????????
????????#.###??????????????????????
????????.....#??2??????????????????
?????????###.#?????????????????????
???????????#.#?????????????????????
???????????....????????????????????
????????????##?????????????????????
???????????????????????????????????
from 6,8 bfs suggest 6,7
see 6,7,up 6,6,down 6,8,left 5,7,right 7,7
see 6,6,up 6,5,down 6,7
see 6,8,up 6,7,down 6,9
see 5,7,right 6,7
see 7,7,left 6,7,right 8,7
see 6,5,up 6,4,down 6,6
see 6,9,up 6,8,down 6,10
see 8,7,left 7,7,right 9,7
see 6,4,up 6,3,down 6,5
see 6,10,up 6,9,down 6,11,left 5,10
see 9,7,down 9,8,left 8,7,right 10,7
see 6,3,down 6,4,left 5,3,right 7,3
see 6,11,up 6,10
see 5,10,left 4,10,right 6,10
see 9,8,up 9,7,down 9,9
see 10,7,left 9,7
see 5,3,left 4,3,right 6,3
see 7,3,left 6,3
see 4,10,left 3,10,right 5,10
see 9,9,up 9,8,down 9,10
see 4,3,left 3,3,right 5,3
see 3,10,left 2,10,right 4,10
see 9,10,up 9,9,right 10,10
see 3,3,left 2,3,right 4,3
see 2,10,left 1,10,right 3,10
see 10,10,left 9,10,right 11,10
see 2,3,left 1,3,right 3,3
see 1,10,up 1,9,right 2,10
see 11,10,left 10,10,right 12,10
see 1,3,down 1,4,right 2,3
see 1,9,up 1,8,down 1,10
see 12,10,down 12,11,left 11,10
see 1,4,up 1,3,down 1,5
see 1,8,up 1,7,down 1,9
see 12,11,up 12,10,down 12,12
see 1,5,up 1,4,down 1,6
see 1,7,up 1,6,down 1,8,right 2,7
see 12,12,up 12,11,down 12,13
see 1,6,up 1,5,down 1,7
see 2,7,left 1,7
see 12,13,up 12,12,left 11,13,right 13,13
see 11,13,left 10,13,right 12,13
see 13,13,left 12,13
see 10,13,left 9,13,right 11,13
see 9,13,down 9,14,right 10,13
see 9,14,up 9,13,down 9,15
see 9,15,up 9,14,down 9,16
see 9,16,up 9,15,down 9,17
see 9,17,up 9,16,down 9,18
see 9,18,up 9,17,down 9,19
see 9,19,up 9,18,down 9,20,right 10,19
see 9,20,up 9,19,down 9,21
see 10,19,left 9,19
see 9,21,up 9,20,down 9,22
see 9,22,up 9,21,left 8,22,right 10,22
see 8,22,right 9,22
see 10,22,left 9,22,right 11,22
see 11,22,left 10,22,right 12,22
see 12,22,down 12,23,left 11,22
see 12,23,up 12,22,down 12,24
see 12,24,up 12,23,down 12,25
see 12,25,up 12,24,left 11,25,right 13,25
see 11,25,right 12,25
see 13,25,left 12,25,right 14,25
see 14,25,left 13,25
no enemy for 6,7
turn 55, move UP
*/
// 2025-01-28 1638 points
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MY_TEST 1

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

typedef struct {
  int x, y;
} Point;

bool point_equals(const Point a, const Point b) {
  return a.x == b.x && a.y == b.y;
}

int point_man_dist(const Point a, const Point b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
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

typedef struct {
  Point monsters[max_players - 1];
  int monsters_count;
  Point explorer;
} GameState;

int index_of_monster(const GameState *game_state, const Point point) {
  for (int i = 0; i < game_state->monsters_count; ++i) {
    const Point monster = game_state->monsters[i];
    if (point_equals(monster, point)) {
      return i;
    }
  }
  return -1;
}

// Queue begin

#define MAX_Q_SIZE max_height *max_width

const Point undefined_point = {-1, -1};

typedef struct {
  Point point;
  Point first_move;
} QueueItem;

struct queue {
  QueueItem queue[MAX_Q_SIZE];
  int front;
  int back;
};

typedef struct queue Queue;

void queue_init(Queue *q) {
  q->front = -1;
  q->back = -1;
}

bool queue_is_empty(const Queue *q) { return (q->front == -1); }

void queue_push(Queue *q, QueueItem data) {
  assert(q->back < (MAX_Q_SIZE - 1));
  if (queue_is_empty(q)) {
    q->front = 0;
  }
  q->back++;
  q->queue[q->back] = data;
}

QueueItem queue_pop(Queue *q) {
  assert(!queue_is_empty(q));
  QueueItem data = q->queue[q->front];
  if (q->front == q->back) {
    q->front = -1;
    q->back = -1;
  } else {
    q->front++;
  }
  return data;
}
// Queue end

enum cell_type { unknown = 0, wall = 1, space = 2 };

typedef enum cell_type CellType;

CellType from_char(const char raw_status) {
  assert(raw_status == '#' || raw_status == '_');
  return (raw_status == '#') ? wall : space;
}

CellType grid[max_height][max_width] = {0};

void set_cell_type(Point point, CellType value) {
  grid[point.y][point.x] = grid[point.y][point.x] | value;
}

bool is_enemy_at(const Point point, const GameState *game_state, bool trace) {
  for (int i = 0; i < game_state->monsters_count; ++i) {
    if (point_equals(game_state->monsters[i], point)) {
      if (trace) {
        fprintf(stderr, "iet: enemy %d at %d,%d\n", i,
                game_state->monsters[i].x, game_state->monsters[i].y);
      }
      return true;
    }
  }
  return false;
}

bool cell_type_is(Point point, CellType value) {
  return grid[point.y][point.x] == value;
}

bool target_for_bfs_move(const Point point, const GameState *game_state,
                         bool trace) {
  return cell_type_is(point, unknown) && !is_enemy_at(point, game_state, trace);
}

QueueItem create(const Point pretender, const QueueItem prev) {

  QueueItem result;
  result.point = pretender;
  if (point_equals(undefined_point, prev.first_move)) {
    result.first_move = pretender;
  } else {
    result.first_move = prev.first_move;
  }
  return result;
}

typedef bool (*BfsTarget)(Point, const GameState *, bool);
typedef bool (*IsAllowedPoint)(const Point);

bool is_allowed_for_explore(const Point point) {
  return !cell_type_is(point, wall);
}

bool is_allowed_for_enemy_predict(const Point point) {
  return cell_type_is(point, space);
}

QueueItem bfs(const Point start, const GameState *game_state,
              BfsTarget bfs_target, IsAllowedPoint is_allowed_point,
              bool trace) {
  int seen[max_height][max_width] = {0};
  Queue queue;
  queue_init(&queue);
  QueueItem start_queue_item = {start, undefined_point};
  queue_push(&queue, start_queue_item);
  while (!queue_is_empty(&queue)) {
    QueueItem current = queue_pop(&queue);
    if (seen[current.point.y][current.point.x]) {
      continue;
    }
    seen[current.point.y][current.point.x] = 1;

    if (bfs_target(current.point, game_state, trace)) {
      return current;
    } else {
      if (trace) {
        fprintf(stderr, "see %d,%d", current.point.x, current.point.y);
      }
    }

    Point pretender;

    pretender = point_up(current.point);
    if (is_allowed_point(pretender)) {
      if (trace) {
        fprintf(stderr, ",up %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_down(current.point);
    if (is_allowed_point(pretender)) {
      if (trace) {
        fprintf(stderr, ",down %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_left(current.point);
    if (is_allowed_point(pretender)) {
      if (trace) {
        fprintf(stderr, ",left %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_right(current.point);
    if (is_allowed_point(pretender)) {
      if (trace) {
        fprintf(stderr, ",right %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    if (trace) {
      fprintf(stderr, "\n");
    }
  }

  return start_queue_item;
}

void dump_grid(const GameState *game_state, int rows, int cols) {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      const Point point = {j, i};
      const int monster_index = index_of_monster(game_state, point);
      if (monster_index + 1) {
        fprintf(stderr, "%d", monster_index);
      } else if (point_equals(game_state->explorer, point)) {
        fprintf(stderr, "%c", 'X');
      } else {
        const CellType cell_type = grid[i][j];
        if (cell_type == unknown) {
          fprintf(stderr, "%c", '?');
        } else if (cell_type & wall) {
          fprintf(stderr, "%c", '#');
        } else if (cell_type & space) {
          fprintf(stderr, "%c", '.');
        } else {
          fprintf(stderr, "%c", '!');
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

void log_move(const MoveType mt) {
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

MoveType from_points_to_move(const Point from, const Point to) {
  if (point_equals(point_left(from), to)) {
    return move_left;
  } else if (point_equals(point_right(from), to)) {
    return move_right;
  } else if (point_equals(point_up(from), to)) {
    return move_up;
  } else if (point_equals(point_down(from), to)) {
    return move_down;
  } else if (point_equals(from, to)) {
    return move_wait;
  }
  abort();
}

Point alter_move(const GameState *game_state) {
  Point moves[5];
  moves[0] = game_state->explorer;
  moves[1] = point_left(game_state->explorer);
  moves[2] = point_right(game_state->explorer);
  moves[3] = point_up(game_state->explorer);
  moves[4] = point_down(game_state->explorer);
  int current_dist = -1;
  int pretender_index = -1;
  for (int i = 0; i < 5; ++i) {
    const Point current_point = moves[i];
    if (cell_type_is(current_point, space)) {

      const QueueItem answer = bfs(current_point, game_state, &is_enemy_at,
                                   &is_allowed_for_enemy_predict, false);

      const bool is_valid = !point_equals(answer.first_move, undefined_point);

      if (is_valid) {
        const Point nearest_enemy = answer.point;
        const int enemy_dist = point_man_dist(current_point, nearest_enemy);
        if (enemy_dist > current_dist) {
          current_dist = enemy_dist;
          pretender_index = i;
        }
      } else {
        current_dist = 100500;
        pretender_index = i;
        break;
      }
    }
  }

  if (pretender_index != -1) {
    fprintf(stderr, "escaper suggest %d,%d with dist %d\n",
            moves[pretender_index].x, moves[pretender_index].y, current_dist);
    return moves[pretender_index];
  } else {
    fprintf(stderr, "escaper suggest STAY\n");
    return moves[0];
  }
}

MoveType do_move(const GameState *game_state) {
  dump_grid(game_state, height, width);

  Point target_point = bfs(game_state->explorer, game_state,
                           &target_for_bfs_move, &is_allowed_for_explore, false)
                           .first_move;
  fprintf(stderr, "from %d,%d bfs suggest %d,%d\n", game_state->explorer.x,
          game_state->explorer.y, target_point.x, target_point.y);

  if (point_equals(undefined_point, target_point)) {
    target_point = game_state->explorer;
  }

  const QueueItem answer = bfs(target_point, game_state, &is_enemy_at,
                               &is_allowed_for_enemy_predict, true);

  const bool is_valid = !point_equals(answer.first_move, undefined_point);

  const Point nearest_enemy = answer.point;

  int enemy_dist;

  if (is_valid) {
    enemy_dist = point_man_dist(target_point, nearest_enemy);
    fprintf(stderr, "enemy at %d,%d, dist %d\n", nearest_enemy.x,
            nearest_enemy.y, enemy_dist);
  } else {
    fprintf(stderr, "no enemy for %d,%d\n", target_point.x, target_point.y);
  }

  if (is_valid && enemy_dist <= 3) {
    return from_points_to_move(game_state->explorer, alter_move(game_state));
  } else {
    return from_points_to_move(game_state->explorer, target_point);
  }
}

void game_loop() {
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

  Point prev_pos[players_count];
  for (int i = 0; i < players_count; ++i) {
    prev_pos[i] = undefined_point;
  }

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
      const Point pos = {x, y};
      bool moved = !point_equals(pos, prev_pos[i]);
      if (moved) {
        fprintf(stderr, "%d: %d %d MOVED from %d %d\n", i, x, y, prev_pos[i].x,
                prev_pos[i].y);
      } else {
        fprintf(stderr, "%d: %d %d STAY\n", i, x, y);
      }
      prev_pos[i] = pos;

      if (i == players_count - 1) {
        game_state.explorer = pos;
        set_cell_type(pos, space);
        set_cell_type(point_up(pos), from_char(up_status[0]));
        set_cell_type(point_down(pos), from_char(down_status[0]));
        set_cell_type(point_right(pos), from_char(right_status[0]));
        set_cell_type(point_left(pos), from_char(left_status[0]));
      } else {
        game_state.monsters[i] = pos;
      }
    }

    const char move = do_move(&game_state);
    fprintf(stderr, "turn %d, move ", turn_num);
    log_move(move);
    printf("%c\n", move);
  }
}

void parse_maze(const char *maze[], int rows, int cols, GameState *state) {
  memset(state, 0, sizeof(*state));

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      const char c = maze[y][x];

      if (c >= '0' && c <= '9') {
        int idx = c - '0';
        if (idx < max_players - 1) {
          state->monsters[state->monsters_count++] = (Point){x, y};
          grid[y][x] = space;
        }
      } else if (c == 'X') {
        state->explorer = (Point){x, y};
        grid[y][x] = space;
      } else {
        switch (c) {
        case '#':
          grid[y][x] = wall;
          break;
        case '.':
          grid[y][x] = space;
          break;
        case '?':
          grid[y][x] = unknown;
          break;
        default:
          grid[y][x] = unknown;
        }
      }
    }
  }
}

void test1() {
  const char *maze[] = {"???????????????????????????????????",
                        "???????????????????????????????????",
                        "?######????????????????????????????",
                        "#.......???????????????????????????",
                        "#.####.#???????????????????????????",
                        "#.#??#.#???????????????????????????",
                        "#.#??#.###?????????????????????????",
                        "#..?1......????????????0???????????",
                        "#.#??#X##.#????????????????????????",
                        "#.####.##.###??????????????????????",
                        "#......##....#?????????????????????",
                        "?#####.??###.#?????????????????????",
                        "?????????###.#?????????????????????",
                        "????????#.....?????????????????????",
                        "????????#.###??????????????????????",
                        "????????#.#????????????????????????",
                        "????????#.#????????????????????????",
                        "????????#.#?????3??????????????????",
                        "????????#.#????????????????????????",
                        "????????#..????????????????????????",
                        "????????#.#????????????????????????",
                        "????????#.###??????????????????????",
                        "????????.....#??2??????????????????",
                        "?????????###.#?????????????????????",
                        "???????????#.#?????????????????????",
                        "???????????....????????????????????",
                        "????????????##?????????????????????",
                        "???????????????????????????????????"};

  int rows = sizeof(maze) / sizeof(maze[0]);
  int cols = (int)strlen(maze[0]);

  fprintf(stderr, "rows = %d, cols = %d\n", rows, cols);

  GameState state;

  parse_maze(maze, rows, cols, &state);

  dump_grid(&state, rows, cols);
}

void tests() {
  fprintf(stderr, "start test 1\n");
  test1();
  fprintf(stderr, "finish test 1\n");
}

int main() {
#ifndef MY_TEST
  game_loop();
#else
  tests();
#endif

  return 0;
}