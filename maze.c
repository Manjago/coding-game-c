// need calc all enemy dist and use wih all dist
// 2025-03-11  662 points
// 2025-03-11  832 points
// 2025-03-11  678 points
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

double elapsed(clock_t start_t, clock_t end_t) {
  const clock_t delta_ticks = end_t - start_t;
  return (double)delta_ticks / CLOCKS_PER_SEC * 1000.0;
}

bool has_time(const clock_t start_t, const clock_t end_t, int limit_ms) {
  double elapsed_ms = elapsed(start_t, end_t);
  return elapsed_ms <= limit_ms;
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

enum {
  max_width = 35,
  max_height = 35,
  max_players = 10,
  response_time_ms = 90,
};

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
  grid[point.y][point.x] = value;
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

typedef enum {
  first_dir = 0,
  up_dir = 0,
  left_dir = 1,
  right_dir = 2,
  down_dir = 3,
  last_dir = 3
} PointDir;

const char *directions[4] = {[up_dir] = "up",
                             [left_dir] = "left",
                             [right_dir] = "right",
                             [down_dir] = "down"};

typedef Point (*PointGen)(const Point);
const PointGen dir_funcs[5] = {[up_dir] = point_up,
                               [left_dir] = point_left,
                               [right_dir] = point_right,
                               [down_dir] = point_down};

typedef struct {
  Point monsters[max_players - 1];
  int monsters_count;
  Point explorer;
  CellType grid[max_height][max_width];
} GameState;

void dump_mob_stat(const MobStat mob_stat, const GameState *game_state) {
  fprintf(stderr, "mob stat: ");
  for (int i = 0; i < game_state->monsters_count; ++i) {
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
bool is_lame_proximity_check(const Point point, const GameState *game_state,
                             bool trace) {
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

bool cell_type_is(const CellType grid[max_height][max_width], Point point,
                  CellType value) {
  return grid[point.y][point.x] == value;
}

CellType get_cell_type(const GameState *game_state, const Point point) {
  return game_state->grid[point.y][point.x];
}

bool target_for_bfs_move(const Point point, const GameState *game_state,
                         bool trace) {
  return cell_type_is(game_state->grid, point, ct_unknown) &&
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
  const bool result = !cell_type_is(game_state->grid, point, ct_wall) &&
                      !is_enemy_at(point, game_state, trace);
  if (trace) {
    fprintf(stderr, " iafe: %d %d ", result, get_cell_type(game_state, point));
  }
  return result;
}

bool is_allowed_for_enemy_predict(const Point point,
                                  const GameState *game_state, bool trace) {
  const bool result = cell_type_is(game_state->grid, point, ct_space) ||
                      is_enemy_at(point, game_state, trace);
  if (trace) {
    fprintf(stderr, " iafep: %d %d ", result, get_cell_type(game_state, point));
  }
  return result;
}

bool is_allowed_for_enemy_move(const Point point, const GameState *game_state,
                               bool trace) {
  const bool result = cell_type_is(game_state->grid, point, ct_space);
  if (trace) {
    fprintf(stderr, " iaem: %d %d %d ", result,
            get_cell_type(game_state, point), game_state->monsters_count);
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
    for (PointDir i = first_dir; i <= last_dir; ++i) {
      pretender = dir_funcs[i](current.point);
      if (is_allowed_point(pretender, game_state, trace)) {
        if (trace) {
          fprintf(stderr, ",%s %d,%d", directions[i], pretender.x, pretender.y);
        }
        queue_push(&queue, create(pretender, current));
      }
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
        const CellType cell_type = game_state->grid[i][j];
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
    fprintf(stderr, "%d,%d->%d,%d is left\n", from.x, from.y, to.x, to.y);
    return move_left;
  } else if (point_equals(point_right(from), to)) {
    fprintf(stderr, "%d,%d->%d,%d is right\n", from.x, from.y, to.x, to.y);
    return move_right;
  } else if (point_equals(point_up(from), to)) {
    fprintf(stderr, "%d,%d->%d,%d is up\n", from.x, from.y, to.x, to.y);
    return move_up;
  } else if (point_equals(point_down(from), to)) {
    fprintf(stderr, "%d,%d->%d,%d is down\n", from.x, from.y, to.x, to.y);
    return move_down;
  } else if (point_equals(from, to)) {
    fprintf(stderr, "%d,%d->%d,%d is stay\n", from.x, from.y, to.x, to.y);
    return move_wait;
  }
  abort();
}

int sum_mob_stat(const MobStat mob_stat, const GameState *game_state) {
  int result = 0;
  for (int i = 0; i < game_state->monsters_count; ++i) {
    int dist = mob_stat.dist[i];
    if (dist >= 0) {
      result += dist;
    }
  }
  return result;
}

int min_mob_stat(const MobStat mob_stat, const GameState *game_state) {
  int result = INFINITY;
  for (int i = 0; i < game_state->monsters_count; ++i) {
    int dist = mob_stat.dist[i];
    if (dist >= 0 && dist < result) {
      result = dist;
    }
  }
  return result;
}

bool is_any_neighbours_contain_enemy(const Point point, GameState *game_state) {
  for (PointDir i = first_dir; i <= last_dir; ++i) {
    const Point pretender = dir_funcs[i](point);
    if (is_enemy_at(pretender, game_state, false)) {
      return true;
    }
  }
  return false;
}

Point random_step(const Point point, const GameState *game_state) {
  Point moves[5];
  int moves_count = 0;

  for (PointDir i = first_dir; i <= last_dir; ++i) {
    const Point current_point = dir_funcs[i](point);
    if (cell_type_is(game_state->grid, current_point, ct_space)) {
      moves[moves_count++] = current_point;
    }
  }
  moves[moves_count++] = point;

  if (moves_count == 1) {
    return moves[0];
  }

  const int current_index = rand() % moves_count;

  return moves[current_index];
}

int simulate_turns(Point pretender, GameState *game_state, int max_depth) {
  assert(get_cell_type(game_state, pretender) == ct_space);

  if (max_depth == 0) {
    return 0;
  }

  if (is_any_neighbours_contain_enemy(pretender, game_state)) {
    return 0;
  }

  for (int i = 0; i < game_state->monsters_count; ++i) {
    const Point monster = game_state->monsters[i];
    lame_proximity_check = pretender;
    const QueueItem next_monster_step_struct =
        bfs(monster, game_state, &is_lame_proximity_check,
            &is_allowed_for_enemy_move, false);
    if (is_valid_queue_item(next_monster_step_struct)) {
      game_state->monsters[i] = next_monster_step_struct.first_move;
    } else {
      game_state->monsters[i] =
          random_step(game_state->monsters[i], game_state);
    }
  }

  if (is_any_neighbours_contain_enemy(pretender, game_state)) {
    return 0;
  }

  const Point next_pretender = random_step(pretender, game_state);

  return 1 + simulate_turns(next_pretender, game_state, max_depth - 1);
}

Point alter_move_2(clock_t start_t, const GameState *real_game_state) {

  Point moves[5];
  int attempts[5] = {0};
  int scorings[5] = {0};
  int moves_count = 0;

  for (PointDir i = first_dir; i <= last_dir; ++i) {
    const Point current_point = dir_funcs[i](real_game_state->explorer);
    if (cell_type_is(real_game_state->grid, current_point, ct_space)) {
      moves[moves_count++] = current_point;
    }
  }
  if (moves_count == 1) {
    return moves[0];
  }

  while (has_time(start_t, clock(), response_time_ms)) {
    const int current_index = rand() % moves_count;
    Point first_move = moves[rand() % moves_count];
    GameState temp_game_state = *real_game_state;
    const int scoring = simulate_turns(first_move, &temp_game_state, 10);
    attempts[current_index]++;
    scorings[current_index] += scoring;
  }

  int pretender_index = -1;
  double pretender_scoring = -1.;
  for (PointDir i = first_dir; i <= last_dir; ++i) {
    if (attempts[i] == 0) {
      continue;
    }

    const double scoring = ((double)scorings[i]) / attempts[i];
    fprintf(stderr, "esc2 %d,%d %d attempts, scoring %f\n", moves[i].x,
            moves[i].y, attempts[i], scoring);
    if (scoring > pretender_scoring) {
      pretender_scoring = scoring;
      pretender_index = i;
    }
  }

  assert(pretender_index >= 0);
  Point pretender = moves[pretender_index];

  fprintf(stderr, "escaper2 suggest %d,%d with scoring %f (%d attempts)\n",
          pretender.x, pretender.y, pretender_scoring,
          attempts[pretender_index]);
  return pretender;
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
    if (cell_type_is(game_state->grid, current_point, ct_space)) {

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
        // const int scoring =  min*100000 + sum;
        const int scoring = sum;
        if (scoring > current_scoring) {
          current_scoring = scoring;
          pretender_index = i;
        }
      }
    }
  }

  if (pretender_index != -1) {
    fprintf(stderr, "escaper suggest %d,%d with scoring %d\n",
            moves[pretender_index].x, moves[pretender_index].y,
            current_scoring);
    return moves[pretender_index];
  } else {
    fprintf(stderr, "escaper suggest STAY\n");
    return moves[0];
  }
}

MoveType do_move(clock_t start_t, const GameState *game_state) {
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

  const bool is_valid = is_valid_queue_item(enemy_checker);

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

  if (is_valid && enemy_dist <= 12) {
    return from_points_to_move(game_state->explorer,
                               alter_move_2(start_t, game_state));
  } else {
    return from_points_to_move(game_state->explorer, suggested_by_bfs);
  }
}

void do_and_print_move(clock_t start_t, GameState *game_state, int turn_num) {
  const char move = do_move(start_t, game_state);
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

  //unsigned int seed = (unsigned int)time(NULL);
  unsigned int seed = (unsigned int)1739300101;
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
        set_cell_type(stable_grid, point_right(pos),
                      from_char(right_status[0]));
        set_cell_type(stable_grid, point_left(pos), from_char(left_status[0]));
      } else {
        set_cell_type(stable_grid, pos, ct_space);
        game_state.monsters[i] = pos;
      }
    }
    update_grid(&game_state, stable_grid);

    do_and_print_move(clock(), &game_state, turn_num);
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
          state->grid[y][x] = ct_space;
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
  do_and_print_move(clock(), &state, -1);
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