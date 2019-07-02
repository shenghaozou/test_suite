#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define LAYERS 2
#define FILES_EACH_LAYER 1000
#define MIN_SIZE 1000
#define MAX_SIZE 2000
#define REPEAT 100

char path[128];

int test_layer(int layer) {
  struct stat st;
  clock_t before = clock();
  for (int j = 0; j < FILES_EACH_LAYER; j++) {
     sprintf(path, "./%d-%d.txt", layer, j);
     int rc = stat(path, &st);
     assert(rc == 0);
  }
  clock_t difference = clock() - before;
  return difference * 1000 / CLOCKS_PER_SEC;
}

void print_layers_time_cost(int msec[], int sum) {
  int i;
  for (int i = 0; i < LAYERS; i++) {
    printf("Layer %d: %d ms\n", i, msec[i]);
  }
  printf("\nTotal: %d ms (%d stat tests).\n", sum, LAYERS * FILES_EACH_LAYER);
}

int test_layers(int msec[]) {
  int sum = 0;
  for (int i = 0; i < LAYERS; i++) {
    msec[i] = test_layer(i);
    sum += msec[i];
  }
  //print_layers_time_cost(msec, sum);
  return sum;
}

int test_layers_rev(int msec[]) {
  int sum = 0;
  for (int i = LAYERS - 1; i >= 0; i--) {
    msec[i] = test_layer(i);
    sum += msec[i];
  }
  //print_layers_time_cost(msec, sum);
  return sum;
}

int main(int argc, char *argv[]) {
    int i, j;
    struct stat st;
    int msec[LAYERS];
    int sum = 0;

    srand(time(0));

    if (strcmp(argv[1], "new") == 0) {
        printf("creating new test files.\n");

        for (i = 0; i < LAYERS; i++) {
            sprintf(path, "./layer-%d", i);
            if (stat(path, &st) == -1) {
                mkdir(path, 0777);
            }
        }

        char random_data[MAX_SIZE + 1];
        for (i = 0; i < LAYERS; i++) {
            for (j = 0; j < FILES_EACH_LAYER; j++) {
                sprintf(path, "./layer-%d/%d-%d.txt", i, i, j);
                printf("file: %s", path);
                FILE *fp;
                fp = fopen(path, "w");
                if (fp == NULL) {
                    printf("ERROR. CANNOT OPEN THE FILE.\n");
                }
                else {
                    int sz = rand() % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE;
                    int k = 0;
                    while (k < sz) {
                        random_data[k++] = rand() % 127;
                    }
                    random_data[sz] = 0;
                    fprintf(fp, "%s", random_data);
                }
            }
        }
    }
    else if (strcmp(argv[1], "test") == 0) {
      sum = test_layers(msec);
      print_layers_time_cost(msec, sum);
    }
    else if (strcmp(argv[1], "test_rev") == 0) {
      sum = test_layers_rev(msec);
      print_layers_time_cost(msec, sum);
    }
    else if (strcmp(argv[1], "test_repeat") == 0) {
      int total_msec[LAYERS];
      int total_sum = 0;
      memset(total_msec, 0, sizeof(total_msec));

      for (i = 0; i < REPEAT; i++) {
        test_layers(msec);
        for (j = 0; j < LAYERS; j++) {
          total_msec[j] += msec[j];
        }
      }

      for (j = 0; j < LAYERS; j++) {
        total_sum += total_msec[j];
      }
      print_layers_time_cost(total_msec, total_sum);
    }
    else if (strcmp(argv[1], "dockerfile") == 0) {
      printf("ADD test /root\n");
      for (i = LAYERS - 1; i >= 0; i--) {
        printf("ADD layer-%d /root\n", i);
      }
    }
}
