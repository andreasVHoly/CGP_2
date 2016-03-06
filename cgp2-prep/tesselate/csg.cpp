//
// csg
//

#include "csg.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>
#include <stack>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;
// using namespace cgp;

GLfloat defaultCol[] = {0.243f, 0.176f, 0.75f, 1.0f};

bool Scene::genVizRender(View * view, ShapeDrawData &sdd)
{

    int i;

    geom.clear();
    geom.setColour(defaultCol);

    //traverse csg tree to find leaf nodes & populate the leaf node vector
    inOrderWalk(csgroot);

    // traverse leaf shapes generating geometry
    for(i = 0; i < (int) leaves.size(); i++)
    {
        leaves[i]->shape->genGeometry(&geom, view);
    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}


//recursive method for finding leaf nodes
void Scene::inOrderWalk(SceneNode* node){

    //check if we have a shape node, cast will fail if not
    if (ShapeNode * sn = dynamic_cast<ShapeNode*>(node)){
        //std::cout << "Found leaf node with " << std::endl;
        leaves.push_back(sn);
    }
    else{
        //we have an opnode and we recurse further
        inOrderWalk(dynamic_cast<OpNode*>(node)->left);
        inOrderWalk(dynamic_cast<OpNode*>(node)->right);
    }
}


bool Scene::genVoxRender(View * view, ShapeDrawData &sdd)
{
    int x, y, z, xdim, ydim, zdim;
    glm::mat4 tfm, idt;
    glm::vec3 trs;
    cgp::Point pnt;

    geom.clear();
    geom.setColour(defaultCol);

    if(voxactive)
    {
        idt = glm::mat4(1.0f); // identity matrix

        vox.getDim(xdim, ydim, zdim);

        // place a sphere at filled voxels but subsample to avoid generating too many spheres
        for(x = 0; x < xdim; x+=10)
            for(y = 0; y < ydim; y+=10)
                for(z = 0; z < zdim; z+=10)
                {
                    if(vox.get(x, y, z))
                    {
                        pnt = vox.getVoxelPos(x, y, z); // convert from voxel space to world coordinates
                        trs = glm::vec3(pnt.x, pnt.y, pnt.z);
                        tfm = glm::translate(idt, trs);
                        geom.genSphere(voxsidelen * 5.0f, 3, 3, tfm);
                    }
                }

    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}

Scene::Scene()
{
    csgroot = NULL;
    col = defaultCol;
    voldiag = cgp::Vector(20.0f, 20.0f, 20.0f);
    voxsidelen = 0.0f;
    voxactive = false;
}

Scene::~Scene()
{
    clear();
}

void Scene::clear(){
    geom.clear();
    vox.clear();
    deleteTree();
}

//cleans up the tree
void Scene::deleteTree(){
    //we need to cast as we want to get access to left and right
    //cout << "called delete tree " << endl;
    //we check if the root can be cast to a OpNode -> it should allow this
    //delete the child if we find it
    //to see that this works, enable the print statements and see that each of the child destructors get called down the tree
    if (OpNode * rn = dynamic_cast<OpNode*>(csgroot)){
        //cout << "called right " << endl;
        delete rn->right;
    }
    //we check for the left child and delete if found
    if (OpNode * ln = dynamic_cast<OpNode*>(csgroot)){
        //cout << "called left " << endl;
        delete ln->left;
    }
}

bool Scene::bindGeometry(View * view, ShapeDrawData &sdd)
{
    if(voxactive)
    {
        return genVoxRender(view, sdd);
    }
    else
        return genVizRender(view, sdd);
}

void Scene::voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg)
{

    //get the dimensions
    int dimx, dimy, dimz;
    leftarg->getDim(dimx,dimy,dimz);
    //cout << dimx << " " << dimy << " " << dimz << endl;

    //we switch on the operation
    switch(op){
        //union operation
        case SetOp::UNION:
            //loop through the voxel and apply union operator
            for (int x = 0; x < dimx; x++){
                for (int y = 0; y < dimy; y++){
                    for (int z = 0; z < dimz; z++){
                        leftarg->set(x,y,z,leftarg->get(x,y,z) | rightarg->get(x,y,z));
                    }
                }
            }
            break;
        //intersection operation
        case SetOp::INTERSECTION:
            //loop through the voxel and apply intersection operator
            for (int x = 0; x < dimx; x++){
                for (int y = 0; y < dimy; y++){
                    for (int z = 0; z < dimz; z++){
                        leftarg->set(x,y,z,leftarg->get(x,y,z) & rightarg->get(x,y,z));
                    }
                }
            }
            break;
        //diff operation
        case SetOp::DIFFERENCE:
            //loop through the voxel and apply difference operator in 2 steps, a AND applied to the result of a XOR
            for (int x = 0; x < dimx; x++){
                for (int y = 0; y < dimy; y++){
                    for (int z = 0; z < dimz; z++){
                        leftarg->set(x,y,z, leftarg->get(x,y,z) & (leftarg->get(x,y,z) ^ rightarg->get(x,y,z)));
                    }
                }
            }
            break;
    }
}



void Scene::voxWalk(SceneNode *root, VoxelVolume *voxels)
{
    //if we have a shape node we populate the voxel
    if (ShapeNode * leaf = dynamic_cast<ShapeNode*>(root)){
        //get dimensions
        int dimx,dimy,dimz;
        voxels->getDim(dimx,dimy,dimz);

        //loop through the voxel
        for (int x = 0; x < dimx; x++){
            for (int y = 0; y < dimy; y++){
                for (int z = 0; z < dimz; z++){
                    //we get the voxel to world conversion
                    cgp::Point point = voxels->getVoxelPos(x,y,z);
                    //we check if the world coord is inside the model
                    bool result = leaf->shape->pointContainment(point);
                    //we change the result in the voxel space
                    voxels->set(x,y,z,result);
                }
            }
        }
    }
    else if (OpNode * op = dynamic_cast<OpNode*>(root)){
        //recurse down the left
        voxWalk(op->left,voxels);
        //VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag); constructor

        //getting all the info to create a new voxel volume
        cgp::Point corner;
        cgp::Vector diag;
        voxels->getFrame(corner,diag);

        int dimx,dimy,dimz;
        voxels->getDim(dimx,dimy,dimz);

        //create new voxel for the right hand side with same dimensions as previous ones
        VoxelVolume * vox2 = new VoxelVolume(dimx,dimy,dimz,corner,diag);
        //recurse down the right
        voxWalk(op->right,vox2);


        //void voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg);

        //apply operation on the 2 voxels
        voxSetOp(op->op, voxels, vox2);
        //cout << "deleting temp voxvolume" << endl;
        //this crashes when using bit packing?
        //delete vox2;
        vox2 = NULL;
    }


}

void Scene::voxelise(float voxlen)
{
    int xdim, ydim, zdim;

    // calculate voxel volume dimensions based on voxlen
    xdim = ceil(voldiag.i / voxlen)+2; // needs a 1 voxel border to ensure a closed mesh if shapes reach write up to the border
    ydim = ceil(voldiag.j / voxlen)+2;
    zdim = ceil(voldiag.k / voxlen)+2;
    voxsidelen = voxlen;
    voxactive = true;

    cgp::Vector voxdiag = cgp::Vector((float) xdim * voxlen, (float) ydim * voxlen, (float) zdim * voxlen);
    cgp::Point voxorigin = cgp::Point(-0.5f*voxdiag.i, -0.5f*voxdiag.j, -0.5f*voxdiag.k);
    vox.setDim(xdim, ydim, zdim);
    vox.setFrame(voxorigin, voxdiag);

    cerr << "Voxel volume dimensions = " << xdim << " x " << ydim << " x " << zdim << endl;

    // actual recursive depth-first walk of csg tree
    if(csgroot != NULL)
        voxWalk(csgroot, &vox);
}

void Scene::sampleScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 4.0f);

    ShapeNode * cyl1 = new ShapeNode();
    cyl1->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * cyl2 = new ShapeNode();
    cyl2->shape = new Cylinder(cgp::Point(0.0f, -7.0f, 0.0f), cgp::Point(0.0f, 7.0f, 0.0f), 2.5f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = sph;
    combine->right = cyl1;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = cyl2;

    csgroot = diff;
}
//used to test the number of leaves
void Scene::testScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 4.0f);

    ShapeNode * cyl1 = new ShapeNode();
    cyl1->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * cyl2 = new ShapeNode();
    cyl2->shape = new Cylinder(cgp::Point(0.0f, -7.0f, 0.0f), cgp::Point(0.0f, 7.0f, 0.0f), 2.5f);

    ShapeNode * cyl3 = new ShapeNode();
    cyl3->shape = new Cylinder(cgp::Point(0.0f, -7.0f, -7.0f), cgp::Point(0.0f, 7.0f, 7.0f), 3.0f);


    ShapeNode * sph2 = new ShapeNode();
    sph2->shape = new Sphere(cgp::Point(1.0f, 1.0f, -1.0f), 3.0f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = cyl1;
    combine->right = cyl2;

    OpNode * ints = new OpNode();
    ints->op = SetOp::INTERSECTION;
    ints->left = sph2;
    ints->right = cyl3;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = sph;

    OpNode * top = new OpNode();
    top->op = SetOp::UNION;
    top->left = diff;
    top->right = ints;

    csgroot = top;
}

void Scene::expensiveScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(1.0f, -2.0f, -2.0f), 3.0f);

    ShapeNode * cyl = new ShapeNode();
    cyl->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * mesh = new ShapeNode();
    Mesh * bunny = new Mesh();
    bunny->readSTL("../meshes/bunny.stl");
    bunny->boxFit(10.0f);
    mesh->shape = bunny;

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = mesh;
    combine->right = cyl;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = mesh;

    csgroot = diff;
}


void Scene::pointScene(){
    ShapeNode * sph = new ShapeNode();
    //create basic sphere with r = 1
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 1.0f);
    csgroot = sph;
}


bool Scene::testTreeTraversal(int size){
    inOrderWalk(csgroot);
    if (leaves.size() != size){
        return false;
    }
    return true;

}

void Scene::doubleSphereUnion(){
    ShapeNode * sph1 = new ShapeNode();
    sph1->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 1.0f);

    ShapeNode * sph2 = new ShapeNode();
    sph2->shape = new Sphere(cgp::Point(1.0f, 0.0f, 0.0f), 1.0f);


    OpNode * node1 = new OpNode();
    node1->op = SetOp::UNION;
    node1->left = sph1;
    node1->right = sph2;

    csgroot = node1;
}

void Scene::doubleSphereInts(){

    ShapeNode * sph1 = new ShapeNode();
    sph1->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 1.0f);

    ShapeNode * sph2 = new ShapeNode();
    sph2->shape = new Sphere(cgp::Point(1.0f, 0.0f, 0.0f), 1.0f);


    OpNode * node1 = new OpNode();
    node1->op = SetOp::INTERSECTION;
    node1->left = sph1;
    node1->right = sph2;

    csgroot = node1;
}
void Scene::doubleSphereDiff(){

    ShapeNode * sph1 = new ShapeNode();
    sph1->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 1.0f);

    ShapeNode * sph2 = new ShapeNode();
    sph2->shape = new Sphere(cgp::Point(1.0f, 0.0f, 0.0f), 1.0f);


    OpNode * node1 = new OpNode();
    node1->op = SetOp::DIFFERENCE;
    node1->left = sph1;
    node1->right = sph2;

    csgroot = node1;
}



bool Scene::finalTest(int test){
    //when testing with the 2 spheres the key points will be used to test:
    //0,0,0 (origin & edges of sphere 2)

    switch (test){
        case 0:
            doubleSphereDiff();
            voxelise(0.05f);
            //if we apply the difference between the 2 spheres, the right side of sphere one wil be easten away by sphere 2
            return differencePoints();
        case 1:
            //if we apply the intersection\ between the 2 spheres, the part where they overalp remains (right side of sphere 1 and left side of sphere 2), the part that was eaten away in the test above
            doubleSphereInts();
            voxelise(0.05f);
            return intersectionPoints();
        case 2:
            //both spheres remain fully
            doubleSphereUnion();
            voxelise(0.05f);
            return unionPoints();
        default:
            return false;
    }
}


bool Scene::intersectionPoints(){
    //NOTE I tested the points on the spheres such as (0,0,0) and edge points that were converted using the below key
    //this relies on the fact that the dimesnions are 402x402x402
    //3d coordinate mappings to voxel space mapping:
        // -1 -> 191
        // 0 -> 201
        // 1 -> 220
        // 2 -> 240
    //if this operation worked, both origins should feature
    if (!vox.get(201,201,201) || !vox.get(220,201,201)){
        cout << "Origin was false" << endl;
        return false;
    }

    //top of the spheres
    if (vox.get(201,220,201) || vox.get(220,220,201)){
        cout << "Top was set to false" << endl;
        return false;
    }

    //left and right side of the spheres
    if (vox.get(191,201,201) || vox.get(240,201,201)){
        cout << "left|right side was set to true" << endl;
        return false;
    }


    //check extremes and half way poiints
    if (vox.get(220,191,201) || vox.get(220,220,201) ){
        cout << "extremes failed" << endl;
        return false;
    }

    //check if 3d points of sphere1 exist
    if (vox.get(201,201,191) || vox.get(201,201,220) ){
        cout << "spehre 1 3d points failed" << endl;
        return false;
    }

    //check if 3d points of sphere2 exist
    if (vox.get(220,201,191) || vox.get(220,201,220)){
        cout << "spehre 2 3d points failed" << endl;
        return false;
    }


    return true;
}


bool Scene::differencePoints(){
    //NOTE I tested the points on the spheres such as (0,0,0) and edge points that were converted using the below key
    //this relies on the fact that the dimesnions are 402x402x402
    //3d coordinate mappings to voxel space mapping:
        // -1 -> 191
        // 0 -> 201
        // 1 -> 220
        // 2 -> 240
    //if this operation worked, the origin should be false
    if (vox.get(201,201,201)){
        cout << "Origin was set to true" << endl;
        return false;
    }

    //top of the sphere should still be true
    if (!vox.get(201,220,201)){
        cout << "Top was set to false" << endl;
        return false;
    }

    //left and right side of the sphere should still be true
    if (!vox.get(191,201,201) || vox.get(220,201,201)){
        cout << "left|right side was set to false" << endl;
        return false;
    }


    //check the other circle is completely gone by checking origin and extremes
    if (vox.get(220,201,201) || vox.get(240,201,201) || vox.get(220,191,201) || vox.get(220,220,201)  || vox.get(220,220,191)  || vox.get(220,220,220) ){
        cout << "spehre 2 still has traces left" << endl;
        return false;
    }

    //check if 3d points of sphere 1 are still there
    if (!vox.get(201,201,191) || !vox.get(201,201,220) ){
        cout << "spehre 2 still has traces left" << endl;
        return false;
    }


    return true;
}

bool Scene::unionPoints(){
    //NOTE I tested the points on the spheres such as (0,0,0) and edge points that were converted using the below key
    //this relies on the fact that the dimesnions are 402x402x402
    //3d coordinate mappings to voxel space mapping:
        // -1 -> 191
        // 0 -> 201
        // 1 -> 220
        // 2 -> 240


    //if this operation worked, both circles shoudld fully feature
    if (!vox.get(201,201,201) || !vox.get(220,201,201)){
        cout << "Origin was set to false" << endl;
        return false;
    }

    //top of the spheres
    if (!vox.get(201,220,201) || !vox.get(220,220,201)){
        cout << "Top was set to false" << endl;
        return false;
    }

    //left and right side of the spheres should still be true
    if (!vox.get(191,201,201) || !vox.get(220,201,201) || !vox.get(201,201,201) || !vox.get(240,201,201)){
        cout << "left|right side was set to false" << endl;
        return false;
    }




    //check if 3d points of sphere1 exist
    if (!vox.get(201,201,191) || !vox.get(201,201,220)){
        cout << "spehre 1 3d points failed" << endl;
        return false;
    }

    //check if 3d points of sphere2 exist
    if (!vox.get(220,201,191) || !vox.get(220,201,220) ){
        cout << "spehre 2 3d points failed" << endl;
        return false;
    }


    return true;
}




bool Scene::testPointContainment(){
    pointScene();
    //convert node
    ShapeNode * leaf = dynamic_cast<ShapeNode*>(csgroot);
    //create positive test points
    //centre
    cgp::Point p1(0.0f,0.0f,0.0f);
    //all edge cases in axis
    cgp::Point p2(1.0f,0.0f,0.0f);
    cgp::Point p3(0.0f,1.0f,0.0f);
    cgp::Point p4(.0f,0.0f,1.0f);
    cgp::Point p5(-1.0f,0.0f,0.0f);
    cgp::Point p6(0.0f,-1.0f,0.0f);
    cgp::Point p7(0.0f,0.0f,-1.0f);
    cgp::Point p11(0.5f,0.5f,0.5f);
    cgp::Point p10(0.5f,0.5f,0.5f);

    //create negative test points
    cgp::Point p8(1.0f,1.0f,1.0f);
    cgp::Point p9(2.0f,2.0f,0.0f);


    if (!leaf->shape->pointContainment(p1)){
        cout << "Point 1 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p2)){
        cout << "Point 2 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p1)){
        cout << "Point 3 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p4)){
        cout << "Point 4 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p5)){
        cout << "Point 5 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p6)){
        cout << "Point 6 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p10)){
        cout << "Point 10 failed" << endl;
        return false;
    }
    if (!leaf->shape->pointContainment(p11)){
        cout << "Point 11 failed" << endl;
        return false;
    }
    if (leaf->shape->pointContainment(p8)){
        cout << "Point 8 failed" << endl;
        return false;
    }
    if (leaf->shape->pointContainment(p9)){
        cout << "Point 9 failed" << endl;
        return false;
    }


    return true;


}
