#pragma once
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wallet_t_ {
  // Add anything here! :)
  pthread_mutex_t *lock;
  pthread_cond_t *cond;
  char *resources[1000];
  //char *resources[1000];
  int counts[1000];
} wallet_t;

void wallet_init(wallet_t *wallet);
int wallet_get(wallet_t *wallet, const char *resource);
int wallet_change_resource(wallet_t *wallet, const char *resource, const int delta);
void wallet_destroy(wallet_t *wallet);

#ifdef __cplusplus
}
#endif