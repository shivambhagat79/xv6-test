#include "types.h"
#include "user.h"

int main() {
    int physical_pages = numpp();

    printf(1, "Number of physical pages in user address space: %d\n", physical_pages);

    exit();
}
