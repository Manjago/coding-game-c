// need calc all enemy dist and use wih all dist
// 2025-03-04  992 points
// 2025-02-03 1060 points
// 2025-01-28 1638 points
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MY_TEST 1
#define INFINITY 100500000

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
  int dist[max_players];
} MobStat;

typedef struct {
  int x, y;
} Point;

enum cell_type { ct_unknown = 0, ct_wall = 1, ct_space = 2 };

typedef enum cell_type CellType;

CellType from_char(const char raw_status) {
  assert(raw_status == '#' || raw_status == '_');
  return (raw_status == '#') ? ct_wall : ct_space;
}

void set_cell_type(CellType grid[max_height][max_width], Point point,
                   CellType value) {
  const CellType old_value = grid[point.y][point.x];
  grid[point.y][point.x] = value;
  if (old_value == ct_unknown) {
    switch (value) {
    case ct_unknown:
      fprintf(stderr, "now %d,%d still unknow???\n", point.x, point.y);
      break;
    case ct_wall:
      fprintf(stderr, "now %d,%d WALL\n", point.x, point.y);
      break;
    case ct_space:
      fprintf(stderr, "now %d,%d SPACE\n", point.x, point.y);
      break;
    }
  }
}

bool point_equals(const Point a, const Point b) {
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

typedef struct {
  Point monsters[max_players - 1];
  int monsters_count;
  Point explorer;
  CellType grid[max_height][max_width];
} GameState;

void dump_mob_stat(const MobStat mob_stat, const GameState * game_state) {
  fprintf(stderr, "mob stat: ");
  for (int i = 0; i < game_state -> monsters_count; ++i) {
    fprintf(stderr, "%d:%d,", i, mob_stat.dist[i]);
  }
  fprintf(stderr, "\n");
}

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

bool is_valid_point(const Point point) {
  return !point_equals(point, undefined_point);
}

typedef struct {
  Point point;
  Point first_move;
  int dist;
} QueueItem;

int effective_dist_to_enemy(const QueueItem queue_item) {
  if (queue_item.dist > 0) {
    return queue_item.dist - 1;
  } else {
    return 0;
  }
}

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

Point lame_proximity_check;
bool is_lame_proximity_check(const Point point, const GameState *game_state, bool trace) {
  bool result = point_equals(point, lame_proximity_check);
  if (trace) {
    fprintf(stderr, "is_lame %d for ex %d,%d\n", result, game_state->explorer.x,
            game_state->explorer.y);
  }
  return result;
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

bool cell_type_is(
    const CellType grid[max_height][max_width], Point point,
    CellType value) {
  return grid[point.y][point.x] == value;
}

CellType get_cell_type(
    const CellType grid[max_height][max_width], const Point point) {
  return grid[point.y][point.x];
}

bool target_for_bfs_move(const Point point, const GameState *game_state,
                         bool trace) {
  return cell_type_is(game_state -> grid, point, ct_unknown) &&
         !is_enemy_at(point, game_state, trace);
}

bool is_valid_queue_item(const QueueItem queue_item) {
  return !point_equals(queue_item.first_move, undefined_point);
}

QueueItem create_from_point(const Point point) {
  return (QueueItem){point, undefined_point, 0};
}

QueueItem create(const Point pretender, const QueueItem prev) {

  QueueItem result;
  result.dist = prev.dist + 1;
  result.point = pretender;
  if (!is_valid_queue_item(prev)) {
    result.first_move = pretender;
  } else {
    result.first_move = prev.first_move;
  }
  return result;
}

typedef bool (*BfsTarget)(Point, const GameState *, bool);
typedef bool (*IsAllowedPoint)(const Point, const GameState *, bool);

bool is_allowed_for_explore(const Point point, const GameState *game_state,
                            bool trace) {
  const bool result = !cell_type_is(game_state -> grid, point, ct_wall) &&
                      !is_enemy_at(point, game_state, trace);
  if (trace) {
    fprintf(stderr, " iafe: %d %d ", result,
            get_cell_type(game_state->grid, point));
  }
  return result;
}

bool is_allowed_for_enemy_predict(const Point point,
                                  const GameState *game_state, bool trace) {
  const bool result = cell_type_is(game_state -> grid, point, ct_space) ||
                      is_enemy_at(point, game_state, trace);
  if (trace) {
    fprintf(stderr, " iafep: %d %d ", result,
            get_cell_type(game_state->grid, point));
  }
  return result;
}

bool is_allowed_for_enemy_move(const Point point, const GameState *game_state,
                               bool trace) {
  const bool result = cell_type_is(game_state -> grid, point, ct_space);
  if (trace) {
    fprintf(stderr, " iaem: %d %d %d ", result,
            get_cell_type(game_state->grid, point), game_state->monsters_count);
  }
  return result;
}

QueueItem bfs(const Point start, const GameState *game_state,
              BfsTarget bfs_target, IsAllowedPoint is_allowed_point,
              bool trace) {
  int seen[max_height][max_width] = {0};
  Queue queue;
  queue_init(&queue);
  QueueItem start_queue_item = create_from_point(start);
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
    if (is_allowed_point(pretender, game_state, trace)) {
      if (trace) {
        fprintf(stderr, ",up %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_down(current.point);
    if (is_allowed_point(pretender, game_state, trace)) {
      if (trace) {
        fprintf(stderr, ",down %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_left(current.point);
    if (is_allowed_point(pretender, game_state, trace)) {
      if (trace) {
        fprintf(stderr, ",left %d,%d", pretender.x, pretender.y);
      }
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_right(current.point);
    if (is_allowed_point(pretender, game_state, trace)) {
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
      const Point point = (Point){j, i};
      const int monster_index = index_of_monster(game_state, point);
      if (monster_index + 1) {
        fprintf(stderr, "%d", monster_index);
      } else if (point_equals(game_state->explorer, point)) {
        fprintf(stderr, "%c", 'X');
      } else {
        const CellType cell_type = game_state -> grid[i][j];
        if (cell_type == ct_unknown) {
          fprintf(stderr, "%c", '?');
        } else if (cell_type & ct_wall) {
          fprintf(stderr, "%c", '#');
        } else if (cell_type & ct_space) {
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

int sum_mob_stat(const MobStat mob_stat, const GameState * game_state) {
  int result = 0;
  for (int i = 0; i < game_state->monsters_count; ++i) {
    int dist = mob_stat.dist[i];
    if (dist >= 0) {
      result += dist;
    }
  }
  return result;
}

int min_mob_stat(const MobStat mob_stat, const GameState * game_state) {
  int result = INFINITY;
  for (int i = 0; i < game_state->monsters_count; ++i) {
    int dist = mob_stat.dist[i];
    if (dist >= 0 && dist < result) {
      result = dist;
    }
  }
  return result;
}

Point alter_move(const GameState *game_state) {
  Point moves[5];
  moves[0] = game_state->explorer;
  moves[1] = point_left(game_state->explorer);
  moves[2] = point_right(game_state->explorer);
  moves[3] = point_up(game_state->explorer);
  moves[4] = point_down(game_state->explorer);
  int current_scoring = -1;
  int pretender_index = -1;
  for (int i = 0; i < 5; ++i) {
    const Point current_point = moves[i];
    if (cell_type_is(game_state -> grid, current_point, ct_space)) {

      MobStat mob_stat;
      for (int m = 0; m < game_state->monsters_count; ++m) {
        lame_proximity_check = current_point;
        const QueueItem proximity_check =
            bfs(game_state->monsters[m], game_state, &is_lame_proximity_check,
                &is_allowed_for_enemy_move, false);

        if (is_valid_queue_item(proximity_check)) {
            mob_stat.dist[m] = effective_dist_to_enemy(proximity_check);
        } else {
          mob_stat.dist[m] = -1;
        }
      }
      fprintf(stderr, "for %d,%d ", current_point.x, current_point.y);
      dump_mob_stat(mob_stat, game_state);

      const int min = min_mob_stat(mob_stat, game_state);
      const bool really_no_enemies = min == INFINITY;
      if (really_no_enemies) {
        current_scoring = INFINITY;
        fprintf(stderr, "no escaper moves\n");
        break;
      } else {
        const int sum = sum_mob_stat(mob_stat, game_state);
        //const int scoring =  min*100000 + sum;
        const int scoring =  sum;
        if (scoring > current_scoring) {
          current_scoring = scoring;
          pretender_index = i;
        }
      }
    }
  }

  if (pretender_index != -1) {
    fprintf(stderr, "escaper suggest %d,%d with scoring %d\n",
            moves[pretender_index].x, moves[pretender_index].y, current_scoring);
    return moves[pretender_index];
  } else {
    fprintf(stderr, "escaper suggest STAY\n");
    return moves[0];
  }
}

MoveType do_move(const GameState *game_state) {
  dump_grid(game_state, height, width);

  Point suggested_by_bfs =
      bfs(game_state->explorer, game_state, &target_for_bfs_move,
          &is_allowed_for_explore, false)
          .first_move;
  fprintf(stderr, "from %d,%d bfs suggest %d,%d\n", game_state->explorer.x,
          game_state->explorer.y, suggested_by_bfs.x, suggested_by_bfs.y);

  if (!is_valid_point(suggested_by_bfs)) {
    suggested_by_bfs = game_state->explorer;
  }

  const QueueItem enemy_checker =
      bfs(suggested_by_bfs, game_state, &is_enemy_at,
          &is_allowed_for_enemy_predict, false);

  const bool is_valid =
      is_valid_queue_item(enemy_checker);

  const Point nearest_enemy = enemy_checker.point;

  int enemy_dist;

  if (is_valid) {
    enemy_dist = effective_dist_to_enemy(enemy_checker);
    fprintf(stderr, "enemy at %d,%d, dist %d to %d,%d\n", nearest_enemy.x,
            nearest_enemy.y, enemy_dist, suggested_by_bfs.x,
            suggested_by_bfs.y);
  } else {
    fprintf(stderr, "no enemy for %d,%d\n", suggested_by_bfs.x,
            suggested_by_bfs.y);
  }

  if (is_valid && enemy_dist <= 3) {
    return from_points_to_move(game_state->explorer, alter_move(game_state));
  } else {
    return from_points_to_move(game_state->explorer, suggested_by_bfs);
  }
}

void do_and_print_move(GameState *game_state, int turn_num) {
  const char move = do_move(game_state);
  fprintf(stderr, "turn %d, move ", turn_num);
  log_move(move);
  printf("%c\n", move);
}

void update_grid(GameState *game_state,
                 CellType external_grid[max_height][max_width]) {
  memcpy(game_state->grid, external_grid,
         sizeof(CellType) * max_height * max_width);
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

  CellType stable_grid[max_height][max_width] = {0};

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
        set_cell_type(stable_grid, pos, ct_space);
        set_cell_type(stable_grid, point_up(pos), from_char(up_status[0]));
        set_cell_type(stable_grid, point_down(pos), from_char(down_status[0]));
        set_cell_type(stable_grid, point_right(pos), from_char(right_status[0]));
        set_cell_type(stable_grid, point_left(pos), from_char(left_status[0]));
      } else {
        set_cell_type(stable_grid, pos, ct_space);
        game_state.monsters[i] = pos;
      }
    }
    update_grid(&game_state, stable_grid);

    do_and_print_move(&game_state, turn_num);
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
          state->monsters[idx] = (Point){x, y};
          state->monsters_count++;
          state -> grid[y][x] = ct_space;
        }
      } else if (c == 'X') {
        state->explorer = (Point){x, y};
        state->grid[y][x] = ct_space;
      } else {
        switch (c) {
        case '#':
          state->grid[y][x] = ct_wall;
          break;
        case '.':
          state->grid[y][x] = ct_space;
          break;
        case '?':
          state->grid[y][x] = ct_unknown;
          break;
        default:
          state->grid[y][x] = ct_unknown;
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
  do_and_print_move(&state, -1);
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