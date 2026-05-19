/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_ANIMATION_H
#define _DISPLAY_ANIMATION_H

#include "DisplayCommon.h"
#include "../../system.h"
struct KeyframeVec3
{
    double time;
    DirectX::XMFLOAT3 value;
};

struct KeyframeQuat
{
    double time;
    DirectX::XMFLOAT4 value; // quaternion
};

struct BoneAnimation
{
    std::vector<KeyframeVec3> translations;
    std::vector<KeyframeQuat> rotations;
    std::vector<KeyframeVec3> scalings;
};


struct AnimationClip
{
    std::string name;   // 动画名（AnimStack 名）
    double startTime;   // 起始时间（秒）
    double endTime;     // 结束时间（秒）

    // 每根骨骼的动画
    std::unordered_map<std::string, BoneAnimation> boneAnims;
};



void DisplayAnimation(FbxScene* pScene);
void DisplayDefaultAnimation(FbxNode* pNode);

#endif // #ifndef _DISPLAY_ANIMATION_H


