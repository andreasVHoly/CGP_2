#ifndef _CSG
#define _CSG
/**
 * @file
 *
 * Constructive Solid Geometry binary tree, with internal nodes representing binary set operations and leaf nodes as shape primitives.
 */

#include <vector>
#include <stdio.h>
#include <iostream>
#include "mesh.h"
#include "voxels.h"

/**
 * Different types of binary set operations on shapes
 */
enum class SetOp
{
    UNION,        ///< combine two shapes together
    INTERSECTION, ///< create a shape in the region where the arguments overlap
    DIFFERENCE,   ///< subtract second shape from the first
};


class SceneNode
{
public:
    // must have at least one virtual member to allow dynamic casting
    virtual ~SceneNode(){};
};

class OpNode: public SceneNode
{
public:
    SceneNode * left, * right;
    SetOp op;

    ~OpNode(){ delete left; delete right; /*cout << "deleted left and right " << endl;*/}
};

class ShapeNode: public SceneNode
{
public:
    BaseShape * shape;

    ~ShapeNode(){
        /*cout << "deleted shape in shape node" << endl;*/
        delete shape; }
};

/**
 * CSG Tree that can be evaluated to produce a volumetric representation.
 */
class Scene
{
private:
    ///< CSG tree root
    SceneNode * csgroot;            ///< root node of the csg tree
    GLfloat * col;                  ///< (r,g,b,a) colour
    cgp::Vector voldiag;                 ///< diagonal of scene bounding box in cm
    VoxelVolume vox;                ///< voxel representation of scene
    float voxsidelen;               ///< side length of a single voxel
    bool voxactive;                 ///< voxel representation has been created
    Mesh voxmesh;                   ///< isosurface of voxel volume
    std::vector<ShapeNode *> leaves;///< the leaves vector


    //added
    void deleteTree();
    void inOrderWalk(SceneNode * node);
    void applyUnion();
    void applyDifference();
    void applyIntersection();
    bool differencePoints();
    bool intersectionPoints();
    bool unionPoints();





    //added

    /**
     * Generate triangle mesh geometry for OpenGL rendering of all leaf nodes.
     * Does not capture set operations at all (except where all set operations are a union)
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genVizRender(View * view, ShapeDrawData &sdd);

    /**
     * Generate triangle mesh geometry for OpenGL rendering of voxel structure.
     * Approximates voxel grid as a set of spheres. Extremely expensive to render but more accurate in
     * capturing set operations
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool genVoxRender(View * view, ShapeDrawData &sdd);

    /**
     * Apply a boolean set operator given two volumetric operands.
     * @param op            boolean set operation being applied (union, intersection or difference). Applied as leftarg = leftarg op rightarg
     * @param[out] leftarg  first voxel grid argument. The result is overwritten to here for space reasons.
     * @param rightarg      second voxel grid argument.
     */
    void voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg);

    /**
     * Convert a CSG tree into a VoxelVolume by evaluating it with a recursive depth-first walk
     * @param root          root node of the CSG tree
     * @param[out] voxels   volumetric representation of the CSG tree
     */
    void voxWalk(SceneNode *root, VoxelVolume *voxels);

public:



    bool finalTest(int test);
    void doubleSphereUnion();
    void doubleSphereInts();
    void doubleSphereDiff();

    ShapeGeometry geom;         ///< triangle mesh geometry for scene

    /// Default constructor
    Scene();

    /// Destructor
    ~Scene();

    /**
     * Reset CSG tree
     */
    void clear();

    /// getter for whether the scene has been voxelised
    bool voxFin(){ return voxactive; }

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool bindGeometry(View * view, ShapeDrawData &sdd);

    /**
     * convert csg tree into a voxel representation
     * @param voxlen    side length of an individual voxel
     */
    void voxelise(float voxlen);

    /**
     * create a sample csg tree to test different shapes and operators
     */
    void sampleScene();

    /**
     * create a sample csg tree to test different shapes and operators. Expensive because it uses mesh point containment with the Bunny.
     */
    void expensiveScene();

    void testScene();
    void pointScene();
    //added for tests
    //testing
    bool testTreeTraversal(int size);
    bool testPointContainment();

};

#endif
