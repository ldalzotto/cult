### LIBC USAGE

Unless explicitely asked, don't use the libc api.
For primitives, you have access to u8, i8, u16, i16m... that are typedefs to C types.

### Example

```c
#include "primitive.h"

u32 value = 100;
```
