#include "fsm.h"

#include <string.h>


static void h_fsm_default_ctrl(h_fsm_t *fsm)
{
        uint8_t state = (uint8_t)fsm->current_state;
        /* This state machine is sequential, so if max_state is reached,
         * it may be incremented further, below, but this check here in the
         * next step, will then bail out. This also means, that the last
         * step is non-recoverable by itself */
        if (fsm->current_state > fsm->max_state) {
                return;
        }

        /* call function assigned to current state */
        if ((fsm->state_mask >> state) & 1) {
                fsm->states[state].func(fsm->state_context);
        }

        /* check all possible transitions for current state */
        h_fsm_state_trans_t *trans = fsm->states[state].transitions;

        /* no implicit sequence */
        if (!trans) {
                return;
        }

        while (trans && trans->transit) {
                if (trans->transit(fsm->state_context)) {
                        if (trans->dest == FSM_STATE_NEXT) {
                                fsm->current_state++;
                                break;
                        }
                        fsm->current_state = trans->dest;
                        fsm->state_mask &= trans->and_mask;
                        fsm->state_mask |= trans->or_mask;
                        break;
                }
                trans++;
        }
}


void h_fsm_init(h_fsm_t *fsm, const h_fsm_state_t *states, void *context,
                uint8_t max_state)
{
        memset(fsm, 0, sizeof(h_fsm_t));

        fsm->states = (h_fsm_state_t *)states;

        /* enable all states per default, and start with initial state */
        fsm->state_mask    = (h_fsm_state_mask_t)-1;
        fsm->current_state = 0;
        fsm->ctrl          = h_fsm_default_ctrl;
        fsm->state_context = context;
        fsm->max_state     = max_state;
}


void h_fsm_step(h_fsm_t *fsm)
{
        fsm->ctrl(fsm);
}


bool h_fsm_sequential_trans(void *ctx)
{
        (void)ctx;
        return true;
}
