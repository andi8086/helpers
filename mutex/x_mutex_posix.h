#ifndef X_MUTEX_POSIX_H
#define X_MUTEX_POSIX_H


int x_mutex_create_posix(x_mutex_t *m);
int x_mutex_destroy_posix(x_mutex_t *m);
int x_mutex_trylock_posix(x_mutex_t *m);
int x_mutex_lock_posix(x_mutex_t *m);
int x_mutex_unlock_posix(x_mutex_t *m);


#endif
