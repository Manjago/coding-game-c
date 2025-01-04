#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum constraints { max_human = 99, max_zombie = 99 };

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

double elapsed(clock_t start_t, clock_t end_t) {
   fprintf(stderr, "clock %ld %ld %ld\n", start_t, end_t, CLOCKS_PER_SEC); 
   return (double)(end_t - start_t) / CLOCKS_PER_SEC * 1000;
}

void dump_game_state(const struct game_state *src) {
  fprintf(stderr, "Ash (%d,%d)\n", src->ash.x, src->ash.x);
  fprintf(stderr, "H: ");
  for (int i = 0; i < src->human_count; ++i) {
    if (i != src->human_count - 1) {
      fprintf(stderr, "%d:(%d,%d),", src->human_id[i], src->human[i].x,
              src->human[i].y);
    } else {
      fprintf(stderr, "%d:(%d,%d)\n", src->human_id[i], src->human[i].x,
              src->human[i].y);
    }
    for (int i = 0; i < src->zombie_count; ++i) {
      if (i != src->zombie_count - 1) {
        fprintf(stderr, "%d:(%d,%d)->(%d,%d),", src->zombie_id[i],
                src->zombie[i].x, src->zombie[i].y, src->zombie_next[i].x,
                src->zombie_next[i].y);
      } else {
        fprintf(stderr, "%d:(%d,%d)->(%d,%d)\n", src->zombie_id[i],
                src->zombie[i].x, src->zombie[i].y, src->zombie_next[i].x,
                src->zombie_next[i].y);
      }
    }
  }
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

  struct game_state game_state;

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
