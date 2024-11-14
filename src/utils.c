#include <stdio.h>

#include "utils.h"

void print_debug(const char *message) {
	if (DEBUG) {
		printf("[DEBUG] %s\n", message);
	}
}

