#include "types.h"
#include "user.h"
int main()
{
    int virtual_pages = numvp();
    printf(1, "Number of virtual pages (including stack guard page): %d\n", virtual_pages);
    exit();
}