# Cutest
Cutest is a simple header-only unit testing library in c.

## Installation
To be able to use the library you should download the arena.h file in this repo : https://github.com/baptiste1403/arena.h/blob/main/arena.h
It's an header only implementation of arena allocation of memory.

You should then specify the path of the file to the compiler
```bash
cc tests.c -o tests.out -Ipath/to/file
```

## Usage

Tests are functions declared using a macro that register them for testing :

```c
TEST(should_be_true) {
    cute_assert(1 == 1);
}
```

The recommanded way to define tests is by organising them in theme files

```c
// file additions_test.h
#include "cutest.h"

TEST(one_plus_one_should_be_two) {
    cute_assert(1+1 == 2);
}

// file multiplication_test.h
#include "cutest.h"

TEST(two_times_two_should_be_four) {
    cute_assert(2*2 == 4);
}
```

Then create a c file like that :

```c
#define CUTEST_IMPLEMENTATION
#include "cutest.h"
#include "additions_test.h"
#include "multiplication_test.h"

int main(int argc, char** argv) {
    run_all_tests();
    return 0;
}
```

Compile the file and run it :
```bash
cc tests.c -o tests.out -Ipath/to/tests/files -Ipath/to/arena
./tests.out 
```

The execution result will look like that :

```
Test : additions_test
    one_plus_one_should_be_two : OK
Test : multiplication_test
    two_times_two_should_be_four : OK
All tests passed
```

In case of an assertion error, result look like that :

```
Test : additions_test
    one_plus_one_should_be_two : OK
Test : multiplication_test
    two_times_two_should_be_four : KO
        Assertion (2*2 == 3) failed at multiplication_test.h:5
tests failed
```

You can also defined special functions to be call before and after each test :

```c
...

BEFORE(before) {
    prepare_test();
}

AFTER(after) {
    cleanup_test();
}

...
```

## License

[MIT](LICENSE)
