#ifndef HELPERS_FSM_H
#define HELPERS_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* the h_fsm_state_mask_t data type must accommodate
 * the maximal amount of states, i.e. number of bits ==
 * number of states possible */
#define H_FSM_MAX_STATES 64

typedef uint64_t h_fsm_state_mask_t;

/* every state is a function that manipulates a user
 * data structure given by a pointer */

typedef void (*h_fsm_state_func_t)(void *);
typedef bool (*h_fsm_state_transit_func_t)(void *);

/* each state transition is defined for a specific
 * starting state and represents an array of possible
 * end states, ordered by likelyhood */
typedef struct h_fsm_state_trans {
        uint8_t dest;
        h_fsm_state_transit_func_t transit;
        h_fsm_state_mask_t or_mask;
        h_fsm_state_mask_t and_mask;
} h_fsm_state_trans_t;


typedef struct h_fsm_state {
        h_fsm_state_func_t func;
        h_fsm_state_trans_t *transitions;
} h_fsm_state_t;


#define FSM_STATES (h_fsm_state_t[])
#define FSM_TRANS  (h_fsm_state_trans_t[])
#define FSM_MASK   (h_fsm_state_mask_t)

#define FSM_AND_DEFAULT -1
#define FSM_OR_DEFAULT  0

#define FSM_ENABLE(x)  (FSM_MASK(1 << x))
#define FSM_DISABLE(x) (~FSM_MASK(1 << x))

/* the fsm operates on a function pointer array of states, together with a
 * controller function.
 * For the controller function there must be another array per state which
 * defines all possible state transistions */

typedef struct h_fsm h_fsm_t;

typedef void (*h_fsm_state_ctrl_t)(h_fsm_t *fsm);


struct h_fsm {
        h_fsm_state_mask_t state_mask;
        h_fsm_state_t *states;
        h_fsm_state_ctrl_t ctrl;
        uint8_t current_state;
        uint8_t max_state;
        void *state_context;
};


void h_fsm_init(h_fsm_t *fsm, const h_fsm_state_t *states, void *context,
                uint8_t max_state);
void h_fsm_step(h_fsm_t *fsm);

bool h_fsm_sequential_trans(void *ctx);

#define FSM_STATE_NEXT (H_FSM_MAX_STATES - 1)

#define FSM_SEQUENTIAL_TRANS                                               \
        {                                                                  \
                .dest = FSM_STATE_NEXT, .transit = h_fsm_sequential_trans, \
                .and_mask = -1, .or_mask = 0                               \
        }

#endif
