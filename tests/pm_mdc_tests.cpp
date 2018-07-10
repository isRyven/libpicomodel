#include "catch.hpp"
#include "picomodel.h"
#include "utils.h"

TEST_CASE("Should load simple mdc") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.mdc", 0);
	REQUIRE(model != NULL);
	PicoFreeModel(model);
}

TEST_CASE("Should correctly parse simple mdc") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.mdc", 0);
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

	picoVec3_t v1{ -1.f, -1.0f, 0.0f };
	picoVec3_t v2{ 1.0f, -1.f, 0.0f };
	picoVec3_t v3{ -1.f, 1.0f, 0.0f };
	picoVec3_t v4{ 1.f, 1.0f, 0.0f };
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 0), v1));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 1), v2));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 2), v3));
	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, 3), v4));

	picoVec3_t cvn{ 0.0f, 0.0f, 1.0f };
	picoVec2_t sts[] = { { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.f, 0.f }, { 1.f, 0.f } };
	for (int n = 0; n < numVerts; n++) {
		REQUIRE(comparevec_approx<3>(PicoGetSurfaceNormal(surface, n), cvn, 0.001f)); // normal
		REQUIRE(comparevec<2>(PicoGetSurfaceST(surface, 0, n), sts[n])); // st
	}

	PicoFreeModel(model);
}
