#ifndef X_MUTEX_WIN32_H
#define X_MUTEX_WIN32_H


int x_mutex_create_win32(x_mutex_t *m);
int x_mutex_destroy_win32(x_mutex_t *m);
int x_mutex_trylock_win32(x_mutex_t *m);
int x_mutex_lock_win32(x_mutex_t *m);
int x_mutex_unlock_win32(x_mutex_t *m);


#endif
