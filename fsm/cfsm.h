#ifndef HELPERS_CFSM_H
#define HELPERS_CFSM_H

#include <stdint.h>
#include <stdbool.h>

#include "../mutex/xmutex.h"


/* the h_cfsm_state_mask_t data type must accommodate the maximal amount of
 * states, i.e. number of bits == number of states possible */
#define H_CFSM_MAX_STATES 64

typedef uint64_t h_cfsm_state_mask_t;

/* every state is a function that manipulates a user data structure given by a
 * pointer */

typedef void (*h_cfsm_state_func_t)(void *);
typedef bool (*h_cfsm_state_transit_func_t)(void *);


typedef enum {
        CFSMDEP_EQ, /* equal */
        CFSMDEP_NE, /* not equal */
        CFSMDEP_LT, /* less than */
        CFSMDEP_LE, /* less or equal */
        CFSMDEP_GT, /* greater then */
        CFSMDEP_GE  /* greater or equal */
} h_cfsm_dep_type_t;


typedef struct h_cfsm h_cfsm_t;


/* coupled state machines introduce dependencies for state transitions, sentinel
 * is dep_fsm == NULL */
typedef struct {
        h_cfsm_t *dep_fsm;
        uint8_t dep_state;
        h_cfsm_dep_type_t dep_type;
} h_cfsm_state_dep_t;


typedef struct {
        uint64_t user_error;
        void *user_data;
} cfsm_error_t;

/* This is the type of a callback function which is executed upon state
 * transition. After dependencies of the transition have been checked true, this
 * callback is called. If it returns false, the state transition is not
 * executed, and user specific error information can be stored in the
 * cfsm_error_t struct */
typedef bool (*h_cfsm_transit_cb_t)(void *, cfsm_error_t *e);


/* each state transition is defined for a specific starting state and represents
 * an array of possible end states, ordered by likelyhood */
typedef struct h_cfsm_state_trans {
        uint8_t dest;
        h_cfsm_state_transit_func_t transit;
        h_cfsm_state_mask_t or_mask;
        h_cfsm_state_mask_t and_mask;
        h_cfsm_state_dep_t *dep_list;
        h_cfsm_transit_cb_t transit_cb;
} h_cfsm_state_trans_t;


typedef struct h_cfsm_state {
        h_cfsm_state_func_t func;
        h_cfsm_state_trans_t *transitions;
} h_cfsm_state_t;


#define CFSM_STATES (h_cfsm_state_t[])
#define CFSM_TRANS  (h_cfsm_state_trans_t[])
#define CFSM_DEPS   (h_cfsm_state_dep_t[])
#define CFSM_MASK   (h_cfsm_state_mask_t)

#define CFSM_AND_DEFAULT -1
#define CFSM_OR_DEFAULT  0

#define CFSM_ENABLE(x)  (CFSM_MASK(1 << x))
#define CFSM_DISABLE(x) (~CFSM_MASK(1 << x))

/* the cfsm operates on a function pointer array of states, together with a
 * controller function.
 * For the controller function there must be another array per state which
 * defines all possible state transistions */


typedef void (*h_cfsm_state_ctrl_t)(h_cfsm_t *cfsm);


struct h_cfsm {
        xmutex_t lock;
        h_cfsm_state_mask_t state_mask;
        h_cfsm_state_t *states;
        h_cfsm_state_ctrl_t ctrl;
        uint64_t current_state;
        uint64_t max_state;
        void *state_context;
        cfsm_error_t *error_context;
};


void h_cfsm_init(h_cfsm_t *cfsm, const h_cfsm_state_t *states, void *context,
                 cfsm_error_t *error_ctx, uint8_t max_state);
void h_cfsm_step(h_cfsm_t *cfsm);

bool h_cfsm_sequential_trans(void *ctx);

/* checks dependencies before switching to new state and returns false if deps
 * are not met or state does not exist */
bool h_cfsm_set_state(h_cfsm_t *cfsm, uint8_t next_state);

/* returns current state of the CFSM */
uint8_t h_cfsm_get_state(h_cfsm_t *cfsm);


#define CFSM_STATE_NEXT (H_CFSM_MAX_STATES - 1)

#define CFSM_SEQUENTIAL_TRANS                                                \
        {                                                                    \
                .dest = CFSM_STATE_NEXT, .transit = h_cfsm_sequential_trans, \
                .and_mask = -1, .or_mask = 0, .dep_list = CFSM_DEPS{{0}},    \
                .transit_cb = NULL                                           \
        }

#endif
