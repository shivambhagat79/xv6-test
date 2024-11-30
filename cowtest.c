#include "types.h"
#include "stat.h"
#include "user.h"

int j = 5; // Shared variable for parent and child

int main(void)
{
    int pid;
    int initial_free_pages, free_pages_after_fork, free_pages_after_child_write;
    
    // Initial number of free pages
    initial_free_pages = getNumFreePages();
    printf(1, "Initial number of free pages: %d\n", initial_free_pages);

    // Fork a child process
    pid = fork();
    if (pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }

    if (pid == 0) { // Child process
        // Number of free pages after fork
        int free_pages_child = getNumFreePages();
        printf(1, "[CHILD] Free pages after fork: %d\n", free_pages_child);

        // Write to the shared variable `j` (should trigger CoW)
        printf(1, "[CHILD] Value of j before write: %d\n", j);
        j = 7; // Modify the shared variable
        printf(1, "[CHILD] Value of j after write: %d\n", j);

        // Number of free pages after writing
        free_pages_after_child_write = getNumFreePages();
        printf(1, "[CHILD] Free pages after write: %d\n", free_pages_after_child_write);

        // Verify that the number of free pages decreased due to CoW
        printf(1, "[CHILD] Number of pages allocated for CoW: %d\n", free_pages_child - free_pages_after_child_write);
        exit();
    } else { // Parent process
        // Number of free pages after fork
        wait();   // Wait for the child to finish
        free_pages_after_fork = getNumFreePages();
        printf(1, "[PARENT] Free pages after fork: %d\n", free_pages_after_fork);

        // Verify the parent process's variable remains unchanged
        printf(1, "[PARENT] Value of j after child writes: %d\n", j);

        // Final number of free pages
        int final_free_pages = getNumFreePages();
        printf(1, "[PARENT] Final number of free pages: %d\n", final_free_pages);
    }

    exit();
}
