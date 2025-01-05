/*
Inspired by
https://www.codingame.com/forum/t/code-vs-zombies-feedback-strategies/1089/37
*/
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum constraints {
  max_human = 99,
  max_zombie = 99,
  response_time_ms = 90,
  max_x_exclusive = 16000,
  max_y_exclusive = 9000,
  max_ash_move = 1000
};

struct point {
  int x, y;
};

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

int zombie_index_by_id(const struct game_state *state, int zombie_id) {
  for (int i = 0; i < state->zombie_count; ++i) {
    if (state->zombie_id[i] == zombie_id) {
      return i;
    }
  }
  return -1;
}

struct strategy {
  int random_moves_count;
  int target_zombie_id;
  struct point first_move;
};

long scoring(const struct game_state *game_state) {
  long result = 0;

  return result;
}

double elapsed(clock_t start_t, clock_t end_t) {
  fprintf(stderr, "clock %ld %ld %ld\n", start_t, end_t, CLOCKS_PER_SEC);
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

void simulate_turn(struct game_state *simulated_state) {
  // todo
}

long simulate_the_strategy(const struct game_state *initial_state,
                           struct strategy *result) {
  struct game_state simulated_state = *initial_state;
  for (int i = 0; i < result->random_moves_count; ++i) {
    const struct point random_dest = {rand() % max_x_exclusive,
                                      rand() % max_y_exclusive};
    const struct point actual_dest =
        move_from_destination(simulated_state.ash, random_dest);
    if (i == 0) {
      result->first_move = actual_dest;
    }
    simulate_turn(&simulated_state);
  }

  int target_zombie_index =
      zombie_index_by_id(&simulated_state, result->target_zombie_id);

  // todo
  return 0;
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
           const struct strategy initial_strategy, const clock_t start_t) {
  struct strategy current_strategy = initial_strategy;
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

int main() {
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

    move(&game_state);
  }

  return 0;
}
