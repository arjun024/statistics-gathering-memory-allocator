#include <iostream>
using namespace std;

#include "statsalloc.h"

int main()
{
  reportStats();
  cout << "-----------" << endl;
  for (int i = 0; i < 10; i++) {
    char * ptr = (char *) malloc(171);
    if (!ptr) {
      printf("%s\n", "malloc returned NULL");
      return 0;
    }
    *ptr = 'a' + i;
    *(ptr + 1) = '\0';
    cout << ptr << endl;
    /* printf("%p\n", (void*)ptr); */
    /* free(ptr); */
  }
  reportStats();
  printf("%s\n", "* walk() itself causes some memory allocation as a side effect due to std::function. This is not hidden from the caller.");
  walk([&](Header * h){ cout << "Object address = " << (void *) (h + 1) << ", requested size = " << h->requestedSize << ", allocated size = " << h->allocatedSize << endl; });
  return 0;
}
