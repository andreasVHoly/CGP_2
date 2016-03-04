//
// Voxels
//

#include "voxels.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;

VoxelVolume::VoxelVolume()
{
    xdim = ydim = zdim = 0;
    voxgrid = NULL;
    setFrame(cgp::Point(0.0f, 0.0f, 0.0f), cgp::Vector(0.0f, 0.0f, 0.0f));
}

VoxelVolume::VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag)
{
    voxgrid = NULL;
    setDim(xsize, ysize, zsize);
    setFrame(corner, diag);
}

VoxelVolume::~VoxelVolume()
{
    clear();
}

void VoxelVolume::clear()
{
    if(voxgrid != NULL)
    {
        delete [] voxgrid;
        voxgrid = NULL;
    }
}

//3d to 1d alg
//Flat[x + WIDTH * (y + DEPTH * z)] = Original[x, y, z]
//*(voxgrid + x + xdim + ydim * (y + zdim * z)) -> should point into right position

void VoxelVolume::fill(bool setval)
{

    //we need to build up the array for x values
    //size of the array is the product of the 3 dimesnions
    //TODO
    /*int size = xdim*ydim*zdim;
    for (int x = 0; x < size; x++){
        *(voxgrid + x) = setval;
    }*/



    for (int x = 0; x < xdim; x++){
        for (int y = 0; y < ydim; y++){
            for (int z = 0; z < zdim; z++){
                int val = (int)ceil((float)(x + xdim + ydim * (y + zdim * z))/((float)(sizeof(int)*8)));
                int index = ( (x + xdim + ydim * (y + zdim * z))) % ((sizeof(int)*8) );
                *(voxgrid + val) |= 1 << index;
            }
        }
    }


}

void VoxelVolume::calcCellDiag()
{
    if(xdim > 0 && ydim > 0 && zdim > 0)
        cell = cgp::Vector(diagonal.i / (float) xdim, diagonal.j / (float) ydim, diagonal.k / (float) zdim);
    else
        cell = cgp::Vector(0.0f, 0.0f, 0.0f);
}

void VoxelVolume::getDim(int &dimx, int &dimy, int &dimz)
{
    dimx = xdim; dimy = ydim; dimz = zdim;
}

void VoxelVolume::setDim(int &dimx, int &dimy, int &dimz)
{
    //assign dimensions
    xdim = dimx;
    ydim = dimy;
    zdim = dimz;
    //allocate memory for the pointer structure
    //voxgrid = new int[xdim*ydim*zdim];
    //TODO changed
    voxgrid = new int[(int)ceil((float)xdim*ydim*zdim/((float)(sizeof(int)*8)))];

    //fill up with false
    std::cout << dimx << "," << dimy << "," << dimz << std::endl;
    //we need to build up the array for x values
    //used fill to do this as the method is the same
    fill(1);

    calcCellDiag();
}

void VoxelVolume::getFrame(cgp::Point &corner, cgp::Vector &diag)
{
    corner = origin;
    diag = diagonal;
}

void VoxelVolume::setFrame(cgp::Point corner, cgp::Vector diag)
{
    origin = corner;
    diagonal = diag;
    calcCellDiag();
}

bool VoxelVolume::set(int x, int y, int z, bool setval){


    //check dimensions
    if (x < 0 || x >= xdim){
        cout << "voxel x index out of range in set()" << endl;
        return false;
    }
    if (y < 0 || y >= ydim){
        cout << "voxel y index out of range in set()" << endl;
        return false;
    }
    if (z < 0 || z >= zdim){
        cout << "voxel z index out of range in set()" << endl;
        return false;
    }

    //calculate the index into the 1D array
    //int index = (x + xdim + ydim * (y + zdim * z));
    //TODO changed
    int val = (int)ceil((float)(x + xdim + ydim * (y + zdim * z))/((float)(sizeof(int)*8)));
    int index = ((x + xdim + ydim * (y + zdim * z))) % ((sizeof(int)*8) ) ;

    //set to specified value
    if (setval){
        *(voxgrid + val) |= 1 << index;
    }
    else{
        *(voxgrid + val) &= ~(1 << index);
    }


    return true;
}

bool VoxelVolume::get(int x, int y, int z)
{   //get individual values
    //TODO
    //int index = *(voxgrid + x + xdim + ydim * (y + zdim * z));
    int val = (int)ceil((float)(x + xdim + ydim * (y + zdim * z))/((float)(sizeof(int)*8)));
    int index = ((x + xdim + ydim * (y + zdim * z))) % ((sizeof(int)*8) );

    //set to specified value


    if (((*(voxgrid + val) >> index) & 1)){
        return true;
    }
    else{
        return false;
    }



}

cgp::Point VoxelVolume::getVoxelPos(int x, int y, int z)
{
    cgp::Point pnt;
    cgp::Vector halfcell;
    float px, py, pz;

    px = (float) x / (float) xdim;
    py = (float) y / (float) ydim;
    pz = (float) z / (float) zdim;

    pnt = cgp::Point(origin.x + px * diagonal.i + 0.5f * cell.i, origin.y + py * diagonal.j + 0.5f * cell.j, origin.z + pz * diagonal.k + 0.5f * cell.k); // convert from voxel space to world coordinates
    return pnt;
}
