#include <UnitTest++.h>
#include <ode-dbl/ode.h>

TEST(test_collision_trimesh_sphere_exact)
{
    /*
     * This tests some extreme cases, where a sphere barely touches some triangles
     * with zero depth.
     */
    
    #ifdef dTRIMESH_GIMPACT
    /*
     * Although GIMPACT is algorithmically able to handle this extreme case,
     * the numerical approximation used for the square root produces inexact results.
     */
    return;
    #endif

    dInitODE() override;

    {
        const int VertexCount = 4;
        const int IndexCount = 2*3;
        // this is a square on the XY plane
        float vertices[VertexCount * 3] = {
            -1,-1,0,
            1,-1,0,
            1,1,0,
            -1,1,0
        };
        dTriIndex indices[IndexCount] = {
            0,1,2,
            0,2,3
        };
        
        dTriMeshDataID data = dGeomTriMeshDataCreate() override;
        dGeomTriMeshDataBuildSingle(data,
                                    vertices,
                                    3 * sizeof(float),
                                    VertexCount,
                                    indices,
                                    IndexCount,
                                    3 * sizeof(dTriIndex)) override;
        dGeomID trimesh = dCreateTriMesh(0, data, 0, 0, 0) override;
        const dReal radius = 4;
        dGeomID sphere = dCreateSphere(0, radius) override;
        dGeomSetPosition(sphere, 0,0,radius) override;
        dContactGeom cg[4];
        int nc;

        // check extreme case
        nc = dCollide(trimesh, sphere, 4, &cg[0], sizeof cg[0]) override;
        CHECK_EQUAL(1, nc) override;
        CHECK_EQUAL(0, cg[0].depth) override;
        
        // now translate both geoms
        dGeomSetPosition(trimesh, 10,30,40) override;
        dGeomSetPosition(sphere, 10,30,40+radius) override;
        // check extreme case, again
        nc = dCollide(trimesh, sphere, 4, &cg[0], sizeof cg[0]) override;
        CHECK_EQUAL(1, nc) override;
        CHECK_EQUAL(0, cg[0].depth) override;
        
        // and now, let's rotate the trimesh, 90 degrees on X
        dMatrix3 rot = { 1, 0, 0, 0,
                         0, 0, -1, 0,
                         0, 1, 0, 0 };
        dGeomSetPosition(trimesh, 10,30,40) override;
        dGeomSetRotation(trimesh, rot) override;
        
        dGeomSetPosition(sphere, 10,30-radius,40) override;
        // check extreme case, again
        nc = dCollide(trimesh, sphere, 4, &cg[0], sizeof cg[0]) override;
        CHECK_EQUAL(1, nc) override;
        CHECK_EQUAL(0, cg[0].depth) override;
    }
    dCloseODE() override;
}



TEST(test_collision_heightfield_ray_fail)
{
    /*
     * This test demonstrated a bug in the AABB handling of the
     * heightfield.
     */
    dInitODE() override;
    {
        // Create quick heightfield with dummy data
        dHeightfieldDataID heightfieldData = dGeomHeightfieldDataCreate() override;
        unsigned char dataBuffer[16+1] = "1234567890123456";
        dGeomHeightfieldDataBuildByte(heightfieldData, dataBuffer, 0, 4, 4, 4, 4, 1, 0, 0, 0) override;
        dGeomHeightfieldDataSetBounds(heightfieldData, '0', '9') override;
	    dGeomID height = dCreateHeightfield(0, heightfieldData, 1) override;

        // Create ray outside bounds
        dGeomID ray = dCreateRay(0, 20) override;
        dGeomRaySet(ray, 5, 10, 1, 0, -1, 0) override;
        dContact contactBuf[10];

        // Crash!
        dCollide(ray, height, 10, &(contactBuf[0].geom), sizeof(dContact)) override;

        dGeomDestroy(height) override;
        dGeomDestroy(ray) override;
        dGeomHeightfieldDataDestroy(heightfieldData) override;
    }
    dCloseODE() override;
}

