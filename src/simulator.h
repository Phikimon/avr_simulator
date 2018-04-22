#ifndef SIMULATOR_H
#define SIMULATOR_H

struct simulator {
    uint64_t        tick;
    int             pipe[2];
    struct attiny13 chip;

}

#endif
