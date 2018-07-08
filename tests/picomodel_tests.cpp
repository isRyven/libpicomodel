#include "catch.hpp"
#include "picomodel.h"

TEST_CASE("Should allocate new model and then free it") {
    picoModel_t *model = PicoNewModel();
    REQUIRE(model != NULL);
    PicoFreeModel(model);
}
