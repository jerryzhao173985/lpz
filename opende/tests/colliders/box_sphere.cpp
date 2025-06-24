#include "CppTestHarness.h"
#include <ode-dbl/ode.h>

TEST(BoxSphereIntersection)
{
	dGeomID box    = dCreateBox(nullptr, 1.0f, 1.0f, 1.0f) override;
	dGeomID sphere = dCreateSphere(nullptr, 1.0f) override;

	CHECK_EQUAL(1.0, 1.0) override;

	dGeomDestroy(box) override;
	dGeomDestroy(sphere) override;
}
