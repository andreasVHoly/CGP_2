#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_mesh.h"
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

void TestMesh::setUp()
{
    mesh = new Mesh();
}

void TestMesh::tearDown()
{
    delete mesh;
}


//_________//
//NEW TESTS//
//---------//
void TestMesh::testVoxel(){
    cout << "...TESTING VOXEL METHODS..." << endl<<endl;
    VoxelVolume vox;
    CPPUNIT_ASSERT(vox.testSetGetDim());
    VoxelVolume vox2;
    CPPUNIT_ASSERT(vox2.testSetGet());
    VoxelVolume vox3;
    CPPUNIT_ASSERT(vox3.testFill());
    cout << "...ALL VOXEL TESTS PASSED..." << endl<<endl;
}


void TestMesh::testCSG(){
    cout << "...TESTING CSG TREE METHODS..." << endl<<endl;
    //__________________________________________
    //TESTING TREE TRAVERSAL TO FIND ALL SHAPE LEAVES
    //sample scence with 3 leaves
    Scene scene;
    scene.sampleScene();
    CPPUNIT_ASSERT(scene.testTreeTraversal(3));

    //made a scene with 5 leaf nodes
    //check to see if traversal finds all 5
    Scene scene2;
    scene2.testScene();
    CPPUNIT_ASSERT(scene2.testTreeTraversal(5));
    //__________________________________________



    //__________________________________________
    //difference
    Scene scene3;
    //CPPUNIT_ASSERT(scene3.finalTest(0));

    //intersection
    Scene scene4;
    //CPPUNIT_ASSERT(scene4.finalTest(1));

    //union
    Scene scene5;
    //CPPUNIT_ASSERT(scene5.finalTest(2));
    //__________________________________________
    cout << "...ALL CSG TREE TESTS PASSED..." << endl<<endl;


    cout << "...TESTING POINT CONTAINMENT METHOD..." <<endl<<endl;

    //__________________________________________
    Scene scene6;
    CPPUNIT_ASSERT(scene6.testPointContainment());
    //__________________________________________
    cout << "...ALL POINT CONTAINMENT TESTS PASSED..." << endl<<endl;
}




//_________//
//NEW TESTS//
//---------//

void TestMesh::testBunny()
{

    mesh->readSTL("../meshes/bunny.stl");
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity()); // bunny has known holes in the bottom
    cerr << "BUNNY TEST PASSED" << endl << endl;

    mesh->readSTL("../meshes/dragon.stl");
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "DRAGON TEST PASSED" << endl << endl;
}



void TestMesh::testSimple()
{
    // test simple valid 2-manifold
    mesh->validTetTest();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(mesh->manifoldValidity());
    cerr << "SIMPLE VALIDITY TEST PASSED" << endl << endl;
}

void TestMesh::testBreak()
{
    // test for duplicate vertices, dangling vertices and out of bounds on vertex indices
    mesh->basicBreakTest();
    CPPUNIT_ASSERT(!mesh->basicValidity());
    cerr << "BASIC INVALID MESH TEST PASSED" << endl << endl;
}

void TestMesh::testOpen()
{
    // test for 2-manifold with boundary
    mesh->openTetTest();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID MESH WITH BOUNDARY TEST PASSED" << endl << endl;
}

void TestMesh::testPinch()
{
    // test for non 2-manifold failure where surfaces touch at a single vertex
    mesh->touchTetsTest();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID PINCHED SURFACE TEST PASSED" << endl << endl;
}

void TestMesh::testOverlap()
{
    // test for non 2-manifold overlapping triangles
    mesh->overlapTetTest();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID NON-2-MANIFOLD TEST PASSED" << endl << endl;
}

//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMesh, TestSet::perBuild());
//#endif
