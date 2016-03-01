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

void VoxelVolume::fill(bool setval)
{

    //we need to build up the array for x values
    int size = xdim+ydim+zdim;
    for (int x = 0; x < size; x++){
        *(voxgrid + x) = setval;
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
    //fill up with false

    //we need to build up the array for x values
    for (int x = 0; x < dimx; x++){
        *(voxgrid + x) = 1;
    }
    //we need to build up the array for y values
    for (int y = 0; y < dimy; y++){
        *(voxgrid + dimx + y) = 1;
    }

    //we need to build up the array for z values
    for (int z = 0; z < dimz; z++){
        *(voxgrid + dimx  + dimy + z) = 1;
    }

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
    //check x bounds
    if (x < 0 || x > xdim){
        return false;
    }
    //check y bounds
    if (y < xdim || y > (ydim+xdim)){
        return false;
    }
    //check z bounds
    if (z < ydim || z > (ydim+xdim+zdim)){
        return false;
    }

    *(voxgrid + x) = setval;
    *(voxgrid+ xdim + y) = setval;
    *(voxgrid + xdim + ydim + z) = setval;
    return true;
}

bool VoxelVolume::get(int x, int y, int z)
{

    return true;
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
