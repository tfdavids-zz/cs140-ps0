#include "pintos_thread.h"

struct station {
    size_t num_waiting; // number of passengers waiting
    size_t num_seats; // number of seats available

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

    lock_init(&station->lock);
    cond_init(&station->space_available);
    cond_init(&station->train_may_leave);
}

void
station_load_train(struct station *station, int count)
{
    // grab the lock
    lock_acquire(&station->lock);

    // tell the station we have seats available, then tell the passengers
    station->num_seats += count;
    cond_broadcast(&station->space_available, &station->lock);

    // now wait for them to board
    while (station->num_seats > 0 && station->num_waiting > 0)
        cond_wait(&station->train_may_leave, &station->lock);

    // now the train will leave, so no more room
    station->num_seats = 0;

    // now give back the lock
    lock_release(&station->lock);
}

void
station_wait_for_train(struct station *station)
{
    // grab the lock
    lock_acquire(&station->lock);

    // we're waiting at the station
    station->num_waiting++;

    // while there's no room, keep waiting and let them have the lock back
    while (station->num_seats == 0)
        cond_wait(&station->space_available, &station->lock);

    // now there's space; get on the train!
    station->num_waiting--;
    station->num_seats--;
    // station_on_board(station);

    // let the train know it might be allowed to leave
    cond_signal(&station->train_may_leave, &station->lock);

    // now give back the lock
    lock_release(&station->lock);
}

void
station_on_board(struct station *station)
{
    return;
}
