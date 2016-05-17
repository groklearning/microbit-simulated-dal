#ifndef MICROBIT_HEAP_ALLOCTOR_H
#define MICROBIT_HEAP_ALLOCTOR_H

/**
  * Initialise the microbit heap according to the parameters defined in MicroBitConfig.h
  * After this is called, any future calls to malloc, new, free or delete will use the new heap.
  * n.b. only code that #includes MicroBitHeapAllocator.h will use this heap. This includes all micro:bit runtime
  * code, and user code targetting the runtime. External code can choose to include this file, or
  * simply use the standard mbed heap.
  *
  * @return MICROBIT_OK on success, or MICROBIT_NO_RESOURCES if the heap could not be allocted.
  */
int microbit_heap_init();

/**
  * Attempt to allocate a given amount of memory from any of our configured heap areas.
  * @param size The amount of memory, in bytes, to allocate.
  * @return A pointer to the allocated memory, or NULL if insufficient memory is available.
  */
void *microbit_malloc(size_t size);


/**
  * Release a given area of memory from the heap. 
  * @param mem The memory area to release.
  */
void microbit_free(void *mem);

#endif
