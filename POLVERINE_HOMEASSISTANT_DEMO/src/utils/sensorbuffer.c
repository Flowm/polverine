#include "sensorbuffer.h"


void sb_init(sensorbuffer* sb,size_t sz)
{
    sb->head = 0;
    sb->full = false;
    if((sz > SENSORBUFFER_MAX_SIZE) || (sz == 0))
        sb->size = SENSORBUFFER_MAX_SIZE;
    else
        sb->size = sz;
}

void sb_add(sensorbuffer* sb, float value)
{
    sb->buffer[sb->head++] = value;
    if(sb->head >= sb->size)
    {
        sb->full = true;
        sb->head = 0;
    }
}

float sb_average(const sensorbuffer* sb)
{
    float sum = 0;
    if(sb->full)
    {
        for(size_t i = 0; i < sb->size; i++)
            sum += sb->buffer[i];
        return sum / (float) sb->size;
    }
    else
    {
        if(sb->head)
        {
        for(size_t i = 0; i < sb->head; i++)
            sum += sb->buffer[i];
        return sum / (float) sb->head;
        }
        else
        return 0.;
    }

}
