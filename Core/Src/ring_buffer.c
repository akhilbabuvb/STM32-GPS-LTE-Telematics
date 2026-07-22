#include "ring_buffer.h"
void RingBuffer_Init(RingBuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}
bool RingBuffer_Write(RingBuffer_t *rb, uint8_t data)
{
    if (RingBuffer_IsFull(rb))
    {
        return false;
    }

    rb->buffer[rb->head] = data;

    rb->head++;

    if (rb->head >= RING_BUFFER_SIZE)
    {
        rb->head = 0;
    }

    return true;
}
bool RingBuffer_IsEmpty(RingBuffer_t *rb)
{
    return (rb->head == rb->tail);
}
bool RingBuffer_IsFull(RingBuffer_t *rb)
{
    uint16_t next_head = (rb->head + 1) % RING_BUFFER_SIZE;

    return (next_head == rb->tail);
}
bool RingBuffer_Read(RingBuffer_t *rb, uint8_t *data)
{
    if (RingBuffer_IsEmpty(rb))
    {
        return false;
    }

    *data = rb->buffer[rb->tail];

    rb->tail++;

    if (rb->tail >= RING_BUFFER_SIZE)
    {
        rb->tail = 0;
    }

    return true;
}
