#include "../../command.h"
#include "clock.h"
#include "spinlocks.h"
#include "task.h"
#include "vmap.h"
#include <asm/type.h>
#include <device.h>
#include <print.h>
#include <string.h>

#define CLOCK_FREQ 10000000
#define US_PERCYCLE (1000000 / CLOCK_FREQ)

#define MAX_CPU 32

static int task_args[4];
static int condition = 0;
static int nr_cpu = 2;
static int nr_repeat = 100000;
static int PING = 0;
static int PONG = 1;
static int duration_cpus[MAX_CPU];

static int sem = 0;

void sem_post() { __atomic_fetch_add(&sem, 1, __ATOMIC_SEQ_CST); }

void sem_wait() {
  while (__atomic_load_n(&sem, __ATOMIC_SEQ_CST) == 0)
    ;
  __atomic_fetch_sub(&sem, 1, __ATOMIC_SEQ_CST);
}

static int do_core2core_cas(void *data) {
  int *ptr = (int *)data;
  int thread_id = ptr[0];
  int flag = ptr[1];

  int i, ret, expected, disired;
  int start, end;
  if (flag) {
    expected = PING;
    disired = PONG;
  } else {
    expected = PONG;
    disired = PING;
  }

  int expected_origin = expected;

  start = get_system_tick();

  for (i = 0; i < nr_repeat; i++) {
  repeat:
    ret = __atomic_compare_exchange_n(&condition, &expected, disired, 0,
                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    if (ret == 0) {
      expected = expected_origin;
      goto repeat;
    }
  }

  end = get_system_tick();
  duration_cpus[thread_id] = (end - start);
  sem_post();
  return 0;
}

static int cmd_c2c_cas_lat_handler(int argc, char *argv[], void *priv) {
  print(" Core To Core CAS Latency \n");

  char *names[] = {"cpu0", "cpu1", "cpu2", "cpu3",
                   "cpu4", "cpu5", "cpu6", "cpu7"};

  int i, j, k;

  if (argc > 0) {
    nr_cpu = atoi(argv[0]);
  }
  if (argc > 1) {
    nr_repeat = atoi(argv[1]);
  }

  print("\n");

  task_args[0] = 0;
  task_args[1] = 0;
  task_args[2] = 0;
  task_args[3] = 1;

  print("         ");
  for(i=0; i < nr_cpu; i++) {
    print("%d%s", i, "                   ");
  }
  print("\n");

  for (i = 0; i < nr_cpu; i++) {
    print("%d ", i);
    for (j = 0; j < nr_cpu; j++) {
      if (i <= j) {
        continue;
      }
      task_args[0] = i;
      task_args[2] = j;
      
      int duration[10];
      int dura_diff = 0;
      int dura_ava;
      int dura_tot = 0;
      for (k = 0; k < 16; k++) {
        create_task(names[i], do_core2core_cas, &task_args[0], i, NULL, 0, NULL);
        create_task(names[j], do_core2core_cas, &task_args[2], j, NULL, 0, NULL);
        
        sem_wait();
        sem_wait();

        duration[k] = (duration_cpus[i] + duration_cpus[j]) / 2;
        dura_tot += duration[k];
      }

      dura_ava = dura_tot / 10;

      for(k=0;k<10;k++) {
        int diff = duration[k] > dura_ava ? duration[k] - dura_ava : dura_ava - duration[k];
        dura_diff = dura_diff > diff ? dura_diff : diff;
      }
      
      print("%7d(+-%7d) - ", dura_ava, dura_diff);
    }
    print("\n");
  }

  return 0;
}

static const struct command cmd_c2c_cas_lat = {
    .cmd = "c2cas",
    .handler = cmd_c2c_cas_lat_handler,
    .priv = NULL,
};

int cmd_c2c_cas_lat_init() {
  register_command(&cmd_c2c_cas_lat);

  return 0;
}

APP_COMMAND_REGISTER(c2c_cas_lat, cmd_c2c_cas_lat_init);
