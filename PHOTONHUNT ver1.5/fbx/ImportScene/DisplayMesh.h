/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_MESH_H
#define _DISPLAY_MESH_H

#include "DisplayCommon.h"
#include "../../system.h"

void DisplayMesh(FbxNode* pNode);
FbxMesh* GetlMesh();
std::vector<Object_3D>& GetFBXOBJ(void);
#endif // #ifndef _DISPLAY_MESH_H


