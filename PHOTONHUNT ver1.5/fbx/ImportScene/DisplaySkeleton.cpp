/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

#include <fbxsdk.h>

#include "DisplayCommon.h"
#include "DisplaySkeleton.h"
#include "DisplayMesh.h"




void DisplaySkeleton(FbxNode* pNode)
{
    // 从这个 Node 上取出“骨骼属性”
    // 注意：只有当这个 Node 是骨骼节点时，这里才是有效的
    FbxSkeleton* lSkeleton = (FbxSkeleton*)pNode->GetNodeAttribute();

    // 输出骨骼节点的名字
    // 一般这个名字就是你在建模软件里看到的骨骼名
    DisplayString("Skeleton Name: ", (char*)pNode->GetName());

    // 输出这个骨骼节点的元数据连接信息
    // （调试用，对理解骨骼结构不是必须）
    DisplayMetaDataConnections(lSkeleton);

    // 骨骼类型的文字说明
    // FBX 把骨骼分成几种“角色”
    const char* lSkeletonTypes[] = {
        "Root",        // 根骨骼（整条骨架的起点）
        "Limb",        // 普通骨骼（比如大腿、手臂）
        "Limb Node",  // 中间节点（辅助用）
        "Effector"    // 末端节点（比如手指尖、脚尖）
    };

    // 输出当前骨骼属于哪一种类型
    DisplayString("    Type: ", lSkeletonTypes[lSkeleton->GetSkeletonType()]);

    // 如果是普通骨骼（Limb）
    // LimbLength 通常表示这根骨骼“理论上的长度”
    if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimb)
    {
        DisplayDouble("    Limb Length: ", lSkeleton->LimbLength.Get());
    }
    // 如果是中间节点（LimbNode）
    // Size 通常只是显示或辅助用的尺寸
    else if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode)
    {
        DisplayDouble("    Limb Node Size: ", lSkeleton->Size.Get());
    }
    // 如果是根骨骼（Root）
    // Size 同样是显示/编辑用的大小
    else if (lSkeleton->GetSkeletonType() == FbxSkeleton::eRoot)
    {
        DisplayDouble("    Limb Root Size: ", lSkeleton->Size.Get());
    }

    // 输出这根骨骼在建模软件里的显示颜色
    // 只用于可视化，不参与动画或权重计算
    DisplayColor("    Color: ", lSkeleton->GetLimbNodeColor());
}



std::vector<BoneInfo> bones; // 顺序 = boneIndex
ID3D11Buffer* skinCBuf = nullptr;
void CollectSkeleton(FbxNode* pNode)
{
    if (!pNode) return;

    FbxNodeAttribute* attr = pNode->GetNodeAttribute();
    if (attr && attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        BoneInfo info{};
        info.node = pNode;

        // 现在先用单位矩阵占位（后面再真正算 invBindPose）
        DirectX::XMStoreFloat4x4(&info.invBindPose, DirectX::XMMatrixIdentity());
        DirectX::XMStoreFloat4x4(&info.skinMatrix, DirectX::XMMatrixIdentity());

        bones.push_back(info);
    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
        CollectSkeleton(pNode->GetChild(i));
}


void CreateSkinningCB()
{
    D3D11_BUFFER_DESC desc{};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = sizeof(SkinningCB);
    desc.Usage = D3D11_USAGE_DEFAULT;

    Win->Gfx().pDevice->CreateBuffer(&desc, nullptr, &skinCBuf);

  

    //bones = std::move(sortedBones);

    //if (bones.size() > 65)
    //    bones.resize(65);

}


std::vector<BoneInfo>& Getbones()
{
    return bones;
}
ID3D11Buffer* GetskinCBuf()
{
    return skinCBuf;
}


void SampleSkeletonAtTime(FbxTime t)
{
    const size_t MAX_BONES = 540;
    size_t count = bones.size() < MAX_BONES ? bones.size() : MAX_BONES;

    // 0 帧（基准姿态）
    static bool baseCached = false;
    static std::vector<DirectX::XMMATRIX> baseGlobal;

    if (!baseCached)
    {
        baseGlobal.resize(count);

        FbxTime t0;
        t0.SetFrame(0, FbxTime::eFrames30);

        for (size_t i = 0; i < count; ++i)
        {
            if (!bones[i].node) continue;

            FbxAMatrix m0 = bones[i].node->EvaluateGlobalTransform(t0);

            baseGlobal[i] = DirectX::XMMatrixSet(
                (float)m0[0][0], (float)m0[0][1], (float)m0[0][2], (float)m0[0][3],
                (float)m0[1][0], (float)m0[1][1], (float)m0[1][2], (float)m0[1][3],
                (float)m0[2][0], (float)m0[2][1], (float)m0[2][2], (float)m0[2][3],
                (float)m0[3][0], (float)m0[3][1], (float)m0[3][2], (float)m0[3][3]
            );
        }

        baseCached = true;
    }

    // 当前帧
    for (size_t i = 0; i < count; ++i)
    {
        if (!bones[i].node)
            continue;

        FbxAMatrix mt = bones[i].node->EvaluateGlobalTransform(t);

        DirectX::XMMATRIX G = DirectX::XMMatrixSet(
            (float)mt[0][0], (float)mt[0][1], (float)mt[0][2], (float)mt[0][3],
            (float)mt[1][0], (float)mt[1][1], (float)mt[1][2], (float)mt[1][3],
            (float)mt[2][0], (float)mt[2][1], (float)mt[2][2], (float)mt[2][3],
            (float)mt[3][0], (float)mt[3][1], (float)mt[3][2], (float)mt[3][3]
        );

        DirectX::XMMATRIX FlipZ =
            DirectX::XMMatrixScaling(1.0f, 1.0f, -1.0f);

        // 相对 0 帧的变化（关键）
        //DirectX::XMMATRIX Skin =
        //    G * DirectX::XMMatrixInverse(nullptr, baseGlobal[i]);

        DirectX::XMMATRIX Skin =
            FlipZ *
            G *
            DirectX::XMMatrixInverse(nullptr, baseGlobal[i]) *
            FlipZ;

        // ❗ 写到 skinMatrix，不要再写 bindPose
        DirectX::XMStoreFloat4x4(&bones[i].skinMatrix, Skin);
    }
}


void UploadSkinningCB()
{
    SkinningCB skinCB{};
    skinCB.HasSkin = bones.empty() ? 0 : 1;

    const size_t MAX_BONES = 540;
    size_t count = bones.size() < MAX_BONES ? bones.size() : MAX_BONES;

    for (size_t i = 0; i < count; ++i)
        skinCB.Bones[i] = bones[i].skinMatrix;

    Win->Gfx().pContext->UpdateSubresource(
        skinCBuf, 0, nullptr, &skinCB, 0, 0
    );

    Win->Gfx().pContext->VSSetConstantBuffers(4, 1, &skinCBuf);
}
