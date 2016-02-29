#ifndef TILER_TEST_MESH_H
#define TILER_TEST_MESH_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/mesh.h"

/// Test code for @ref Mesh
class TestMesh : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMesh);
    CPPUNIT_TEST(testBunny);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testBreak);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testPinch);
    CPPUNIT_TEST(testOverlap);
    CPPUNIT_TEST_SUITE_END();

private:
    Mesh * mesh;

public:

    /// Initialization before unit tests
    void setUp();

    /// Tidying up after unit tests
    void tearDown();

    /** 
     * Run standard validity tests on bunny mesh
     * @pre bunny.stl must be located in the project root directory
     */
    void testBunny();

    /**
     * Run standard validity tests for simple 2-manifold tetrahedron
     */
    void testSimple();

    /**
     * Run standard validity tests for tetrahedron with basic flaws
     */
    void testBreak();

    /**
     * Run standard validity tests for tetrahedron with boundary
     */
    void testOpen();

    /**
     * Run standard validity tests for two tetrahedrons joined at a single vertex
     */
    void testPinch();

    /**
     * Run standard validity tests for a tetrahedron with a double coincident shell
     */
    void testOverlap();
};

#endif /* !TILER_TEST_MESH_H */
