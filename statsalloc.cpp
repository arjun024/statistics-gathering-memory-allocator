#define HEADER_SIZE sizeof(Header)
#define KiB16 16384
#define MB512 536870912
#define KiB16_CLASS 1024
#define MB512_CLASS 1039


template <class SourceHeap>
void * StatsAlloc<SourceHeap>::malloc(size_t sz) {
  int size_class;
  size_t rounded_sz, total_sz;
  void * heap_mem;
  Header *free_chunk;

  size_class = getSizeClass(sz);
  if (size_class < 0)
    return NULL;

  /* round to the next class size */
  rounded_sz = getSizeFromClass(size_class);
  if (!rounded_sz)
    return NULL;

  /* We try to get memory from the free list.
   * If the corresponding free list has no chunks left,
   * we look for memory from the SourceHeap.
   */
  free_chunk = freedObjects[size_class];
  if(free_chunk) {
    /*remove the first chunk from this free list and give
    * it to the caller */
    freedObjects[size_class] = free_chunk->nextObject;
    if (free_chunk->nextObject) {
      freedObjects[size_class]->prevObject = NULL;
    }
    goto success;
  }


  total_sz = HEADER_SIZE + rounded_sz;
  heap_mem = SourceHeap::malloc(total_sz);
  if (!heap_mem) {
    perror("Out of Memory!!");
    return NULL;
  }
  free_chunk = (Header*) heap_mem;
  free_chunk->requestedSize = sz;
  free_chunk->allocatedSize = rounded_sz;

success:
  /* Connect it to the doubly linked list tailed by @allocatedObjects */
  if (!allocatedObjects) {
    allocatedObjects = free_chunk;
    free_chunk->prevObject = free_chunk->nextObject = NULL;
  } else {
    allocatedObjects->nextObject = free_chunk;
    free_chunk->prevObject = allocatedObjects;
    free_chunk->nextObject = NULL;
    allocatedObjects = free_chunk;
  }

  /* A little stats */
  allocated += rounded_sz;
  requested += sz;
  if (allocated > maxAllocated)
    maxAllocated = allocated;
  if (requested > maxRequested)
    maxRequested = requested;
  return (void*)(free_chunk + 1);
}



template <class SourceHeap>
void StatsAlloc<SourceHeap>::free(void * ptr) {
  Header *header, *freelist;
  int size_class;
  header = (Header*)ptr - 1;

  /* Disconnect from SourceHeap's doubly linked list tailed by @allocatedObjects */
  if (header == allocatedObjects)
    allocatedObjects = header->prevObject;
  if (header->prevObject)
    header->prevObject->nextObject = header->nextObject;
  if (header->nextObject)
    header->nextObject->prevObject = header->prevObject;
  header->prevObject = header->nextObject = NULL;

  /* Connect to its free list chain */
  size_class = getSizeClass(header->allocatedSize);
  freelist = freedObjects[size_class];
  freedObjects[size_class] = header;
  header->prevObject = NULL;
  if (!freelist) {
    header->nextObject = NULL;
    goto update_stats;
  }
  header->nextObject = freelist;
  freelist->prevObject = header;

update_stats:
  allocated -= header->allocatedSize;
  requested -= header->requestedSize;
}



template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::getSize(void * p) {
  Header *header;
  header = (Header*)p - 1;
  return header->allocatedSize;
}



// number of bytes currently allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesAllocated() {
  return allocated;
}



// max number of bytes allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesAllocated() {
  return maxAllocated;
}



// number of bytes *requested* (e.g., malloc(4) might result in an allocation of 8; 4 = bytes requested, 8 = bytes allocated)
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesRequested() {
  return requested;
}



// max number of bytes *requested*
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesRequested() {
  return maxRequested;
}



template <class SourceHeap>
void StatsAlloc<SourceHeap>::walk(const std::function< void(Header *) >& f) {
  // FIX ME
}



template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::getSizeFromClass(int index) {
  size_t class_sz;
  if (index <= KiB16_CLASS)
    return (size_t)(index * 16);
  class_sz = (size_t) pow(2, KiB16_CLASS - index + 16);
  /* check for overflow */
  if (!class_sz) {
    perror("Out of memory!");
    return 0;
  }
}



template <class SourceHeap>
int StatsAlloc<SourceHeap>::getSizeClass(size_t sz) {
  /* The standard says that size of size_t ie. SIZE_MAX must be at least 65535 */
  /* The size of sizeClass excludes the header size */

  if (sz <= 0 || sz > SIZE_MAX || sz > MB512)
    return -1;

  /* 
   * problem says: Your size classes should be exact multiples of 16
   * for every size up to 16384, and then powers of two from 16,384 to 512 MB
   * Class0 is kept unused for easy calculations.
   * Class1 to Class1039 is valid.
   */

  if (sz <= KiB16)
    return (int) ceil(sz / 16.0);

  return (int) ceil(log2(sz));
}
