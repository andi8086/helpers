#include "../fsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum { STATE_A, STATE_B, STATE_C, STATE_D } states_t;


/* every state has an implementation function */
void state_func_A(void *ctx)
{
        (void)ctx;
}


void state_func_B(void *ctx)
{
        (void)ctx;
}


void state_func_C(void *ctx)
{
        (void)ctx;
}


void state_func_D(void *ctx)
{
        (void)ctx;
}


/* define state transition functions */
typedef struct {
        int i;
} user_data_t;


bool state_trans_BD(void *ctx)
{
        int *i = &((user_data_t *)ctx)->i;
        (*i)++;
        printf("Context: %d\n", *i);
        if (*i > 1)
                return false;
        return true;
}


bool state_trans_DA(void *ctx)
{
        (void)ctx;
        return true;
}


/* now define fsm with states and transitions */
const h_fsm_state_t states[] = {
    [STATE_A] = {.func        = state_func_A,
                 .transitions = FSM_TRANS{FSM_SEQUENTIAL_TRANS, {0}}},
    [STATE_B] = {.func        = state_func_B,
                 .transitions = FSM_TRANS{{.dest     = STATE_D,
                                           .transit  = state_trans_BD,
                                           .and_mask = FSM_AND_DEFAULT,
                                           .or_mask  = FSM_OR_DEFAULT},
                                          FSM_SEQUENTIAL_TRANS,
                                          {0}}},
    [STATE_C] = {.func        = state_func_C,
                 .transitions = FSM_TRANS{FSM_SEQUENTIAL_TRANS, {0}}},
    [STATE_D] = {.func        = state_func_D,
                 .transitions = FSM_TRANS{{.dest     = STATE_A,
                                           .transit  = state_trans_DA,
                                           .and_mask = FSM_AND_DEFAULT,
                                           .or_mask  = FSM_OR_DEFAULT},
                                          {0}}}};

/* define user specific context */
user_data_t user_data;

#define CHECK(x, s)                         \
        {                                   \
                printf("%-44s", s);         \
                if (x)                      \
                        printf("ok\n");     \
                else {                      \
                        printf("failed\n"); \
                        return -1;          \
                }                           \
        }

int main(void)
{
        h_fsm_t fsm;
        user_data.i = 0;

        h_fsm_init(&fsm, states, (void *)&user_data, STATE_D);

        const states_t expected_sequence[] = {STATE_A, STATE_B, STATE_D,
                                              STATE_A, STATE_B, STATE_C,
                                              STATE_D, STATE_A};

        for (int i = 0; i < 8; i++) {
                printf("Current State: %d\n", fsm.current_state);
                CHECK(fsm.current_state == expected_sequence[i],
                      "FSM in correct state...");

                h_fsm_step(&fsm);
        }

        return EXIT_SUCCESS;
}
