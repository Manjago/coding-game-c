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

#define SIZE_OF_INT sizeof(int)
#define FIBO_SIZE (int)(sizeof(fibo) / sizeof(fibo[0]))
#define RAND_SIZE (int)(sizeof(rand_data) / sizeof(rand_data[0]))

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

void zero_array(int arr[], size_t size) { memset(arr, 0, SIZE_OF_INT * size); }

enum constraints {
  max_human = 99,
  max_zombie = 99,
  response_time_ms = 90,
  max_x_exclusive = 16000,
  max_y_exclusive = 9000,
  max_ash_move = 1000,
  max_zombie_move = 400,
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

struct strategy {
  int random_moves_count;
  int target_zombie_id;
  struct point first_move;
};

double elapsed(clock_t start_t, clock_t end_t) {
  const clock_t delta_ticks = end_t - start_t;
  return (double)delta_ticks / CLOCKS_PER_SEC * 1000.0;
}

void dump_game_state_ash(const struct game_state *state) {
  fprintf(stderr, "Ash: (%d,%d)\n", state->ash.x, state->ash.y);
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

bool has_time(const clock_t start_t, const clock_t end_t, int limit_ms) {
  double elapsed_ms = elapsed(start_t, end_t);
  return elapsed_ms <= limit_ms;
}

struct point move_from_destination(const struct point from,
                                   const struct point to, int max_dist) {
  struct point result;
  const int delta_x = to.x - from.x;
  const int delta_y = to.y - from.y;
  const double real_dist = hypot(delta_x, delta_y);
  if (real_dist <= max_dist) {
    result = to;
  } else {
    const double coeff = real_dist / max_dist;
    const double real_delta_x_double = delta_x / coeff;
    const double real_delta_y_double = delta_y / coeff;
    const int real_delta_x_int = (int)real_delta_x_double;
    const int real_delta_y_int = (int)real_delta_y_double;
    result.x = from.x + real_delta_x_int;
    result.y = from.y + real_delta_y_int;
  }

  return result;
}

/*
repeat X times (X chosen randomly between 0 and 3)
    move to a random position
for each
zombie still alive (the order of the zombies is also chosen randomly)
    move toward the zombie until he's killed
*/
void generate_a_random_strategy(int zombie_count, struct strategy *result,
                                int (*frand)()) {
  assert(0 != zombie_count);
  const int x = (*frand)() % 4;
  result->random_moves_count = x;
  const int zombie_index = (*frand)() % zombie_count;
  result->target_zombie_id = zombie_index;
}

void zombies_move_towards_their_targets(struct game_state *simulated_state) {
  for (int i = 0; i < simulated_state->zombie_count; ++i) {
    simulated_state->zombie[i] = simulated_state->zombie_next[i];
  }
}

void ash_moves_towards_his_target(struct game_state *simulated_state,
                                  const struct point ash_move) {
  simulated_state->ash = ash_move;
}

void any_zombie_in_range_is_destroyed(struct game_state *simulated_state,
                                      int critical_dist_2,
                                      int killed_zombie_index[]) {
  for (int i = 0; i < simulated_state->zombie_count; ++i) {
    const double kill_dist =
        dist2(simulated_state->ash, simulated_state->zombie[i]);
    if (kill_dist <= critical_dist_2) {
      killed_zombie_index[i] = 1;
    }
  }
}

void zombies_eat_human(struct game_state *simulated_state,
                       int killed_zombie_index[], int killed_human_index[]) {
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
}

struct scoring_and_kill_stat {
  long scoring;
  int zombie_killed;
  int human_killed;
};

struct scoring_and_kill_stat calc_scoring(struct game_state *simulated_state,
                                          int killed_zombie_index[],
                                          int killed_human_index[]) {
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

  struct scoring_and_kill_stat result;
  result.scoring = scoring;
  result.zombie_killed = zombie_killed;
  result.human_killed = human_killed;
  return result;
}

int vacuum(int arr_to_remove[], struct point arr_data[], int arr_id[],
           int size) {
  int read_index = 0;
  int write_index = 0;
  while (read_index < size) {
    if (!arr_to_remove[read_index]) {
      arr_data[write_index] = arr_data[read_index];
      arr_id[write_index++] = arr_id[read_index];
    }
    read_index++;
  }
  return write_index;
}

void calc_zombie_next_point(struct game_state *simulated_state,
                            int zombie_move_len) {
  for (int i = 0; i < simulated_state->zombie_count; ++i) {

    const struct point nearest_human =
        find_nearest(simulated_state->zombie[i], simulated_state->human,
                     simulated_state->human_count);

    const struct point dest = move_from_destination(
        simulated_state->zombie[i], nearest_human, zombie_move_len);
    simulated_state->zombie_next[i] = dest;
  }
}

long simulate_turn(struct game_state *simulated_state,
                   const struct point ash_move, int critical_dist_2,
                   int zombie_move_len) {
  /*
  1. Zombies move towards their targets.
  2. Ash moves towards his target.
  3. Any zombie within a 2000 unit range around Ash is destroyed.
  4. Zombies eat any human they share coordinates with.
  5-6-7. tech steps
  */

  /* step 1 */
  zombies_move_towards_their_targets(simulated_state);

  /* step 2 */
  ash_moves_towards_his_target(simulated_state, ash_move);

  /* step 3 */
  assert(simulated_state->zombie_count > 0);
  int killed_zombie_index[simulated_state->zombie_count];
  zero_array(killed_zombie_index, (size_t)simulated_state->zombie_count);

  any_zombie_in_range_is_destroyed(simulated_state, critical_dist_2,
                                   killed_zombie_index);

  /* step 4 */
  int killed_human_index[simulated_state->human_count];
  zero_array(killed_human_index, (size_t)simulated_state->human_count);

  zombies_eat_human(simulated_state, killed_zombie_index, killed_human_index);

  /* step 5 calc scoring */
  const struct scoring_and_kill_stat scoring_info =
      calc_scoring(simulated_state, killed_zombie_index, killed_human_index);

  const long scoring = scoring_info.scoring;
  const int zombie_killed = scoring_info.zombie_killed;
  const int human_killed = scoring_info.human_killed;

  /* step 6 recalc zombie and humans */
  if (zombie_killed > 0) {
    simulated_state->zombie_count =
        vacuum(killed_zombie_index, simulated_state->zombie,
               simulated_state->zombie_id, simulated_state->zombie_count);
  }

  if (human_killed > 0) {
    simulated_state->human_count =
        vacuum(killed_human_index, simulated_state->human,
               simulated_state->human_id, simulated_state->human_count);
  }

  /* step 7 calc zombie next point*/
  calc_zombie_next_point(simulated_state, zombie_move_len);

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
        move_from_destination(simulated_state.ash, random_dest, max_ash_move);
    if (i == 0) {
      result->first_move = actual_dest;
    }
    simulate_turn(&simulated_state, actual_dest, kill_dist_2, max_zombie_move);
  }

  int target_zombie_index;
  while ((target_zombie_index = index_by_value(
              simulated_state.zombie_id, simulated_state.zombie_count,
              result->target_zombie_id)) != -1) {
    const struct point go_to = simulated_state.zombie[target_zombie_index];

    const struct point actual_dest =
        move_from_destination(simulated_state.ash, go_to, max_ash_move);

    if (result->random_moves_count == 0) {
      result->first_move = actual_dest;
    }

    simulate_turn(&simulated_state, actual_dest, kill_dist_2, max_zombie_move);
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

  while (has_time(start_t, clock(), response_time_ms)) {
    generate_a_random_strategy(actual_state->zombie_count, &pretender_strategy,
                               &rand);
    long scoring = simulate_the_strategy(actual_state, &pretender_strategy);
    if (scoring > current_scoring) {
      current_scoring = scoring;
      current_strategy = pretender_strategy;
    };
  }

  const struct point move = apply_the_first_move(&current_strategy);
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

  int c[5] = {1, 2, 3, 4, 5};
  assert(-1 == index_by_value(c, 5, 0));

  int d[3] = {2, 2, 2};
  assert(0 == index_by_value(d, 3, 2));
}

void test_sum_ones() {
  int a[4] = {1, 0, 1, 1};
  assert(3 == sum_ones(a, 4));

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
}

void test_elapsed() {
  assert(fabs(elapsed(1, 1)) < 1e-8);

  assert(elapsed(10, 5) < 0);

  clock_t start = clock();
  clock_t end = start + CLOCKS_PER_SEC;
  assert(fabs(elapsed(start, end) - 1000.0) < 10.0);

  start = clock();
  end = start + 5 * CLOCKS_PER_SEC;
  assert(fabs(elapsed(start, end) - 5000.0) < 50.0);

  clock_t min_clock_t = 0;
  clock_t max_clock_t = CLOCKS_PER_SEC * 1000;
  assert(elapsed(min_clock_t, max_clock_t) > 999000.0);
}

void test_has_time() {
  assert(has_time(0, CLOCKS_PER_SEC, 1050));

  assert(!has_time(0, CLOCKS_PER_SEC, 950));

  const clock_t two_seconds = 2 * CLOCKS_PER_SEC;
  assert(has_time(0, two_seconds, 2100));

  const clock_t half_second = CLOCKS_PER_SEC / 2;
  assert(has_time(0, half_second, 600));

  assert(has_time(0, CLOCKS_PER_SEC / 1000, 1));
}

void test_move_from_destination() {
  const struct point a_from = {0, 0};
  const struct point a_to = {10, 0};
  const struct point a_expected = {5, 0};
  assert(point_equals(a_expected, move_from_destination(a_from, a_to, 5)));
  assert(point_equals(a_to, move_from_destination(a_from, a_to, 10)));

  const struct point b_from = {-5, -5};
  const struct point b_to = {10, 10};
  const struct point b_expected = {0, 0};
  assert(point_equals(b_expected, move_from_destination(b_from, b_to, 8)));

  const struct point c_from = {0, 0};
  const struct point c_to = {10, 10};
  const struct point c_expected = {0, 0};
  assert(point_equals(c_expected, move_from_destination(c_from, c_to, 0)));

  const struct point d_from = {5, 5};
  const struct point d_to = {5, 5};
  const struct point d_expected = {5, 5};
  assert(point_equals(d_expected, move_from_destination(d_from, d_to, 10)));

  const struct point e_from = {0, 0};
  const struct point e_to = {5, 5};
  const struct point e_expected = {5, 5};
  assert(point_equals(e_expected, move_from_destination(e_from, e_to, 10)));

  const struct point f_from = {1, 2};
  const struct point f_to = {4, 6};
  const struct point f_expected = {4, 6};
  assert(point_equals(f_to, move_from_destination(f_from, f_to, 10)));
  assert(point_equals(f_expected, move_from_destination(f_from, f_to, 5)));
}

int custom_rand() {
  static int rand_data[2] = {5, 6};
  static int index = 0;
  if (index > RAND_SIZE - 1) {
    index = 0;
  }
  return rand_data[index++];
}

void test_generate_a_random_strategy() {
  struct point not_changed_point = {42, 42};
  struct strategy strategy = {0, 0, not_changed_point};
  generate_a_random_strategy(3, &strategy, &custom_rand);
  assert(1 == strategy.random_moves_count);
  assert(0 == strategy.target_zombie_id);
  assert(point_equals(not_changed_point, strategy.first_move));
}

void test_zero_array() {
  int test_array[3] = {1, 0, 2};
  zero_array(test_array, 3);
  assert(0 == test_array[0]);
  assert(0 == test_array[1]);
  assert(0 == test_array[2]);
}

void test_vacuum() {
  {
    int arr_to_remove[5] = {0, 1, 0, 1, 0};
    struct point arr_data[5] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}};
    int arr_id[5] = {0, 1, 2, 3, 4};

    int new_size = vacuum(arr_to_remove, arr_data, arr_id, 5);

    assert(3 == new_size);

    assert(0 == arr_id[0]);
    assert(2 == arr_id[1]);
    assert(4 == arr_id[2]);

    const struct point expected_point_0 = {0, 1};
    assert(point_equals(expected_point_0, arr_data[0]));
    const struct point expected_point_1 = {4, 5};
    assert(point_equals(expected_point_1, arr_data[1]));
    const struct point expected_point_2 = {8, 9};
    assert(point_equals(expected_point_2, arr_data[2]));
  }

  {
    int arr_to_remove[2] = {1, 1};
    struct point arr_data[5] = {{0, 1}, {2, 3}};
    int arr_id[5] = {0, 1};

    int new_size = vacuum(arr_to_remove, arr_data, arr_id, 2);

    assert(0 == new_size);
  }
}

void test_simulate_turn() {
  /*
            1111111111222
  01234567890123456789012
 0..................Zz.H.
 1.......................
 2.......................
 3.......................
 4.......................
 5.....A.................
 6.......................
 7.....a.................
 8.......................
 9.H..zZ.................
10.......................
*/

  struct game_state simulated_state = {{5, 5},
                                       2,
                                       {0, 1},
                                       {{21, 0}, {0, 9}},
                                       2,
                                       {0, 1},
                                       {{18, 0}, {5, 9}},
                                       {{19, 0}, {4, 9}}};

  // test step 1
  zombies_move_towards_their_targets(&simulated_state);
  assert(
      point_equals(simulated_state.zombie[0], simulated_state.zombie_next[0]));
  assert(
      point_equals(simulated_state.zombie[1], simulated_state.zombie_next[1]));

  // test step 2
  const struct point ash_move = {5, 7};
  ash_moves_towards_his_target(&simulated_state, ash_move);
  assert(point_equals(ash_move, simulated_state.ash));

  // test step 3
  assert(2 == simulated_state.zombie_count);
  int killed_zombie_index[simulated_state.zombie_count];
  zero_array(killed_zombie_index, (size_t)simulated_state.zombie_count);
  any_zombie_in_range_is_destroyed(&simulated_state, 9, killed_zombie_index);
  assert(0 == killed_zombie_index[0]);
  assert(1 == killed_zombie_index[1]);
  dump_game_state(&simulated_state);
}

void tests() {
  test_get_fibo();
  test_dist2();
  test_point_equals();
  test_index_by_value();
  test_sum_ones();
  test_find_nearest();
  test_elapsed();
  test_has_time();
  test_move_from_destination();
  test_generate_a_random_strategy();
  test_zero_array();
  test_vacuum();
  test_simulate_turn();
  printf("All tests SUCCESSFUL!\n");
}

int main() {

#ifndef MY_TEST
  game_loop();
#else
  tests();
#endif

  return 0;
}
