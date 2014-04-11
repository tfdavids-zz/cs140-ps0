#include "pintos_thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
    size_t h_excess, h_needed;

    struct lock lock;
    struct condition o_available;
    struct condition h_available;
};

void
reaction_init(struct reaction *reaction)
{
    reaction->h_excess = 0;
    reaction->h_needed = 0;

    lock_init(&reaction->lock);
    cond_init(&reaction->o_available);
    cond_init(&reaction->h_available);
}

void
reaction_h(struct reaction *reaction)
{
    lock_acquire(&reaction->lock);
    reaction->h_excess++;
    cond_signal(&reaction->h_available, &reaction->lock);
    while (reaction->h_needed == 0)
        cond_wait(&reaction->o_available, &reaction->lock);
    reaction->h_needed--;
    lock_release(&reaction->lock);
}

void
reaction_o(struct reaction *reaction)
{
    lock_acquire(&reaction->lock);
    // need to wait until we have two h's available;
    // otherwise, we might use one when there's not another to use.
    while (reaction->h_excess < 2)
        cond_wait(&reaction->h_available, &reaction->lock);
    // now we know we can take at least two h's
    reaction->h_needed += 2;
    reaction->h_excess -= 2;
    cond_broadcast(&reaction->o_available, &reaction->lock);
    make_water();
    lock_release(&reaction->lock);
}
