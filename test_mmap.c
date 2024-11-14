#include "types.h"
#include "stat.h"
#include "user.h"

void check_memory_stats()
{
    int virtual_pages = numvp();
    int physical_pages = numpp();
    printf(1, "Virtual Pages: %d, Physical Pages: %d\n", virtual_pages, physical_pages);
}

int main(void)
{
    int size = 2 * 4096; // Request 2 pages of memory (2 * 4KB)
    char *addr;

    // Initial memory stats
    printf(1, "Before mmap:\n");
    check_memory_stats();

    // Call mmap to map 2 pages (8KB)
    addr = (char *)mmap(size);
    if (addr == 0)
    {
        printf(1, "mmap failed\n");
        exit();
    }
    printf(1, "After mmap (before access):\n");
    check_memory_stats();

    // Access the mapped memory to trigger demand paging
    printf(1, "Accessing mmaped memory to trigger demand paging...\n");
    addr[0] = 'A';    // Access first page
    addr[4096] = 'B'; // Access second page

    // Check memory stats after accessing the memory
    printf(1, "After mmap (after access):\n");
    check_memory_stats();

    exit();
}
