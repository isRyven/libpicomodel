#include "catch.hpp"
#include "picomodel.h"
#include "utils.h"

TEST_CASE("Should load simple mdl") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.mdl", 0);
	REQUIRE(model != NULL);
	PicoFreeModel(model);
}

TEST_CASE("Should correctly parse simple mdl") {
	picoModel_t *model = PicoLoadModel("../tests/assets/model.mdl", 0);
	int numSurfaces = PicoGetModelNumSurfaces(model);
	REQUIRE(numSurfaces == 1);
	picoSurface_t *surface = PicoGetModelSurface(model, 0);
	int numShaders = PicoGetModelNumShaders(model);
	REQUIRE(numShaders == 1);
	picoShader_t *shader = PicoGetSurfaceShader(surface);
	REQUIRE(std::string("../tests/assets/model_img") == PicoGetShaderName(shader));
	int numVerts = PicoGetSurfaceNumVertexes(surface);
	REQUIRE(numVerts == 6);
	int numInds = PicoGetSurfaceNumIndexes(surface);
	REQUIRE(numInds == 6);

	//picoVec3_t verts[] = { { 1.f, 0.0f, 1.0f },{ 1.f, 0.0f, -1.0f },{ -1.f, 0.0f, -1.0f },{ -1.f, 0.0f, 1.0f } };
	//for (int i = 0; i < numVerts; i++) {
	//	REQUIRE(comparevec<3>(PicoGetSurfaceXYZ(surface, i), verts[i]));
	//}

	//picoVec3_t cvn{ 0.0f, 1.0f, 0.0f };
	//picoVec2_t sts[] = { { 1.f, 0.f },{ 1.f, 1.f },{ 0.0f, 1.0f },{ 0.0f, 0.0f } };
	//for (int n = 0; n < numVerts; n++) {
	//	REQUIRE(comparevec_approx<3>(PicoGetSurfaceNormal(surface, n), cvn, 0.001f)); // normal
	//	REQUIRE(comparevec<2>(PicoGetSurfaceST(surface, 0, n), sts[n])); // st
	//}

	PicoFreeModel(model);
}
