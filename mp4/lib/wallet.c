#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement `wallet_init`
  wallet->lock = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(wallet->lock, NULL);
  wallet->cond = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(wallet->cond, NULL);
  memset(wallet->resources, 0, sizeof(wallet->resources));
  memset(wallet->counts, 0, sizeof(wallet->counts));
}

/**
 * Returns the amount of a given `resource` in the given `wallet`.
 */
int wallet_get(wallet_t *wallet, const char *resource) {
  // Implement `wallet_get`  
  for (int i = 0; i < 1000; i++) {
    if (wallet->resources[i] == NULL) {
      return 0;
    }
    if (strcmp(wallet->resources[i], resource) == 0) {
      return wallet->counts[i];
    }
  }
  return 0;
}

/**
 * Modifies the amount of a given `resource` in a given `wallet by `delta`.
 * - If `delta` is negative, this function MUST NOT RETURN until the resource can be satisfied.
 *   (Ths function MUST BLOCK until the wallet has enough resources to satisfy the request.)
 * - Returns the amount of resources in the wallet AFTER the change has been applied.
 */
int wallet_change_resource(wallet_t *wallet, const char *resource, const int delta) {
  // Implement `wallet_change_resource`
  int found = -1;
  int idx = 0;
  for (int i = 0; i < 1000; i++) {
    if (wallet->resources[i] == NULL) { // reaches last resource without finding resource
      idx = i;
      break;
    }
    if (strcmp(wallet->resources[i], resource) == 0) { // already have resource
      found = i;
      break;
    }
  }
  if (found == -1) { // adds new resources
    wallet->resources[idx] = strdup(resource);
    wallet->counts[idx] = 0;
    found = idx;
  }
  pthread_mutex_lock(wallet->lock);
  // hold if delta would make us go in debt
  while (wallet->counts[found] + delta < 0) {
    pthread_cond_wait(wallet->cond, wallet->lock);
  }
  wallet->counts[found] += delta;
  
  pthread_cond_broadcast(wallet->cond);
  pthread_mutex_unlock(wallet->lock);
  return wallet->counts[found];
}

/**
 * Destroys a wallet, freeing all associated memory.
 */
void wallet_destroy(wallet_t *wallet) {
  // Implement `wallet_destroy`
  pthread_mutex_destroy(wallet->lock);
  pthread_cond_destroy(wallet->cond);
  free(wallet->cond);
  free(wallet->lock);
  for (int i = 0; i < 1000; i++) {
    if (wallet->resources[i] == NULL) {
      break;
    }
    free(wallet->resources[i]);
  }
}