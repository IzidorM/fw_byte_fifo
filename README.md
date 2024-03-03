# byte fifo
This is one of the cleanest and fastest implementation of a byte fifo queue.
It is perfect for transfering data from irq rutine to the main loop or RTOS task.

## Features
* Written in C
* Should be one of the smallest by code size and fastest implementation available
* Single producer and single consumer thread safe
* Initialization using dynamic or static memory allocation
* BSD license

## Usage

First the fifo needs to be initialised. There are 2 ways how this can be done.

### Initializing using dynamic memory allocation
The prefered way of initializing the byte fifo is by using the "dynamic" memory allocation,
where you provide your own memory allocation function to the settings structure. This way 
you can fake dynamic memory allocation in case you dont have support.

``` c
#include <stdint.h>
#include "byte_fifo.h"

// memory size for the data must be power of 2
static uint8_t mem[8];

struct byte_fifo *example_fifo_init_with_dynamic_memory_allocation(void)
{

	struct byte_fifo_settings settings = {
		.fifo_buff = mem,
		.fifo_size = sizeof(mem),
		.my_malloc = malloc,
	};

    struct byte_fifo *f = byte_fifo_init(&settings);

    // fifo initialization can fail, so test the return value
    assert(f);

    return f;
}

```

### Initializing using static memory allocation
There is a way to avoid using the dynamic memory allocation. You can get access to internal
fifo structure by including the byte_fifo_internal.h header...

``` c
#include <stdint.h>
#include "byte_fifo.h"
#include "byte_fifo_internal.h"

static struct byte_fifo byte_fifo_01;

// memory size for the data must be power of 2
static uint8_t byte_fifo_01_mem[8];

enum rstatus example_byte_fifo_init_with_static_memory_allocation(void)
{

    return byte_fifo_init_internal(&byte_fifo_01, byte_fifo_01_mem, 
                                 sizeof(byte_fifo_01_mem)));

}

```

## Byte fifo Design Considerations

### Memory Size Constraint:

Memory size must be a power of 2 for efficient head/tail index
wrapping using a bit mask, a common optimization in circular
buffers. The size is also limited to 65535 bytes, which should be
enought for most embedded systems. This limitation is because uint16_t
variables are used to hold the head/tail index values...

### No Full or Empty Checks:

Write and read functions lack checks for a full or empty byte fifo to
optimize performance. Undefined behavior may occur if used
incorrectly. To get a safe version of read/write you need to wrap them
in is_full or is_empty functions as shown below:

``` c

void some_function_getting_data_from_fifo(void)
{
    uint8_t d;
    if (!byte_fifo_is_empty(&f))
    {
        uint8_t d = byte_fifo_read(&f);
        // do something with the data
    }
}

void some_function_putting_data_to_fifo(uint8_t data_byte)
{

    if (!byte_fifo_is_empty(&f)) //only one check is needed
    {
        byte_fifo_write(&f, data_byte);
    }
    else
    {
        // hadle full fifo condition
    }
}

```

If there is a need to get multiple bytes from the fifo, you can
optimize this by using the byte_fifo_get_fill_count like this

``` c
void some_function_getting_data_from_fifo(void)
{
    uint16_t elements_if_fifo = 
        byte_fifo_get_current_elements_count(&f);

    for (uint32_t i = 0; elements_if_fifo > i; i++)
    {
        uint8_t d = byte_fifo_read(&f);
        // do something with the data
    }
}

```

## Unit tests
There are unit tests provided in the unit_test folder. 
Correcting the path to Unity in Makefile in needed before first
run. Other than that they should run out of the box on any linux
system with gcc installed. 

## Size and speed comparison
TODO


