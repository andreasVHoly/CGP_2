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
        std::cout << "Found leaf node with " << std::endl;
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

    //we check if the root can be cast to a OpNode -> it should allow this
    //delete the child if we find it
    if (OpNode * rn = dynamic_cast<OpNode*>(csgroot)){
        delete rn->right;
    }
    //we check for the left child and delete if found
    if (OpNode * ln = dynamic_cast<OpNode*>(csgroot)){
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
    //TODO, needs completing

    int dimx, dimy, dimz;
    leftarg->getDim(dimx,dimy,dimz);
    cout << dimx << " " << dimy << " " << dimz << endl;
    //for xdim then for all

    switch(op){
        //union operation
        case SetOp::UNION:
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
            for (int x = 0; x < dimx; x++){
                for (int y = 0; y < dimy; y++){
                    for (int z = 0; z < dimz; z++){
                        leftarg->set(x,y,z, leftarg->get(x,y,z) & (leftarg->get(x,y,z) ^ rightarg->get(x,y,z)));
                        //leftarg->set(x,y,z, leftarg->get(x,y,z) ^ rightarg->get(x,y,z));

                        /*if (leftarg->get(x,y,z) == 1 && rightarg->get(x,y,z) == 1){
                            leftarg->set(x,y,z,0);
                        }
                        else if (leftarg->get(x,y,z) == 0 && rightarg->get(x,y,z) == 0){
                            leftarg->set(x,y,z,0);
                        }
                        else{

                        }*/
                    }
                }
            }
            break;
    }
}



void Scene::voxWalk(SceneNode *root, VoxelVolume *voxels)
{
    //TODO, needs completing
    // will require dynamic casting of SceneNode pointers with 0's -> false

    if (ShapeNode * leaf = dynamic_cast<ShapeNode*>(root)){


        int dimx,dimy,dimz;
        voxels->getDim(dimx,dimy,dimz);

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
        voxWalk(op->left,voxels);
        //VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag); constructor

        //getting all the info to create a new voxel volume
        cgp::Point corner;
        cgp::Vector diag;
        voxels->getFrame(corner,diag);

        int dimx,dimy,dimz;
        voxels->getDim(dimx,dimy,dimz);
        VoxelVolume * vox2 = new VoxelVolume(dimx,dimy,dimz,corner,diag);




        voxWalk(op->right,vox2);


        //void voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg);
        voxSetOp(op->op, voxels, vox2);

        delete vox2;
        //TODO need to delete the VoxelVolume
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
