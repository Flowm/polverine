#ifndef SENSORBUFFER_H_
#define SENSORBUFFER_H_

#include <stddef.h>
#include <stdbool.h>

#define SENSORBUFFER_MAX_SIZE 60

typedef struct 
{
    float buffer[SENSORBUFFER_MAX_SIZE];
    size_t size;
    size_t head;
    bool full;
} sensorbuffer;

void sb_init(sensorbuffer*sb,size_t sz);
void sb_add(sensorbuffer*sb, float value);
float sb_average(const sensorbuffer*sb);

#endif