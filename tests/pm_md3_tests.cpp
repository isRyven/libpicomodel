#include "catch.hpp"
#include "picomodel.h"
#include <cinttypes>
#include <fstream>
#include <string>
#include <cmath>

template<int n, typename T>
bool comparevec(T a, T b) {
	for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
	return true;
}

template<int n, typename T>
bool comparevec_approx(T a, T b, float epsilon) {
	for (int i = 0; i < n; i++) if (std::fabs(a[i] - b[i]) > epsilon) return false;
	return true;
}

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

TEST_CASE("Should load simple md3") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.md3", 0);
	REQUIRE(model != NULL);
	PicoFreeModel(model);
}

TEST_CASE("Should correctly parse simple md3") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.md3", 0);
	int numSurfaces = PicoGetModelNumSurfaces(model);
	REQUIRE(numSurfaces == 1);
	picoSurface_t *surface = PicoGetModelSurface(model, 0);
	int numShaders = PicoGetModelNumShaders(model);
	REQUIRE(numShaders == 1);
	picoShader_t *shader = PicoGetSurfaceShader(surface);
	REQUIRE(std::string("myshader_1") == PicoGetShaderName(shader));
	int numVerts = PicoGetSurfaceNumVertexes(surface);
	REQUIRE(numVerts == 4);
	int numInds = PicoGetSurfaceNumIndexes(surface);
	REQUIRE(numInds == 6);
	REQUIRE(std::string("surf0") == PicoGetSurfaceName(surface));

	picoVec3_t v1{ -1.f, 0.0f, 1.0f };
	picoVec3_t v2{ 1.f, 0.0f, 1.0f };
	picoVec3_t v3{ 1.f, 0.0f, -1.0f };
	picoVec3_t v4{ -1.f, 0.0f, -1.0f };
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 0), v1));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 1), v2));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 2), v3));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 3), v4));

	picoVec3_t cvn{ 0.0f, 1.0f, 0.0f };
	for (int n = 0; n < numVerts; n++) {
		REQUIRE(comparevec_approx<3>(PicoGetSurfaceNormal(surface, n), cvn, 0.001f));
	}

	PicoFreeModel(model);
}
