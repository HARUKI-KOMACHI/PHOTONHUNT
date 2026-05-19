/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_SKELETON_H
#define _DISPLAY_SKELETON_H

#include "DisplayCommon.h"
#include "../../system.h"

void DisplaySkeleton(FbxNode* pNode);


void CollectSkeleton(FbxNode* pNode);
void CreateSkinningCB();

struct BoneInfo
{
    FbxNode* node;                      // 骨骼节点（FBX）
    DirectX::XMFLOAT4X4 invBindPose;    // 逆绑定姿态（初始化一次）
    DirectX::XMFLOAT4X4 skinMatrix;     // 当前帧的动画矩阵（每帧更新）
};


std::vector<BoneInfo>& Getbones();
ID3D11Buffer* GetskinCBuf();

void SampleSkeletonAtTime(FbxTime t);

void UploadSkinningCB();
#endif // #ifndef _DISPLAY_SKELETON_H


