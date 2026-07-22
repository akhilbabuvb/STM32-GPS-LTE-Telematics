#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 128

typedef struct
{
    uint8_t buffer[RING_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;

} RingBuffer_t;

void RingBuffer_Init(RingBuffer_t *rb);
bool RingBuffer_Write(RingBuffer_t *rb, uint8_t data);
bool RingBuffer_IsFull(RingBuffer_t *rb);
bool RingBuffer_IsEmpty(RingBuffer_t *rb);
bool RingBuffer_Read(RingBuffer_t *rb, uint8_t *data);

#endif /* RING_BUFFER_H */
