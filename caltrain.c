#include "pintos_thread.h"

struct station {
    // we need to keep track of who is waiting, boarding, and in their seat,
    // and how many seats are available on the train
    size_t num_waiting, num_seats, num_boarding, num_boarded;

    struct lock lock;
    struct condition space_available;
    struct condition train_may_leave;
};

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
    // keep loading until we've filled the train, or until nobody is waiting
    // and everyone who is boarding is in his or her seat
    while (station->num_boarded < station->num_seats
           && (station->num_waiting > 0
               || station->num_boarded < station->num_boarding))
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
    // tell the station we're boarding, and not waiting anymore
    station->num_boarding++;
    station->num_waiting--;
    lock_release(&station->lock);
}

void
station_on_board(struct station *station)
{
    lock_acquire(&station->lock);
    // tell the station we've boarded, and let
    // the train know it might be able to leave
    station->num_boarded++;
    cond_signal(&station->train_may_leave, &station->lock);
    lock_release(&station->lock);
}
