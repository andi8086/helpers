#ifndef X_MUTEX_TIRTOS_H
#define X_MUTEX_TIRTOS_H


int x_mutex_create_tirtos(x_mutex_t *m);
int x_mutex_destroy_tirtos(x_mutex_t *m);
int x_mutex_trylock_tirtos(x_mutex_t *m);
int x_mutex_lock_tirtos(x_mutex_t *m);
int x_mutex_unlock_tirtos(x_mutex_t *m);


#endif
