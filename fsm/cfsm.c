#include "cfsm.h"

#include <string.h>
#include "../threads/x-atomic.h"


static bool check_dep(h_cfsm_t *fsm, uint8_t state, h_cfsm_dep_type_t dt)
{
        switch (dt) {
        case CFSMDEP_EQ: return h_cfsm_get_state(fsm) == state;
        case CFSMDEP_NE: return h_cfsm_get_state(fsm) != state;
        case CFSMDEP_LT: return h_cfsm_get_state(fsm) < state;
        case CFSMDEP_LE: return h_cfsm_get_state(fsm) <= state;
        case CFSMDEP_GT: return h_cfsm_get_state(fsm) > state;
        case CFSMDEP_GE: return h_cfsm_get_state(fsm) >= state;
        default: return false;
        }
        return false;
}


uint8_t h_cfsm_get_state(h_cfsm_t *cfsm)
{
        uint8_t res;

        xmutex_lock(&cfsm->lock);
        res = (uint8_t)x_atomic_load64(&cfsm->current_state);
        xmutex_unlock(&cfsm->lock);

        return res;
}


bool h_cfsm_set_state(h_cfsm_t *cfsm, uint8_t next_state)
{
        xmutex_lock(&cfsm->lock);

        /* check if current_state is ok */
        if (cfsm->current_state > cfsm->max_state) {
                xmutex_unlock(&cfsm->lock);
                return false;
        }

        /* look if there is a corresponding state transition */
        h_cfsm_state_t *s = &cfsm->states[cfsm->current_state];

        h_cfsm_state_trans_t *t = s->transitions;

        if (!t) {
                xmutex_unlock(&cfsm->lock);
                return false;
        }

        /* find transition to next_state */
        while (t->transit) {
                if (t->dest == CFSM_STATE_NEXT &&
                    next_state == cfsm->current_state + 1) {
                        /* we have a sequential transition, that matches, these
                         * never have deps */
                        cfsm->current_state = next_state;
                        xmutex_unlock(&cfsm->lock);
                        return true;
                }
                if (t->dest == next_state) {
                        break;
                }
                t++;
        }

        if (t->dest != next_state) {
                /* there is no such transition, cannot change */
                xmutex_unlock(&cfsm->lock);
                return false;
        }
        h_cfsm_state_dep_t *dep = t->dep_list;
        while (dep && dep->dep_fsm) {
                if (!check_dep(dep->dep_fsm, dep->dep_state, dep->dep_type)) {
                        xmutex_unlock(&cfsm->lock);
                        return false;
                }
                dep++;
        }

        if (t->transit_cb) {
                if (!t->transit_cb(cfsm->state_context, cfsm->error_context)) {
                        xmutex_unlock(&cfsm->lock);
                        return false;
                }
        }
        /* everything is well-defined, set next state */
        cfsm->current_state = next_state;

        xmutex_unlock(&cfsm->lock);
        return true;
}


static bool transit_checker(h_cfsm_state_trans_t *trans, h_cfsm_t *cfsm)
{
        bool res;

        res = trans->transit(cfsm->state_context);
        if (!res) {
                return res;
        }

        if (!trans->transit_cb) {
                /* no transit callback exists, return result of transit function
                 */
                return res;
        }

        /* transit function would indicate a transit,
         * now check the user callback */
        return trans->transit_cb(cfsm->state_context, cfsm->error_context);
}


static void h_cfsm_default_ctrl(h_cfsm_t *cfsm)
{
        xmutex_lock(&cfsm->lock);

        uint8_t state = (uint8_t)cfsm->current_state;

        /* This state machine is sequential, so if max_state is reached,
         * it may be incremented further, below, but this check here in the
         * next step, will then bail out. This also means, that the last
         * step is non-recoverable by itself */
        if (cfsm->current_state > cfsm->max_state) {
                xmutex_unlock(&cfsm->lock);
                return;
        }

        /* call function assigned to current state */
        if ((cfsm->state_mask >> state) & 1) {
                cfsm->states[state].func(cfsm->state_context);
        }

        /* check all possible transitions for current state */
        h_cfsm_state_trans_t *trans = cfsm->states[state].transitions;

        /* no implicit sequence */
        if (!trans) {
                xmutex_unlock(&cfsm->lock);
                return;
        }

        while (trans && trans->transit) {
                /* check dependencies of transit first, so only call transit
                 * function if dependency is met first */
                h_cfsm_state_dep_t *dep = trans->dep_list;
                while (dep && dep->dep_fsm) {
                        if (!check_dep(dep->dep_fsm, dep->dep_state,
                                       dep->dep_type)) {
                                /* this transition is not possible, check next
                                 * one */
                                goto check_next_trans;
                        }
                        dep++;
                }

                /* here, all previous dependency checks, if any, were true */
                if (transit_checker(trans, cfsm)) {
                        if (trans->dest == CFSM_STATE_NEXT) {
                                cfsm->current_state++;
                                break;
                        }
                        cfsm->current_state = trans->dest;
                        cfsm->state_mask &= trans->and_mask;
                        cfsm->state_mask |= trans->or_mask;
                        break;
                }
        check_next_trans:
                trans++;
        }
        xmutex_unlock(&cfsm->lock);
}


void h_cfsm_init(h_cfsm_t *cfsm, const h_cfsm_state_t *states, void *context,
                 cfsm_error_t *error_ctx, uint8_t max_state)
{
        memset(cfsm, 0, sizeof(h_cfsm_t));

        cfsm->states = (h_cfsm_state_t *)states;

        /* enable all states per default, and start with initial state */
        cfsm->state_mask    = (h_cfsm_state_mask_t)-1;
        cfsm->current_state = 0;
        cfsm->ctrl          = h_cfsm_default_ctrl;
        cfsm->state_context = context;
        cfsm->max_state     = max_state;
        cfsm->error_context = error_ctx;

        xmutex_init(&cfsm->lock);
}


void h_cfsm_step(h_cfsm_t *cfsm)
{
        cfsm->ctrl(cfsm);
}


bool h_cfsm_sequential_trans(void *ctx)
{
        (void)ctx;
        return true;
}
