#include <stdio.h>
#include <time.h>
#include "topsig-config.h"
#include "topsig-global.h"
#include "topsig-atomic.h"
#include "topsig-progress.h"
#include "topsig-semaphore.h"

static struct {
  enum {PROGRESS_NONE, PROGRESS_PERIODIC, PROGRESS_FULL} type;
  int period;
  int totaldocs;
} cfg;

TSemaphore sem_progress;

void Progress_InitCfg()
{
  cfg.type = PROGRESS_NONE;
  cfg.period = 1000;
  cfg.totaldocs = 0;
  
  if (lc_strcmp(Config("OUTPUT-PROGRESS"),"none")==0) cfg.type = PROGRESS_NONE;
  if (lc_strcmp(Config("OUTPUT-PROGRESS"),"periodic")==0) cfg.type = PROGRESS_PERIODIC;
  if (lc_strcmp(Config("OUTPUT-PROGRESS"),"full")==0) cfg.type = PROGRESS_FULL;
  
  if (Config("OUTPUT-PERIOD")) cfg.period = atoi(Config("OUTPUT-PERIOD"));
  if (Config("OUTPUT-PROGRESS-DOCUMENTS")) cfg.totaldocs = atoi(Config("OUTPUT-PROGRESS-DOCUMENTS"));
  
  tsem_init(&sem_progress, 0, 1);
  
  if (cfg.totaldocs > 0) printf("\n");
}

static int current_docs = 0;

void ProgressTick(const char *identifier)
{
  int c = atomic_add(&current_docs, 1) + 1;
  
  if (cfg.type == PROGRESS_NONE) return;
  if ((cfg.type == PROGRESS_PERIODIC) && (c % cfg.period != 0)) return;
  tsem_wait(&sem_progress);
  
  char time_est[256] = "";
  
  clock_t clockval = clock();
  int time_secs = clockval / CLOCKS_PER_SEC;
  int time_ms = clockval % CLOCKS_PER_SEC / (CLOCKS_PER_SEC / 1000);
  int time_mins = time_secs / 60;
  int dps = 0;
  dps = (double) c / (double) clockval * CLOCKS_PER_SEC;
  time_secs = time_secs % 60;
  
  sprintf(time_est, "- %d:%02d.%03d (%d per sec)", time_mins, time_secs, time_ms, dps);
  
  if (cfg.totaldocs <= 0) {
  
    if (cfg.type == PROGRESS_FULL) printf("[%s]  ", identifier);
    printf("%d %s\n", c, time_est);
    
  } else {
    char meter[11] = "          ";
    int meter_prg = (c * 10 + (cfg.totaldocs / 2)) / cfg.totaldocs;
    if (meter_prg > 10) meter_prg = 10;
    for (int i = 0; i < meter_prg; i++) {
      meter[i] = '*';
    }
    printf("\r[%s] %d/%d %s", meter, c, cfg.totaldocs, time_est);
  }
  
  tsem_post(&sem_progress);
}
