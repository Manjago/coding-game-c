#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

/*

// todo

u _ r # d # l _
0: 3 20 -> 3 1
1: 5 1 -> 5 1
2: 17 8 -> 17 8
3: 7 22 -> 7 3
4: 3 22 -> 3 3
#...#???.......??.??????
#.#0#1??..........??????
#.#.#.??.?????.??.??????
#..4#.?3..........??????
#..#..??.?.???....??????
#.#.#.??.?.....??.??????
#.#.#.??.?????.??.??????
#.#.#.??.......??.??????
#...#?????????.??2??????
#.#.#....?.....?????????
#...#.??.?.???.?????????
#.#.#.??.?..??.?????????
#.#.#.??.?????.?????????
#...#.??.......?????????
#.#.#.???????.??????????
#...#.???????.??????????
#.#.#.....???.....??????
#.#.#????.???.???.??????
#.#.#????.???.???.??????
from 3,3 bfs suggest -1,-1

обработай случай, когда враг закупорил и некуда децца (ну тогда побег от врага
или ждать)

// todo

u _ r _ d _ l #
0: 21 5 -> 21 5
1: 6 5 -> 6 5
2: 11 17 -> 11 17
3: 16 17 -> 16 17
4: 9 19 -> 9 19
???????????????????????????????????
???????????????????????????????????
???????????????????????????????????
???????????????????????????????????
???????????????????????????????????
??????1??????????????0?????????????
??????.??????????????.?????????????
??????....????????....?????????????
?????????.????????.????????????????
?????????.????????.????????????????
?????????....??....????????????????
????????????.??.???????????????????
????????????.??.???????????????????
????????????....???????????????????
??????????????.????????????????????
???????????......??????????????????
???????????????????????????????????
?????????.?2????3??????????????????
????????#.#????????????????????????
????????#4..???????????????????????
????????#.#????????????????????????
????????#.###??????????????????????
????????.....#?????????????????????
?????????###.#?????????????????????
???????????#.#?????????????????????
???????????....????????????????????
????????????##?????????????????????
???????????????????????????????????
from 9,19 bfs suggest 9,18
turn 21, move UP

обработай случай, когда пугают зря (ну или в бфс занести, чтобы такие не
рассматривать просто)


// todo

даже не пытайся убить врага - здесь это не выйдет, только убегать

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
int players_count;

#define MAX_Q_SIZE max_height *max_width

enum move_type {
  move_right = 'A',
  move_wait = 'B',
  move_up = 'C',
  move_down = 'D',
  move_left = 'E'
};

void decode_move(const enum move_type mt) {
  switch (mt) {
  case move_right:
    fprintf(stderr, "RIGHT");
    break;
  case move_wait:
    fprintf(stderr, "WAIT");
    break;
  case move_up:
    fprintf(stderr, "UP");
    break;
  case move_down:
    fprintf(stderr, "DOWN");
    break;
  case move_left:
    fprintf(stderr, "LEFT");
    break;
  }
}

enum cell_type { unknown = 0, free_cell = 1, wall = 2 };

struct point {
  int x, y;
};

const struct point undefined_point = {-1, -1};

struct queue_item {
  struct point point;
  struct point first_move;
};

struct queue {
  struct queue_item queue[MAX_Q_SIZE];
  int front;
  int back;
};

void queue_init(struct queue *q) {
  q->front = -1;
  q->back = -1;
}

bool queue_is_empty(const struct queue *q) { return (q->front == -1); }

void queue_push(struct queue *q, struct queue_item data) {
  assert(q->back < (MAX_Q_SIZE - 1));
  if (queue_is_empty(q)) {
    q->front = 0;
  }
  q->back++;
  q->queue[q->back] = data;
}

struct queue_item queue_pop(struct queue *q) {
  assert(!queue_is_empty(q));
  struct queue_item data = q->queue[q->front];
  if (q->front == q->back) {
    q->front = -1;
    q->back = -1;
  } else {
    q->front++;
  }
  return data;
}

struct point players[max_players];
enum cell_type grid[max_height][max_width] = {0};

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
        switch (grid[i][j]) {
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

bool point_equals(const struct point a, const struct point b) {
  return a.x == b.x && a.y == b.y;
}

struct point point_up(const struct point point) {
  struct point result = point;
  result.y = up_index(result.y);
  return result;
}

struct point point_down(const struct point point) {
  struct point result = point;
  result.y = down_index(result.y);
  return result;
}

struct point point_left(const struct point point) {
  struct point result = point;
  result.x = left_index(result.x);
  return result;
}

struct point point_right(const struct point point) {
  struct point result = point;
  result.x = right_index(result.x);
  return result;
}

enum cell_type up(int x, int y) { return grid[up_index(y)][x]; }

enum cell_type down(int x, int y) { return grid[down_index(y)][x]; }

enum cell_type left(int x, int y) { return grid[y][left_index(x)]; }
enum cell_type right(int x, int y) { return grid[y][right_index(x)]; }

struct queue_item create(const struct point pretender,
                         const struct queue_item prev) {

  struct queue_item result;
  result.point = pretender;
  if (point_equals(undefined_point, prev.first_move)) {
    result.first_move = pretender;
  } else {
    result.first_move = prev.first_move;
  }
  return result;
}

struct point bfs(struct point start) {
  int seen[max_height][max_width] = {0};
  struct queue queue;
  queue_init(&queue);
  struct queue_item start_queue_item = {start, undefined_point};
  queue_push(&queue, start_queue_item);
  while (!queue_is_empty(&queue)) {
    struct queue_item current = queue_pop(&queue);
    if (seen[current.point.y][current.point.x]) {
      continue;
    }
    seen[current.point.y][current.point.x] = 1;

    if (grid[current.point.y][current.point.x] == unknown) {
      return current.first_move;
    }

    struct point pretender;

    pretender = point_up(current.point);
    if (grid[pretender.y][pretender.x] != wall) {
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_down(current.point);
    if (grid[pretender.y][pretender.x] != wall) {
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_left(current.point);
    if (grid[pretender.y][pretender.x] != wall) {
      queue_push(&queue, create(pretender, current));
    }
    pretender = point_right(current.point);
    if (grid[pretender.y][pretender.x] != wall) {
      queue_push(&queue, create(pretender, current));
    }
  }

  return undefined_point;
}

enum move_type from_points_to_move(const struct point from,
                                   const struct point to) {
  if (point_equals(point_left(from), to)) {
    return move_left;
  } else if (point_equals(point_right(from), to)) {
    return move_right;
  } else if (point_equals(point_up(from), to)) {
    return move_up;
  } else if (point_equals(point_down(from), to)) {
    return move_down;
  }
  abort();
}

bool is_enemy_at(const struct point point) {
  for (int i = 0; i < players_count - 1; ++i) {
    if (point_equals(point, players[i])) {
      fprintf(stderr, "enemy at %d,%d\n", point.x, point.y);
      return true;
    }
  }
  return false;
}

bool is_will_be_killed(const struct point point, bool external) {

  struct point check;

  check = point_up(point);
  if (is_enemy_at(check)) {
    return true;
  }
  if (external && is_will_be_killed(check, false)) {
    return true;
  }

  check = point_down(point);
  if (is_enemy_at(check)) {
    return true;
  }
  if (external && is_will_be_killed(check, false)) {
    return true;
  }

  check = point_left(point);
  if (is_enemy_at(check)) {
    return true;
  }
  if (external && is_will_be_killed(check, false)) {
    return true;
  }

  check = point_right(point);
  if (is_enemy_at(check)) {
    return true;
  }
  if (external && is_will_be_killed(check, false)) {
    return true;
  }

  return false;
}

enum move_type do_move(const struct point me) {
  const struct point target_point = bfs(me);
  fprintf(stderr, "from %d,%d bfs suggest %d,%d\n", me.x, me.y, target_point.x,
          target_point.y);

  if (!is_will_be_killed(target_point, true)) {
    const enum move_type suggested_move = from_points_to_move(me, target_point);
    return suggested_move;
  }

  fprintf(stderr, "enemy wanna kill me at %d,%d\n", target_point.x,
          target_point.y);

  struct point just_escape;

  just_escape = point_up(me);
  if ((grid[just_escape.y][just_escape.x] == free_cell) &&
      !is_will_be_killed(just_escape, true)) {
    const enum move_type suggested_move = from_points_to_move(me, just_escape);
    fprintf(stderr, "escape from %d,%d to %d,%d\n", me.x, me.y, just_escape.x,
            just_escape.y);
    return suggested_move;
  }

  just_escape = point_down(me);
  if ((grid[just_escape.y][just_escape.x] == free_cell) &&
      !is_will_be_killed(just_escape, true)) {
    const enum move_type suggested_move = from_points_to_move(me, just_escape);
    fprintf(stderr, "escape from %d,%d to %d,%d\n", me.x, me.y, just_escape.x,
            just_escape.y);
    return suggested_move;
  }

  just_escape = point_left(me);
  if ((grid[just_escape.y][just_escape.x] == free_cell) &&
      !is_will_be_killed(just_escape, true)) {
    const enum move_type suggested_move = from_points_to_move(me, just_escape);
    fprintf(stderr, "escape from %d,%d to %d,%d\n", me.x, me.y, just_escape.x,
            just_escape.y);
    return suggested_move;
  }

  just_escape = point_right(me);
  if ((grid[just_escape.y][just_escape.x] == free_cell) &&
      !is_will_be_killed(just_escape, true)) {
    const enum move_type suggested_move = from_points_to_move(me, just_escape);
    fprintf(stderr, "escape from %d,%d to %d,%d\n", me.x, me.y, just_escape.x,
            just_escape.y);
    return suggested_move;
  }

  fprintf(stderr, "just wait\n");
  return move_wait;
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
  scanf("%d", &players_count);
  assert(players_count > 0 && players_count <= max_players);
  fgetc(stdin);

  unsigned int seed = (unsigned int)time(NULL);
  fprintf(stderr, "ver 1.6.0, seed = %u\n", seed);
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
      grid[y_mod][x_mod] = free_cell;
      if (i == players_count - 1) {
        grid[up_index(y_mod)][x_mod] = from_char(up_status[0]);
        grid[down_index(y_mod)][x_mod] = from_char(down_status[0]);
        grid[y_mod][right_index(x_mod)] = from_char(right_status[0]);
        grid[y_mod][left_index(x_mod)] = from_char(left_status[0]);
      }
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // printf("A, B, C, D or E\n");
    dump_grid(players_count);
    const struct point me = players[players_count - 1];
    const char move = do_move(me);
    fprintf(stderr, "turn %d, move ", turn_num);
    decode_move(move);
    fprintf(stderr, "\n");
    printf("%c\n", move);
  }

  return 0;
}