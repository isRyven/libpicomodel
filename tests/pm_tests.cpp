#include "catch.hpp"
#include "picomodel.h"
#include <string>

template<int n, typename T>
bool comparevec(T a, T b) {
	for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
	return true;
}

// Tests and example usages for libpicomodel public APIs

/*
	Basically, what you want to use to load model is PicoLoadModel function, which accepts filepath and frame, 
	and returns loaded model you can examine and use data from using the public APIs defined in picomodel.h.
	It automatically selects the right model loader by reading model header.

	picomodel.h contains getters to fetch different types of data, including shaders and surfaces:
		#include "picomodel.h"
		...
		picoModel_t *model = PicoLoadModel("models/tree.md3", 0);
		int surfaceCount = PicoGetModelNumSurfaces(model);
		for (int i = 0; i < surfaceCount; i++) {
			picoSurface_t *surface = PicoGetModelSurface(model, i);
			picoShader_t *shader = PicoGetSurfaceShader(surface);
			int numVertexes = PicoGetSurfaceNumVertexes(surface);
			// ...
		}

	In order to use PicoLoadModel, one should first set malloc, print and file handlers, using next functions:
		PicoSetMallocFunc(malloc); -> has default setting (malloc), not required
		PicoSetFreeFunc(free); -> has default setting (free), not required
		PicoSetPrintFunc(printFunc); -> messages go into: void(int level, const char *str)
			note: levels are defined in picoPrintLevel_t
		PicoSetLoadFileFunc(loadFileFunc); -> handles all file loads: void(const char *name, unsigned char **buffer, int *bufSize)
		PicoSetFreeFileFunc(freeFileFunc); -> void(void *buffer)

	However, if you want to build own model loader, you would need to use both apis from picomodel.h, and picointernal.h,
	which contain all facilities you would want to use to build a proper model loader.

	Here are tested APIs that are only related to the module creation exposed in picomodel.h. For better understanding 
	consider looking at real modules (such as md3 one).
*/

/*	PicoNewModel allocates model to hold shader and surface data. Usually you would use PicoLoadModel to do that, 
	which essentially uses specific module to load, parse, instantiate and fill the model. */
TEST_CASE("Should allocate new model") {
    picoModel_t *model = PicoNewModel();
    REQUIRE(model != NULL);
	/*	We are not free it up properly yet, because model freeing procedure would currently try to free up unallocated resources, 
		which can lead to the crash. */
	delete model;
}

/*	To do proper model freeing, we should first call PicoAdjustModel, which essentially allocates shader 
	and surface pointer storage (if required), that will be used to hold pointers for allocated shaders and surfaces.
	(storage type: picoShader_t **shader). */
TEST_CASE("Should allocate shader and surface pointers in the model") {
	picoModel_t *model = PicoNewModel();
	/*	Increase shader and surface number, allocate pointer storage for them. */
	PicoAdjustModel(model, 9, 9);
	REQUIRE(model->shader != NULL);
	REQUIRE(model->surface != NULL);
	REQUIRE(model->numShaders == 9);
	REQUIRE(model->numSurfaces == 9);
	/*	Each reallocation would increase storage on 16 pointers. */
	REQUIRE(model->maxShaders == PICO_GROW_SHADERS);
	REQUIRE(model->maxSurfaces == PICO_GROW_SURFACES);
	/*	Exceed number of maxShaders/maxSurfaces -> expand the storage. */
	PicoAdjustModel(model, 18, 18);
	REQUIRE(model->maxShaders == PICO_GROW_SHADERS * 2);
	REQUIRE(model->maxSurfaces == PICO_GROW_SURFACES * 2);
	/*	Free it up manually atm, since we don't have any real shaders or surfaces added yet.
		Freeing up resources would cause crash, because pointerstorage pointers are not referencing 
		any real data yet and model->numShaders is set to non zero value, which is used to free up 
		inner resources. */
	delete model->shader;
	delete model->surface;
	delete model;
}

/*	PicoFreeModel is taking care of freeing up any allocated resources model could have:
	shaders and its resources, surfaces and its resources and shader/surface pointer storages. */
TEST_CASE("Should free model and allocated shader and surface pointers storage") {
	picoModel_t *model = PicoNewModel();
	/*	Allocates pointerstorages for shaders and surfaces (picoShader_t **shader, picoSurface_t **surface) in model,
		note, these are not real shaders or surfaces, just pointers to them, the storage can dynamically expand, increasing
		the number of shaders/surfaces it can reference to.
		It also sets numShaders/numSurfaces fields to be non 0, which are used to free up any allocated shaders or surfaces.
		Since we are not allocating any real shaders or surfaces yet, we should set the numShaders and numSurfaces back to 0,
		but the pointerstorage should remain allocated, which should be deallocated after PicoFreeModel call. */
	PicoAdjustModel(model, 9, 9);
	/*	Change numShaders, numSurfaces back to 0, but don't touch pointertorage. */
	PicoAdjustModel(model, 0, 0);
	/*	So, atm it should free up pointerstorages(for both shaders and surfaces) and model. Having numShaders and 
		numSurfaces set to 0 will avoid freeing up anything that pointers in storage could point to - since we don't
		have any real data there yet. */
	PicoFreeModel(model);
}

/*
	PicoNewShader allocates shader structure as well as shader pointerstorage (picoShader_t **shader)
	in the mode if neeeded, so you don't need to call PicoAdjustModel manually.
*/
TEST_CASE("Should allocate new shader and attach it to the model") {
	picoModel_t *model = PicoNewModel();
	picoShader_t *shader = PicoNewShader(model);
	/*	Should allocate pointerstorage for shaders. */
	REQUIRE(model->shader != NULL);
	REQUIRE(model->maxShaders == PICO_GROW_SHADERS);
	/*	First pointer in storage should reference our shader. */
	REQUIRE(model->shader[0] == shader);
	REQUIRE(model->numShaders == 1);
	/*  Do manual clean up atm, to test model free in separate unit. */
	delete shader;
	/*	Pointerstorage (holds 16 pointers at the start), release the resources. */
	delete model->shader; 
	delete model;
}

/*	Check PicoFreeModel for segfaultness upon freeing up shader resources. */
TEST_CASE("Should free model, shader pointers storage and shader itself") {
	picoModel_t *model = PicoNewModel();
	picoShader_t *shader = PicoNewShader(model);
	/*	Should free up any allocated shaders, shader pointer storage and model. */
	PicoFreeModel(model);
}

/*  Pretty much same as shaders, except it has more allocatable stuff inside: vertices, normals, 
	indexes, texture coords, smoothing groups, colors. */
TEST_CASE("Should allocate new surface and attach it to the model") {
	picoModel_t *model = PicoNewModel();
	/*	Calls PicoAdjustModel internally to allocate surface pointer storage. */
	picoSurface_t *surface = PicoNewSurface(model);
	REQUIRE(model->surface != NULL);
	REQUIRE(model->maxSurfaces == PICO_GROW_SURFACES);
	REQUIRE(model->surface[0] == surface);
	REQUIRE(model->numSurfaces == 1);
	/*	Manual delete atm. */
	delete surface;
	delete model->surface;
	delete model;
}

/*	Check PicoFreeModel for segfaultness upon freeing up surface resources. */
TEST_CASE("Should free model, surface pointers storage and surface itself") {
	picoModel_t *model = PicoNewModel();
	picoSurface_t *surface = PicoNewSurface(model);
	/*	Internally surface will clean up its own pointerstorages, which are not allocated by themselves without 
		explicitly calling PicoAdjustSurface.
		Calling PicoAdjustSurface sets numbers for vertexes, normals, indices, etc, which causes pointerstorage reallocation, 
		to fit all these resources are meant to be set in surface. */
	PicoAdjustSurface(surface, 1, 1, 1, 1, 1);
	REQUIRE(surface->maxVertexes == PICO_GROW_VERTEXES);
	REQUIRE(surface->numVertexes == 1);
	REQUIRE(surface->xyz != NULL);
	REQUIRE(surface->normal != NULL);
	REQUIRE(surface->smoothingGroup != NULL);

	REQUIRE(surface->maxIndexes == PICO_GROW_INDEXES);
	REQUIRE(surface->numIndexes == 1);
	REQUIRE(surface->index != NULL);

	REQUIRE(surface->maxFaceNormals == PICO_GROW_FACES);
	REQUIRE(surface->numFaceNormals == 1);
	REQUIRE(surface->faceNormal != NULL);

	REQUIRE(surface->maxSTArrays == PICO_GROW_ARRAYS);
	REQUIRE(surface->numSTArrays == 1);
	REQUIRE(surface->st != NULL);

	REQUIRE(surface->maxColorArrays == PICO_GROW_ARRAYS);
	REQUIRE(surface->numColorArrays == 1);
	REQUIRE(surface->color != NULL);
	/*	Setting resource numbers to 0 so we don't deallocate any resources we don't have yet, same as we did with model, 
		the pointer storages and single chunks are still going to remain however, which are going to be release upon 
		PicoFreeModel call. So basically we are only care about pointerstorages, which may point to garbage if no 
		actual data was set, if for instance numColor is set to higher than 0, though no real colors were set. */
	PicoAdjustSurface(surface, 0, 0, 0, 0, 0);
	/*	Should free up any allocated surfaces, its resources, surface pointerstorage and model */
	PicoFreeModel(model);
}

TEST_CASE("Should free model, surface pointers storage, surface resources and surface") {
	picoModel_t *model = PicoNewModel();
	picoSurface_t *surface = PicoNewSurface(model);
	PicoAdjustSurface(surface, 1, 1, 1, 1, 1);
	/*	Only st and color have poiter storages, to reference actual data, rest(verts, normals) are array chunks. */
	picoColor_t color{ 255, 0, 255, 255 };
	picoVec2_t st{ 0, 0 };
	/*	Set color for 1st colors array for 1st vertex */
	PicoSetSurfaceColor(surface, 0, 0, color);
	REQUIRE(comparevec<4>(surface->color[0][0], color));
	/*  same */
	PicoSetSurfaceST(surface, 0, 0, st);
	REQUIRE(comparevec<2>(surface->st[0][0], st));
	/*	Should now aswell free up resources in pointer storages (color, st). */
	PicoFreeModel(model);
}

/*	Basic usage, allocate model, allocate surface, allocate shader, fill data, free up model */
TEST_CASE("Should be able to initilize complete model") {
	picoModel_t *model = PicoNewModel();
	/*	Set model name, usually derived from the model itself */
	PicoSetModelName(model, "triangle");
	REQUIRE(std::string("triangle") == model->name);
	/*	set model file path */
	PicoSetModelFileName(model, "triangle.model");
	REQUIRE(std::string("triangle.model") == model->fileName);
	/*	set frame (model derived from) */
	PicoSetModelFrameNum(model, 0);
	REQUIRE(model->frameNum == 0);
	/*	total number of frames available */
	PicoSetModelNumFrames(model, 1);
	REQUIRE(model->numFrames == 1);

	picoSurface_t *surface = PicoNewSurface(model);
	PicoSetSurfaceType(surface, PICO_TRIANGLES);
	REQUIRE(surface->type == PICO_TRIANGLES);
	PicoSetSurfaceName(surface, "triangle_surface_1");
	REQUIRE(std::string("triangle_surface_1") == surface->name);

	picoShader_t *shader = PicoNewShader(model);
	PicoSetShaderName(shader, "triangle_shader_1");
	REQUIRE(std::string("triangle_shader_1") == shader->name);
	/*	Each surface can only reference single shader. */
	PicoSetSurfaceShader(surface, shader);
	REQUIRE(surface->shader == shader);

	const int numTris = 3;
	/*	Fill in sequence of indecies. */
	for (int t = 0; t < numTris; t++) {
		PicoSetSurfaceIndex(surface, t * 3 + 0, t * 3 + 0);
		PicoSetSurfaceIndex(surface, t * 3 + 1, t * 3 + 1);
		PicoSetSurfaceIndex(surface, t * 3 + 2, t * 3 + 2);
	}

	for (int t = 0; t < numTris; t++) {
		REQUIRE(surface->index[t * 3 + 0] == t * 3 + 0);
		REQUIRE(surface->index[t * 3 + 1] == t * 3 + 1);
		REQUIRE(surface->index[t * 3 + 2] == t * 3 + 2);
	}

	picoColor_t color{ 255, 255, 255, 255 };
	const int numVerts = 9;
	/*	Fill in vertix data, including vertex position, normal, color and texture coords. */
	for (int v = 0; v < numVerts; v++) {
		picoVec3_t xyz{ 10.f + v * 10.f, 20.f + v * 10.f, 30.f + v * 10.f };
		/*	Set vertex position where v is an index of vertex. */
		PicoSetSurfaceXYZ(surface, v, xyz);
		picoVec3_t normal{ 1.f, 1.f, 0.f };
		/*	Set normal vector where v is an index of vector. */
		PicoSetSurfaceNormal(surface, v, normal);
		picoVec2_t st{ v & 2, v & 2 }; // 0, 0 or 1, 1
		/*	Set texture coordinate in the first texture coordinates array in index v, that matches vertex index. */
		PicoSetSurfaceST(surface, 0, v, st);
		PicoSetSurfaceColor(surface, 0, v, color);
	}

	for (int v = 0; v < numVerts; v++) {
		picoVec3_t xyz{ 10.f + v * 10.f, 20.f + v * 10.f, 30.f + v * 10.f };
		REQUIRE(comparevec<3>(*(surface->xyz + v), xyz));
		picoVec3_t normal{ 1.f, 1.f, 0.f };
		REQUIRE(comparevec<3>(*(surface->normal + v), normal));
		picoVec2_t st{ v & 2, v & 2 };
		REQUIRE(comparevec<2>(*(surface->st[0] + v), st));
		REQUIRE(comparevec<4>(*(surface->color[0] + v), color));
	}

	/*	The model now contains all needed data to work with, both picoSurface_t and picoShader_t have additional fields 
		that could be filled, but they are not used in Q3Map2 for instance. */

	/*	Should free up resources. */
	PicoFreeModel(model);
}
