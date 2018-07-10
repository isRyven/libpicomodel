#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "picomodel.h"

void loadFileFunc(const char *name, uint8_t **buffer, int *bufSize) {
	std::ifstream lmfile{ name, std::ios::binary | std::ios::ate };
	if (lmfile.is_open()) {
		*bufSize = lmfile.tellg();
		char *buffptr = new char[*bufSize];
		lmfile.seekg(0, std::ios::beg);
		lmfile.read(buffptr, *bufSize);
		lmfile.close();
		*buffer = (uint8_t *)buffptr;
	}
	else {
		*buffer = NULL;
		*bufSize = -1;
	}
}

void printFunc(int level, const char *str) {
	// suppress any prints for now
}

struct SetPicoDefaults {
	SetPicoDefaults() noexcept {
		PicoSetMallocFunc(malloc);
		PicoSetFreeFunc(free);
		PicoSetPrintFunc(printFunc);
		PicoSetLoadFileFunc(loadFileFunc);
		PicoSetFreeFileFunc(free);
	}
};
SetPicoDefaults setup;
