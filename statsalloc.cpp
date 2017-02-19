#define HEADER_SIZE sizeof(Header)

template <class SourceHeap>
void * StatsAlloc<SourceHeap>::malloc(size_t sz) {
  int size_class;
  size_t rounded_sz, total_sz;
  void * heap_mem;
  Header *free_chunk;

  printf("%s\n", "inside StatsAlloc::malloc");
  size_class = getSizeClass(sz);
  if (size_class < 0)
    return NULL;

  /* We try to get memory from the free list.
   * If the corresponding free list has no chunks left,
   * we take it from the mmap-ed part.
   */
  free_chunk = freedObjects[size_class];
  if(free_chunk) {
    printf("%s\n", "found a free list");
    /*remove the first chunk from this free list and give
    * it to the caller */
    freedObjects[size_class] = free_chunk->nextObject;
    if (free_chunk->nextObject) {
      freedObjects[size_class]->prevObject = NULL;
    }
    free_chunk->prevObject = free_chunk->nextObject = NULL;
    return free_chunk;
  }

  printf("%s\n", "checkin mmaped mem");
  /* round to the next class size */
  rounded_sz = getSizeFromClass(size_class);
  if (!rounded_sz)
    return NULL;

  total_sz = HEADER_SIZE + rounded_sz;
  heap_mem = SourceHeap::malloc(total_sz);
  if (!heap_mem) {
    perror("Out of Memory!!");
    return NULL;
  }
  free_chunk = (Header*) heap_mem;
  free_chunk->requestedSize = sz;
  free_chunk->allocatedSize = rounded_sz;
  free_chunk->prevObject = free_chunk->nextObject = NULL;
  return (void*)(free_chunk + 1);
}
  
template <class SourceHeap>
void StatsAlloc<SourceHeap>::free(void * ptr) {
  // FIX ME
}

template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::getSize(void * p) {
  return 0; // FIX ME
}

// number of bytes currently allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesAllocated() {
  return 0; // FIX ME
}

// max number of bytes allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesAllocated() {
  return 0; // FIX ME
}

// number of bytes *requested* (e.g., malloc(4) might result in an allocation of 8; 4 = bytes requested, 8 = bytes allocated)
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesRequested() {
  return 0; // FIX ME
}
  
// max number of bytes *requested*
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesRequested() {
  return 0; // FIX ME
}

template <class SourceHeap>
void StatsAlloc<SourceHeap>::walk(const std::function< void(Header *) >& f) {
  // FIX ME
}

#define KiB16 16384
#define MB512 536870912
#define KiB16_CLASS 1024
#define MB512_CLASS 1039

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

  printf("%s\n", "in getSizeClass");
  if (sz <= 0 || sz > SIZE_MAX || sz > MB512)
    return -1;

  /* 
   * problem says: Your size classes should be exact multiples of 16
   * for every size up to 16384, and then powers of two from 16,384 to 512 MB
   * Class0 is kept unused for easy calculations.
   * Class1 to Class1039 is valid.
   */

  if (sz <= KiB16)
    return (int) ceil(sz / 16);

  return (int) ceil(log2(sz));
}
