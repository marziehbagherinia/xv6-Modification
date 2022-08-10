#include "types.h"

#define PROT_NONE       0  // pages are inaccessible
#define PROT_WRITE      1  // pages are writable
#define PROT_READ       2  // pages are readable

#define MAP_ANONYMOUS   0
#define MAP_SHARED        1
#define MAP_PRIVATE     2

#define MAP_FAILED      ((void *)-1) // failed mmap
#define UNMAP_FAILED    -1           // failed munmap
#define UNMAP_SUCCESS   0            // successful munmap