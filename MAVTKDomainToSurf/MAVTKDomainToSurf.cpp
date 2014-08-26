#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _MAVTKDomainToSurf_cpp[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         MAVTKDomainToSurf.cpp
* \author       Bill Hill
* \date         October 2012
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Computes an stl boundary for the given Woolz domain.
* \ingroup	MAVTKApps
*/

#include <stdio.h>
#include <unistd.h>
#include <Wlz.h>
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkContourFilter.h>
#include <vtkMarchingCubes.h>
#include <vtkQuadricDecimation.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkSTLWriter.h>

int             main(int argc, char **argv)
{
  int		option,
  		ok = 1,
		usage = 0,
		regGrow = 0,
		scVoxSz = 0;
  double	decimate = 0.0,
		erosion = 5.0,
		smooth = 0.0;
  WlzObject	*inObj = NULL,
  		*sdObj = NULL;
  WlzIBox3	objBB;
  WlzIVertex3	imgSz,
  		objSz;
  char		*inFileStr,
  		*outFileStr;
  WlzErrorNum   errNum = WLZ_ERR_NONE;
  const int	domVal = 128,
		imgPad = 8;
  const char    *errMsg;
  static char   optList[] = "hrzd:o:s:",
  		fileStrDef[] = "-";
  
  opterr = 0;
  inFileStr = outFileStr = fileStrDef;
  while((usage == 0) && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'd':
        if(sscanf(optarg, "%lg", &decimate) != 1)
	{
	  usage = 1;
	}
	break;
      case 'o':
        outFileStr = optarg;
	break;
      case 'r':
        regGrow = 1;
	break;
      case 's':
        if((sscanf(optarg, "%lg", &smooth) != 1) ||
           (smooth < 0.0) || (smooth > 1.0))
	{
	  usage = 1;
	}
	break;
      case 'z':
        scVoxSz = 1;
	break;
      case 'h': /* FALLTHROUGH */
      default:
        usage = 1;
	break;
    }
  }
  if((usage == 0) && (optind < argc))
  {
    if((optind + 1) != argc)
    {
      usage = 1;
    }
    else
    {
      inFileStr = argv[optind];
    }
  }
  ok = (usage == 0);
  if(ok)
  {
    FILE	*fP = NULL;

    errNum = WLZ_ERR_READ_EOF;
    if((fP = (strcmp(inFileStr, "-")?  fopen(inFileStr, "r"): stdin)) != NULL)
    {
      inObj = WlzAssignObject(WlzReadObj(fP, &errNum), NULL);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      if(inObj == NULL)
      {
        errNum = WLZ_ERR_OBJECT_NULL;
      }
      else if(inObj->type != WLZ_3D_DOMAINOBJ)
      {
        errNum = WLZ_ERR_OBJECT_TYPE;
      }
      else if(inObj->domain.core == NULL)
      {
        errNum = WLZ_ERR_DOMAIN_NULL;
      }
    }
    if(errNum == WLZ_ERR_NONE)
    {
      objBB = WlzBoundingBox3I(inObj, &errNum);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      objSz.vtX = objBB.xMax - objBB.xMin + 1;
      objSz.vtY = objBB.yMax - objBB.yMin + 1;
      objSz.vtZ = objBB.zMax - objBB.zMin + 1;
      if((objSz.vtX < 1) || (objSz.vtY < 1) || (objSz.vtZ < 1))
      {
        errNum = WLZ_ERR_DOUBLE_DATA;
      }
    }
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: Failed to read input object (%s).\n",
		     *argv, errMsg);
    }
    if(fP)
    {
      if(strcmp(inFileStr, "-"))
      {
        (void )fclose(fP);
      }
    }
  }
  if(ok && regGrow)
  {
    WlzObject	*stObj = NULL;

    stObj = WlzAssignObject(
    	    WlzMakeSphereObject(WLZ_3D_DOMAINOBJ, erosion,
	                        0.0, 0.0, 0.0, &errNum), NULL);
    if(errNum == WLZ_ERR_NONE)
    {
      sdObj = WlzStructErosion(inObj, stObj, &errNum);
    }
    (void )WlzFreeObj(stObj);
    if(errNum != WLZ_ERR_NONE)
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: Failed to make region growing seed object (%s).\n",
		     *argv, errMsg);
    }
  }
  if(ok)
  {
    vtkSmartPointer<vtkImageData> imgDat =
    		vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkSTLWriter> stlWriter =
    		vtkSmartPointer<vtkSTLWriter>::New();
    vtkSmartPointer<vtkPolyData> surf =
        	vtkSmartPointer<vtkPolyData>::New();

    imgSz.vtX = objSz.vtX + (2 * imgPad);
    imgSz.vtY = objSz.vtY + (2 * imgPad);
    imgSz.vtZ = objSz.vtZ + (2 * imgPad);
    imgDat->SetOrigin(objBB.xMin - imgPad, objBB.yMin - imgPad,
		      objBB.zMin - imgPad);
    imgDat->SetDimensions(imgSz.vtX, imgSz.vtY, imgSz.vtZ);
    imgDat->SetSpacing(1.0, 1.0, 1.0); 
    imgDat->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    for(int iZ = 0; iZ < imgSz.vtZ; ++iZ)
    {
      int oZ = iZ + objBB.zMin - imgPad;
      for(int iY = 0; iY < imgSz.vtY; ++iY)
      {
	int oY = iY + objBB.yMin - imgPad;
        for(int iX = 0; iX < imgSz.vtX; ++iX)
	{
          int oX = iX + objBB.xMin - imgPad;
	  WlzUByte *p = static_cast<WlzUByte*>(imgDat->GetScalarPointer(iX,
 				 iY, iZ));
	  p[0] = (WlzInsideDomain3D(inObj->domain.p, oZ, oY, oX, &errNum))?
		 0: domVal;
	}
      }
    }
    if(regGrow)
    {
      ok = 0;
      (void )fprintf(stderr,
                     "%s: Sorry region growing in unimplimented.\n",
		     *argv);
    }
    else
    {
      vtkSmartPointer<vtkMarchingCubes> mc =
      		vtkSmartPointer<vtkMarchingCubes>::New();
      mc->SetInputData(imgDat);
      mc->ComputeNormalsOff();
      mc->SetValue(0, domVal);
      mc->Update();
      surf->ShallowCopy(mc->GetOutput());
    }
    if(scVoxSz)
    {
      vtkSmartPointer<vtkTransformFilter> trModel =
          vtkSmartPointer<vtkTransformFilter>::New();
      vtkSmartPointer<vtkTransform> tr =
          vtkSmartPointer<vtkTransform>::New();
      tr->Scale(inObj->domain.p->voxel_size[0],
	        inObj->domain.p->voxel_size[1],
	        inObj->domain.p->voxel_size[2]);
      trModel->SetTransform(tr);
      trModel->SetInputData(surf);
      trModel->Update();
      surf->ShallowCopy(trModel->GetOutput());
    }
    if(decimate > 1.0)
    {
      double	in;
      		
      in = surf->GetNumberOfPolys();
      if(in > 0)
      {
	vtkSmartPointer<vtkQuadricDecimation> dec =
	    vtkSmartPointer<vtkQuadricDecimation>::New();
	dec->SetInputData(surf);
	dec->SetTargetReduction(1.0 - (decimate / in));
	dec->Update();
        surf->ShallowCopy(dec->GetOutput());
      }
    }
    if(smooth > 0.0)
    {
      vtkSmartPointer<vtkWindowedSincPolyDataFilter> smo =
		vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
      smo->SetInputData(surf);
      smo->SetNumberOfIterations(10);
      smo->SetPassBand(smooth);
      smo->NormalizeCoordinatesOn();
      smo->Update(); 
      surf->ShallowCopy(smo->GetOutput());
    }
    stlWriter->SetFileName(outFileStr);
    stlWriter->SetInputData(surf);
    stlWriter->Write();
  }
  (void )WlzFreeObj(inObj);
  (void )WlzFreeObj(sdObj);
  if(usage)
  {
    (void )fprintf(stderr,
    "Usage: %s%s%s%s%s%sExample: %s%s",
    *argv,
    " [-d <dec target>] [-h] [-o<output file>]\n"
    "\t\t[-s <smoothing>] [-z] [<input object>]\n"
    "Woolz Version: ",
    WlzVersion(),
    "\nVTK Version: ",
    vtkVersion::GetVTKVersion(),
    "\nOptions:\n"
    "  -r  Use region growing algorithm rather than marching cubes to\n"
    "      extract the surface.\n"
    "  -d  Decimate mesh using quadric edge collapse to given number\n"
    "      of surface elements.\n"
    "  -h  Help, prints usage message.\n"
    "  -o  Output file.\n"
    "  -s  Smooth surface using Taubin filter. The smoothing parameter\n"
    "      is the pass band value [0-1]. Useful range probably [0.01 - 0.1]\n"
    "      with higher values producing less smoothing.\n"
    "  -z  Scale using the Woolz voxel size.\n"
    "\nComputes a boundary surface from a Woolz domain using Woolz and WTK.\n",
    *argv,
    " -o surf.stl dom.wlz\n"
    "Reads a Woolz domain from the file dom.wlz, computes a boundary surface\n"
    "and then writes it to the file surf.stl.\n");
  }
  exit(errNum);
}
