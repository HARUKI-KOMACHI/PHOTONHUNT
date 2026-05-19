/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayLink.h"
#if defined (FBXSDK_ENV_MAC)
// disable the 吐ormat not a string literal and no format arguments・warning since
// the FBXSDK_printf calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void DisplayLink(FbxGeometry* pGeometry)
{
    // ===============================
    // 这一整个函数的目的：
    // 遍历这个 Mesh 上的「Skin（蒙皮）」，
    // 找到：哪根骨骼 → 影响了哪些 Control Point → 权重是多少
    //
    // 注意：
    // 这里“不是在处理顶点本身”，
    // 而是在处理【骨骼 ↔ ControlPoint】的关系
    // ===============================

    int i, j;

    // 一个 Mesh 可能有多个 Skin（一般只有 1 个）
    int lSkinCount = 0;

    // 一个 Skin 里有多少个 Cluster（≈ 骨骼）
    int lClusterCount = 0;

    FbxCluster* lCluster;

    // 获取这个几何体上 Skin 的数量
    lSkinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);

    // ===============================
    // 第一层：遍历每一个 Skin
    // ===============================
    for (i = 0; i != lSkinCount; ++i)
    {
        // 获取当前 Skin
        FbxSkin* lSkin = (FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin);

        // 一个 Skin 里有多少根骨骼（Cluster）
        lClusterCount = lSkin->GetClusterCount();

        // ===============================
        // 第二层：遍历 Skin 里的每一根骨骼
        // ===============================
        for (j = 0; j != lClusterCount; ++j)
        {
            // 当前处理的骨骼（Cluster）
            lCluster = lSkin->GetCluster(j);

            // 骨骼的混合模式（一般不用管，Normalize 最常见）
            const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };
            DisplayString("    Mode: ", lClusterModes[lCluster->GetLinkMode()]);

            // 这根骨骼对应的 Node（真正的骨骼对象）
            if (lCluster->GetLink() != NULL)
            {
                // 骨骼名字（你以后可以用它来建骨骼表）
                DisplayString("        Name: ", (char*)lCluster->GetLink()->GetName());
            }

            // ===============================
            // 重点数据开始
            // ===============================

            // 这根骨骼一共影响了多少个 Control Point
            int lIndexCount = lCluster->GetControlPointIndicesCount();

            // 被影响的 Control Point 索引数组
            int* lIndices = lCluster->GetControlPointIndices();

            // 对应的权重数组（和上面的索引一一对应）
            double* lWeights = lCluster->GetControlPointWeights();

            // 例子：
            // lIndices[k]  → ControlPointIndex
            // lWeights[k]  → 这个骨骼对该 ControlPoint 的影响权重
            //
            // 也就是：
            // ControlPoint[lIndices[k]] 受到 bone[j] 的影响，权重 = lWeights[k]

            for (int k = 0; k < lIndexCount; k++)
            {
                int controlPointIndex = lIndices[k];
                float weight = (float)lWeights[k];

                // 你将来真正要做的是：
                // cpWeights[controlPointIndex].push_back({ boneIndex, weight });
            }

            // ===============================
            // 下面这些是矩阵信息（绑定姿态用）
            // 现在你可以先不动
            // ===============================

            FbxAMatrix lMatrix;

            // Mesh 在绑定时的变换
            lMatrix = lCluster->GetTransformMatrix(lMatrix);
            Display3DVector("        Transform Translation: ", lMatrix.GetT());
            Display3DVector("        Transform Rotation: ", lMatrix.GetR());
            Display3DVector("        Transform Scaling: ", lMatrix.GetS());

            // 骨骼在绑定时的变换（Bind Pose）
            lMatrix = lCluster->GetTransformLinkMatrix(lMatrix);
            Display3DVector("        Transform Link Translation: ", lMatrix.GetT());
            Display3DVector("        Transform Link Rotation: ", lMatrix.GetR());
            Display3DVector("        Transform Link Scaling: ", lMatrix.GetS());

            // 关联模型（高级情况，一般可以忽略）
            if (lCluster->GetAssociateModel() != NULL)
            {
                lMatrix = lCluster->GetTransformAssociateModelMatrix(lMatrix);
                DisplayString("        Associate Model: ", (char*)lCluster->GetAssociateModel()->GetName());
                Display3DVector("        Associate Model Translation: ", lMatrix.GetT());
                Display3DVector("        Associate Model Rotation: ", lMatrix.GetR());
                Display3DVector("        Associate Model Scaling: ", lMatrix.GetS());
            }
        }
    }
}




std::vector<std::vector<BoneWeight>> BuildCPWeights(FbxGeometry* pGeometry)
{
    std::vector<std::vector<BoneWeight>> cpWeights;

    if (!pGeometry)
        return cpWeights;

    // ===============================
    // 关键一步：先给 Control Point 数量分配空间
    // ===============================
    int controlPointCount = pGeometry->GetControlPointsCount();
    cpWeights.resize(controlPointCount);

    // 一个 Mesh 可能有多个 Skin（一般只有 1 个）
    int skinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);

    // ===============================
    // 遍历所有 Skin
    // ===============================
    for (int i = 0; i < skinCount; ++i)
    {
        FbxSkin* skin = (FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin);
        if (!skin) continue;

        int clusterCount = skin->GetClusterCount();

        // ===============================
        // 遍历 Skin 里的每一根骨骼（Cluster）
        // ===============================
        for (int j = 0; j < clusterCount; ++j)
        {
            FbxCluster* cluster = skin->GetCluster(j);
            if (!cluster) continue;

            // j 就是“这根骨骼的索引”
            uint32_t boneIndex = (uint32_t)j;

            int indexCount = cluster->GetControlPointIndicesCount();
            int* indices = cluster->GetControlPointIndices();
            double* weights = cluster->GetControlPointWeights();

            // ===============================
            // 这根骨骼影响的所有 Control Point
            // ===============================
            for (int k = 0; k < indexCount; ++k)
            {
                int cpIndex = indices[k];
                float w = (float)weights[k];

                if (cpIndex < 0 || cpIndex >= controlPointCount)
                    continue;

                if (w <= 0.0f)
                    continue;

                BoneWeight bw;
                bw.boneIndex = boneIndex;
                bw.weight = w;

                cpWeights[cpIndex].push_back(bw);
            }
        }
    }

    return cpWeights;
}

