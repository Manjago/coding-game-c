/*
Inspired by
https://www.codingame.com/forum/t/code-vs-zombies-feedback-strategies/1089/37
*/

#define MY_TEST 1

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FIBO_SIZE (int)(sizeof(fibo) / sizeof(fibo[0]))

long get_fibo(int num) {

  static long fibo[] = {
      1,        2,        3,        5,         8,         13,       21,
      34,       55,       89,       144,       233,       377,      610,
      987,      1597,     2584,     4181,      6765,      10946,    17711,
      28657,    46368,    75025,    121393,    196418,    317811,   514229,
      832040,   1346269,  2178309,  3524578,   5702887,   9227465,  14930352,
      24157817, 39088169, 63245986, 102334155, 165580141, 267914296};

  if (num >= FIBO_SIZE) {
    return fibo[FIBO_SIZE - 1];
  } else if (num < 0) {
    return fibo[0];
  } else {
    return fibo[num];
  }
}

int index_by_value(int arr[], int size, int value) {
  for (int i = 0; i < size; ++i) {
    if (arr[i] == value) {
      return i;
    }
  }
  return -1;
}

int sum_ones(int arr[], int size) {
  int sum = 0;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum;
}

enum constraints {
  max_human = 99,
  max_zombie = 99,
  response_time_ms = 90,
  max_x_exclusive = 16000,
  max_y_exclusive = 9000,
  max_ash_move = 1000,
  kill_dist_2 = 4000000
};

struct point {
  int x, y;
};

int dist2(struct point a, struct point b) {
  const int dx = a.x - b.x;
  const int dy = a.y - b.y;
  return dx * dx + dy * dy;
}

bool point_equals(const struct point a, const struct point b) {
  return a.x == b.x && a.y == b.y;
}

struct game_state {
  struct point ash;
  int human_count;
  int human_id[max_human];
  struct point human[max_human];
  int zombie_count;
  int zombie_id[max_zombie];
  struct point zombie[max_zombie];
  struct point zombie_next[max_zombie];
};

struct point find_nearest(const struct point tested, const struct point arr[],
                          int size) {
  int answer_dist = -1;
  int answer_index = -1;
  for (int i = 0; i < size; ++i) {
    const int current_dist = dist2(tested, arr[i]);
    if (answer_dist == -1 || current_dist < answer_dist) {
      answer_dist = current_dist;
      answer_index = i;
    }
  }

  if (answer_index == -1) {
    return tested;
  } else {
    return arr[answer_index];
  }
}

struct point find_nearest_human(const struct game_state *state,
                                int zombie_index) {
  int answer_dist = -1;
  int answer_human_index = -1;
  for (int i = 0; i < state->human_count; ++i) {
    int current_dist = dist2(state->zombie[zombie_index], state->human[i]);
    if (answer_dist == -1 || current_dist < answer_dist) {
      current_dist = answer_dist;
      answer_human_index = i;
    }
  }

  if (answer_human_index == -1) {
    return state->zombie[zombie_index];
  } else {
    return state->human[answer_human_index];
  }
}

struct strategy {
  int random_moves_count;
  int target_zombie_id;
  struct point first_move;
};

double elapsed(clock_t start_t, clock_t end_t) {
  // fprintf(stderr, "clock %ld %ld %ld\n", start_t, end_t, CLOCKS_PER_SEC);
  return (double)(end_t - start_t) / CLOCKS_PER_SEC * 1000;
}

void dump_game_state_ash(const struct game_state *state) {
  fprintf(stderr, "Ash: (%d,%d)\n", state->ash.x, state->ash.x);
}

void dump_game_state_humans(const struct game_state *state) {
  fprintf(stderr, "H: ");
  for (int i = 0; i < state->human_count; ++i) {
    if (i != state->human_count - 1) {
      fprintf(stderr, "%d:(%d,%d),", state->human_id[i], state->human[i].x,
              state->human[i].y);
    } else {
      fprintf(stderr, "%d:(%d,%d)\n", state->human_id[i], state->human[i].x,
              state->human[i].y);
    }
  }
}

void dump_game_state_zombies(const struct game_state *state) {
  fprintf(stderr, "Z: ");
  for (int i = 0; i < state->zombie_count; ++i) {
    if (i != state->zombie_count - 1) {
      fprintf(stderr, "%d:(%d,%d)->(%d,%d),", state->zombie_id[i],
              state->zombie[i].x, state->zombie[i].y, state->zombie_next[i].x,
              state->zombie_next[i].y);
    } else {
      fprintf(stderr, "%d:(%d,%d)->(%d,%d)\n", state->zombie_id[i],
              state->zombie[i].x, state->zombie[i].y, state->zombie_next[i].x,
              state->zombie_next[i].y);
    }
  }
}

void dump_game_state(const struct game_state *state) {
  dump_game_state_ash(state);
  dump_game_state_humans(state);
  dump_game_state_zombies(state);
}

bool has_time(const clock_t start_t) {
  clock_t end_t = clock();
  double elapsed_ms = elapsed(start_t, end_t);
  return elapsed_ms < response_time_ms;
}

struct point move_from_destination(struct point from, struct point to) {
  int a = from.x - to.x;
  int b = from.y - to.y;
  double real_dist = sqrt(a * a + b * b);
  if (real_dist <= max_ash_move) {
    return to;
  };

  double coeff = real_dist / max_ash_move;
  int a_mod = (int)(a / coeff);
  int b_mod = (int)(b / coeff);
  struct point result = {from.x + a_mod, from.y + b_mod};
  return result;
}

/*
repeat X times (X chosen randomly between 0 and 3)
    move to a random position
for each
zombie still alive (the order of the zombies is also chosen randomly)
    move toward the zombie until he's killed
*/
void generate_a_random_strategy(const struct game_state *state,
                                struct strategy *result) {
  const int x = rand() % 4;
  result->random_moves_count = x;
  const int zombie_index = rand() % state->zombie_count;
  result->target_zombie_id = zombie_index;
}

long simulate_turn(struct game_state *simulated_state,
                   const struct point ash_move) {
  /*
  1. Zombies move towards their targets.
  2. Ash moves towards his target.
  3. Any zombie within a 2000 unit range around Ash is destroyed.
  4. Zombies eat any human they share coordinates with.
  */

  /* step 1 */
  for (int i = 0; i < simulated_state->zombie_count; ++i) {
    const struct point nearest_human = find_nearest_human(simulated_state, i);
    const struct point dest =
        move_from_destination(simulated_state->zombie[i], nearest_human);
    simulated_state->zombie[i] = dest;
  }

  /* step 2 */
  simulated_state->ash = ash_move;

  /* step 3 */
  int killed_zombie_index[simulated_state->zombie_count];

  for (int i = 0; i < simulated_state->zombie_count; ++i) {
    const double kill_dist =
        dist2(simulated_state->ash, simulated_state->zombie[i]);
    if (kill_dist <= kill_dist_2) {
      killed_zombie_index[i] = 1;
    } else {
      killed_zombie_index[i] = 0;
    }
  }

  /* step 4 */
  int killed_human_index[simulated_state->human_count];
  memset(killed_human_index, 0,
         sizeof(int) * (size_t)simulated_state->human_count);

  for (int i = 0; i < simulated_state->zombie_count; ++i) {
    if (killed_zombie_index[i]) {
      continue;
    }
    for (int j = 0; j < simulated_state->human_count; ++j) {
      if (point_equals(simulated_state->human[j], simulated_state->zombie[i])) {
        killed_human_index[j] = 1;
      }
    }
  }

  /* step 5 calc scoring */
  const int zombie_killed =
      sum_ones(killed_zombie_index, simulated_state->zombie_count);
  const int human_killed =
      sum_ones(killed_human_index, simulated_state->human_count);

  long scoring = 0;
  for (int i = 0; i < zombie_killed; ++i) {
    long worth =
        get_fibo(i) * (simulated_state->human_count - human_killed) * 10;
    scoring += worth;
  }

  /* step 6 recalc zombie and humans */
  if (zombie_killed > 0) {
    int read_index = 0;
    int write_index = 0;
    while (read_index < simulated_state->zombie_count) {
      if (killed_zombie_index[read_index]) {
        read_index++;
      } else {
        simulated_state->zombie[write_index] =
            simulated_state->zombie[read_index];
        simulated_state->zombie_id[write_index] =
            simulated_state->zombie_id[read_index];
        read_index++;
        write_index++;
      }
    }
    simulated_state->zombie_count = write_index;
  }

  if (human_killed > 0) {
    int read_index = 0;
    int write_index = 0;
    while (read_index < simulated_state->human_count) {
      if (killed_human_index[read_index]) {
        read_index++;
      } else {
        simulated_state->human[write_index] =
            simulated_state->human[read_index];
        simulated_state->human_id[write_index] =
            simulated_state->human_id[read_index];
        read_index++;
        write_index++;
      }
    }
    simulated_state->human_count = write_index;
  }

  return scoring;
}

long simulate_the_strategy(const struct game_state *initial_state,
                           struct strategy *result) {

  long scoring = 0;

  struct game_state simulated_state = *initial_state;
  for (int i = 0; i < result->random_moves_count; ++i) {
    const struct point random_dest = {rand() % max_x_exclusive,
                                      rand() % max_y_exclusive};
    const struct point actual_dest =
        move_from_destination(simulated_state.ash, random_dest);
    if (i == 0) {
      result->first_move = actual_dest;
    }
    simulate_turn(&simulated_state, actual_dest);
  }

  int target_zombie_index;
  while ((target_zombie_index = index_by_value(
              simulated_state.zombie_id, simulated_state.zombie_count,
              result->target_zombie_id)) != -1) {
    const struct point go_to = simulated_state.zombie[target_zombie_index];

    const struct point actual_dest =
        move_from_destination(simulated_state.ash, go_to);

    if (result->random_moves_count == 0) {
      result->first_move = actual_dest;
    }

    simulate_turn(&simulated_state, actual_dest);
  }

  return scoring;
}

struct point apply_the_first_move(const struct strategy *strategy) {
  return strategy->first_move;
}

void sendMove(const struct point move) { printf("%d %d\n", move.x, move.y); }

/*
while there is some time left
    generate a random strategy
    simulate the strategy
apply the first move of the best strategy seen so far
*/
void move2(const struct game_state *actual_state,
           const struct strategy *initial_strategy, const clock_t start_t) {
  struct strategy current_strategy = *initial_strategy;
  long current_scoring = -1;
  struct strategy pretender_strategy;

  while (has_time(start_t)) {
    generate_a_random_strategy(actual_state, &pretender_strategy);
    long scoring = simulate_the_strategy(actual_state, &pretender_strategy);
    if (scoring > current_scoring) {
      current_scoring = scoring;
      current_strategy = pretender_strategy;
    };
  }

  const struct point move = apply_the_first_move(&pretender_strategy);
  sendMove(move);
}

void move(const struct game_state *src) {
  clock_t start_t, end_t;
  start_t = clock();
  dump_game_state(src);
  printf("0 0\n");
  end_t = clock();
  fprintf(stderr, "elapsed: %.3f ms\n", elapsed(start_t, end_t));
}

void game_loop() {
  srand((unsigned int)time(NULL));

  struct game_state game_state;
  int zombie_count_at_start = -1;
  struct strategy do_nothing_strategy;
  do_nothing_strategy.random_moves_count = 0;
  do_nothing_strategy.target_zombie_id = -1;

  // game loop
  while (1) {
    int x;
    int y;
    scanf("%d%d", &x, &y);
    game_state.ash.x = x;
    game_state.ash.y = y;

    int human_count;
    scanf("%d", &human_count);
    game_state.human_count = human_count;

    for (int i = 0; i < human_count; i++) {
      int human_id;
      int human_x;
      int human_y;
      scanf("%d%d%d", &human_id, &human_x, &human_y);
      game_state.human_id[i] = human_id;
      game_state.human[i].x = human_x;
      game_state.human[i].y = human_y;
    }

    int zombie_count;
    scanf("%d", &zombie_count);
    game_state.zombie_count = zombie_count;
    if (zombie_count_at_start != -1) {
      zombie_count_at_start = zombie_count;
    }

    for (int i = 0; i < zombie_count; i++) {
      int zombie_id;
      int zombie_x;
      int zombie_y;
      int zombie_xnext;
      int zombie_ynext;
      scanf("%d%d%d%d%d", &zombie_id, &zombie_x, &zombie_y, &zombie_xnext,
            &zombie_ynext);
      game_state.zombie_id[i] = zombie_id;
      game_state.zombie[i].x = zombie_x;
      game_state.zombie[i].y = zombie_y;
      game_state.zombie_next[i].x = zombie_xnext;
      game_state.zombie_next[i].y = zombie_ynext;
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    // move(&game_state);
    move2(&game_state, &do_nothing_strategy, clock());
  }
}

void test_get_fibo() {
  assert(1 == get_fibo(-10));
  assert(1 == get_fibo(-1));
  assert(1 == get_fibo(0));
  assert(2 == get_fibo(1));
  assert(165580141 == get_fibo(39));
  assert(267914296 == get_fibo(40));
  assert(267914296 == get_fibo(41));
  assert(267914296 == get_fibo(50));
}

void test_dist2() {
  const struct point a = {0, 0};
  const struct point b = {3, 4};
  assert(25 == dist2(a, b));
  assert(25 == dist2(b, a));

  const struct point c = {1, 2};
  const struct point d = {5, 7};
  assert(41 == dist2(c, d));
  assert(41 == dist2(d, c));

  const struct point e = {-1, 2};
  const struct point f = {3, -4};
  assert(52 == dist2(e, f));
  assert(52 == dist2(f, e));

  const struct point g = {0, 0};
  const struct point h = {0, 0};
  assert(0 == dist2(g, h));
  assert(0 == dist2(h, g));
}

void test_point_equals() {
  const struct point a = {1, 2};
  const struct point b = {3, 4};
  assert(!point_equals(a, b));

  const struct point c = {5, 6};
  const struct point d = {5, 6};
  assert(point_equals(c, d));

  const struct point e = {5, 6};
  const struct point f = {6, 5};
  assert(!point_equals(e, f));

  const struct point g = {-1, 2};
  const struct point h = {-1, 2};
  assert(point_equals(g, h));

  const struct point i = {0, 0};
  const struct point j = {0, 0};
  assert(point_equals(i, j));
}

void test_index_by_value() {
  int a[4] = {0, 2, 2, 1};
  assert(1 == index_by_value(a, 4, 2));
  assert(0 == index_by_value(a, 4, 0));
  assert(3 == index_by_value(a, 4, 1));
  assert(-1 == index_by_value(a, 4, 3));

  int b[0] = {};
  assert(-1 == index_by_value(b, 0, 42));

  int c[5] = {1, 2, 3, 4, 5};
  assert(-1 == index_by_value(c, 5, 0));

  int d[3] = {2, 2, 2};
  assert(0 == index_by_value(d, 3, 2));
}

void test_sum_ones() {
  int a[4] = {1, 0, 1, 1};
  assert(3 == sum_ones(a, 4));

  int b[0] = {};
  assert(0 == sum_ones(b, 0));

  int c[5] = {0, 0, 0, 0, 0};
  assert(0 == sum_ones(c, 5));

  int d[3] = {1, 1, 1};
  assert(3 == sum_ones(d, 3));

  int e[6] = {0, 0, 1, 0, 1, 1};
  assert(3 == sum_ones(e, 6));
}

void test_find_nearest() {
  const struct point a = {10, 10};
  const struct point a_expected = {11, 11};
  const struct point arr_a[3] = {
      {0, 0},
      {11, 11},
      {5, 5},
  };
  assert(point_equals(a_expected, find_nearest(a, arr_a, 3)));

  const struct point b = {0, 0};
  const struct point b_expected = {0, 0};
  const struct point arr_b[1] = {{0, 0}};
  assert(point_equals(b_expected, find_nearest(b, arr_b, 1)));

  const struct point c = {10, 10};
  const struct point arr_c[3] = {
      {10, 10},
      {11, 11},
      {9, 9},
  };
  assert(point_equals(c, find_nearest(c, arr_c, 3)));

  const struct point d = {0, 0};
  const struct point arr_d[0] = {};
  assert(point_equals(d, find_nearest(d, arr_d, 0)));
}

void tests() {
  test_get_fibo();
  test_dist2();
  test_point_equals();
  test_index_by_value();
  test_sum_ones();
  test_find_nearest();
}

int main() {

#ifndef MY_TEST
  game_loop();
#else
  tests();
#endif

  return 0;
}
