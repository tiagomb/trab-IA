#include <stdio.h>

#define DEBUG 1

void print_debug(const char *message) {
	if (DEBUG) {
		printf("[DEBUG] %s\n", message);
	}
}

