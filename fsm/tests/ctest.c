#include "../cfsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum { STATE_A, STATE_B } states_fsm1_t;
typedef enum { STATE_X, STATE_Y } states_fsm2_t;


bool state_A_called, state_B_called, state_X_called, state_Y_called;


/* every state has an implementation function */
void state_func_A(void *ctx)
{
        state_A_called = true;
        (void)ctx;
}


void state_func_B(void *ctx)
{
        state_B_called = true;
        (void)ctx;
}


void state_func_X(void *ctx)
{
        state_X_called = true;
        (void)ctx;
}


void state_func_Y(void *ctx)
{
        state_Y_called = true;
        (void)ctx;
}


/* define state transition functions */
typedef struct {
        int i;
} user_data_t;


bool state_trans_AB(void *ctx)
{
        (void)ctx;
        return true;
}


bool state_trans_XY(void *ctx)
{
        (void)ctx;
        return false;
}

h_cfsm_t fsm1;
h_cfsm_t fsm2;

/* now define fsm with states and transitions */
const h_cfsm_state_t states1[] = {
    [STATE_A] = {.func = state_func_A,
                 .transitions =
                     CFSM_TRANS{{.dest       = STATE_B,
                                 .transit    = state_trans_AB,
                                 .or_mask    = CFSM_OR_DEFAULT,
                                 .and_mask   = CFSM_AND_DEFAULT,
                                 .dep_list   = CFSM_DEPS{{.dep_fsm   = &fsm2,
                                                          .dep_state = STATE_Y,
                                                          .dep_type = CFSMDEP_EQ},
                                                         {0}},
                                 .transit_cb = NULL},
                                {0}}},
    [STATE_B] = {.func        = state_func_B,
                 .transitions = CFSM_TRANS{CFSM_SEQUENTIAL_TRANS, {0}}},
    {0}};

const h_cfsm_state_t states2[] = {
    [STATE_X] = {.func        = state_func_X,
                 .transitions = CFSM_TRANS{CFSM_SEQUENTIAL_TRANS, {0}}},
    [STATE_Y] = {.func        = state_func_Y,
                 .transitions = CFSM_TRANS{CFSM_SEQUENTIAL_TRANS, {0}}},
    {0}};

/* define user specific context */
user_data_t user_data;


cfsm_error_t error_ctx1, error_ctx2;


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


void reset_calls(void)
{
        state_A_called = state_B_called = state_X_called = state_Y_called =
            false;
}


int main(void)
{

        h_cfsm_init(&fsm1, states1, NULL, &error_ctx1, STATE_B);
        h_cfsm_init(&fsm2, states2, NULL, &error_ctx2, STATE_Y);

        reset_calls();
        CHECK(state_A_called == false, "A was not called...");
        CHECK(state_B_called == false, "B was not called...");
        CHECK(state_X_called == false, "X was not called...");
        CHECK(state_Y_called == false, "Y was not called...");

        /* fsm1 cannot change to B, since fsm2 is in state X */
        h_cfsm_step(&fsm1);
        CHECK(fsm1.current_state == STATE_A, "fsm1 didn't change ...");
        CHECK(state_A_called == true, "A was called...");

        reset_calls();
        /* the same... */
        h_cfsm_step(&fsm1);
        CHECK(fsm1.current_state == STATE_A, "fsm1 didn't change ...");
        CHECK(state_A_called == true, "A was called...");


        reset_calls();
        /* here fsm2 executes state func for X  and changes to Y */
        h_cfsm_step(&fsm2);
        CHECK(fsm2.current_state == STATE_Y, "fsm2 changed to Y ...");
        CHECK(state_X_called == true, "X was called...");


        reset_calls();
        /* here fsm1 still is in state A and now can switch to B */
        h_cfsm_step(&fsm1);
        CHECK(fsm1.current_state == STATE_B, "fsm1 now can switch to B...");
        CHECK(state_A_called == true, "A was called (seq. latency)...");

        reset_calls();
        /* now, fsm1 is in state B and calls state func B */
        h_cfsm_step(&fsm1);
        CHECK(fsm1.current_state == (uint8_t)STATE_B + 1,
              "fsm1 is in B + 1...");
        CHECK(state_B_called == true, "B was called...");

        reset_calls();
        /* fsm2 executes state func for Y and change to Y++, which does not
         * exist */
        h_cfsm_step(&fsm2);
        CHECK(fsm2.current_state == (uint8_t)STATE_Y + 1,
              "fsm2 is in Y + 1...");
        CHECK(state_Y_called == true, "Y was called (seq. latency)...");

        reset_calls();
        /* here, no state is called, because it is invalid */
        h_cfsm_step(&fsm2);
        h_cfsm_step(&fsm1);
        CHECK(state_A_called == false && state_B_called == false &&
                  state_X_called == false && state_Y_called == false,
              "No state was called...");

        return EXIT_SUCCESS;
}
