#include "pintos_thread.h"

struct station {
    size_t num_waiting; // number of passengers waiting
    size_t num_seats; // number of seats available
    size_t num_boarding;
    size_t num_boarded;

    struct lock lock; // monitor lock
    struct condition space_available;
    struct condition train_may_leave;
};

void station_on_board(struct station *station);

void
station_init(struct station *station)
{
    station->num_waiting = 0;
    station->num_seats = 0;
    station->num_boarding = 0;
    station->num_boarded = 0;

    lock_init(&station->lock);
    cond_init(&station->space_available);
    cond_init(&station->train_may_leave);
}

void
station_load_train(struct station *station, int count)
{
    lock_acquire(&station->lock);
    station->num_boarding = 0;
    station->num_boarded = 0;
    station->num_seats = count;
    cond_broadcast(&station->space_available, &station->lock);
    while (station->num_boarded < station->num_seats && (station->num_waiting > 0 || station->num_boarded < station->num_boarding))
        cond_wait(&station->train_may_leave, &station->lock);
    station->num_seats = 0;
    lock_release(&station->lock);
}

void
station_wait_for_train(struct station *station)
{
    lock_acquire(&station->lock);
    station->num_waiting++;
    while (station->num_boarding == station->num_seats)
        cond_wait(&station->space_available, &station->lock);
    station->num_boarding++;
    station->num_waiting--;
    lock_release(&station->lock);
}

void
station_on_board(struct station *station)
{
    lock_acquire(&station->lock);
    station->num_boarded++;
    cond_signal(&station->train_may_leave, &station->lock);
    lock_release(&station->lock);
}
