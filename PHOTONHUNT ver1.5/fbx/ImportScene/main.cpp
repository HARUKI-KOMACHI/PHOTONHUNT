/****************************************************************************************

   版权所有 (C) 2015 Autodesk, Inc.
   保留所有权利。

   本软件的使用需遵循 Autodesk 许可协议，
   该协议在安装或下载时以电子或纸质形式随附。

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// 本示例演示：
// 如何检测场景是否被密码保护，
// 以及导入并浏览场景以访问节点和动画信息。
// 程序会显示通过参数传入的 FBX 文件内容。
// 你可以使用导出示例生成的各种 FBX 文件进行测试。
//
/////////////////////////////////////////////////////////////////////////


#include "../Common/Common.h"              // 公共工具与通用函数
#include "DisplayCommon.h"                 // 通用显示辅助
#include "DisplayHierarchy.h"              // 场景层级（节点树）显示
#include "DisplayAnimation.h"              // 动画信息显示
#include "DisplayMarker.h"                 // 标记（Marker）显示
#include "DisplaySkeleton.h"               // 骨骼结构显示
#include "DisplayMesh.h"                   // 网格数据显示
#include "DisplayNurb.h"                   // NURBS 曲线/曲面显示
#include "DisplayPatch.h"                  // Patch 曲面显示
#include "DisplayLodGroup.h"               // LOD 组显示
#include "DisplayCamera.h"                 // 摄像机信息显示
#include "DisplayLight.h"                  // 灯光信息显示
#include "DisplayGlobalSettings.h"          // 全局设置显示
#include "DisplayPose.h"                   // Pose（姿势）显示
#include "DisplayPivotsAndLimits.h"         // 枢轴与限制信息显示
#include "DisplayUserProperties.h"          // 用户自定义属性显示
#include "DisplayGenericInfo.h"             // 通用/杂项信息显示

// 本地函数声明
void DisplayContent(FbxScene* pScene);          // 显示场景内容
void DisplayContent(FbxNode* pNode);            // 显示节点内容
void DisplayTarget(FbxNode* pNode);              // 显示目标信息
void DisplayTransformPropagation(FbxNode* pNode); // 显示变换传播方式
void DisplayGeometricTransform(FbxNode* pNode);  // 显示几何变换
void DisplayMetaData(FbxScene* pScene);          // 显示元数据

static bool gVerbose = true;                     // 是否输出详细信息


int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;   // FBX SDK 管理器
    FbxScene* lScene = NULL;         // FBX 场景
    bool lResult;                    // 加载结果

    // 初始化 FBX SDK
    InitializeSdkObjects(lSdkManager, lScene);

    // 加载场景
    // 示例程序可以通过参数接收一个 FBX 文件
    FbxString lFilePath("");
    for (int i = 1, c = argc; i < c; ++i)
    {
        if (FbxString(argv[i]) == "-test") gVerbose = false; // 关闭详细输出
        else if (lFilePath.IsEmpty()) lFilePath = argv[i];  // 获取 FBX 文件路径
    }

    if (lFilePath.IsEmpty())
    {
        lResult = false;
        FBXSDK_printf("\n\nUsage: ImportScene <FBX file name>\n\n"); // 用法提示
    }
    else
    {
        FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer()); // 输出文件名
        lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer()); // 加载场景
    }

    if (lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene..."); // 加载失败
    }
    else
    {
        // 显示场景内容
        DisplayMetaData(lScene); // 显示元数据

        FBXSDK_printf("\n\n---------------------\nGlobal Light Settings\n---------------------\n\n");
        if (gVerbose) DisplayGlobalLightSettings(&lScene->GetGlobalSettings()); // 全局灯光设置

        FBXSDK_printf("\n\n----------------------\nGlobal Camera Settings\n----------------------\n\n");
        if (gVerbose) DisplayGlobalCameraSettings(&lScene->GetGlobalSettings()); // 全局相机设置

        FBXSDK_printf("\n\n--------------------\nGlobal Time Settings\n--------------------\n\n");
        if (gVerbose) DisplayGlobalTimeSettings(&lScene->GetGlobalSettings()); // 全局时间设置

        FBXSDK_printf("\n\n---------\nHierarchy\n---------\n\n");
        if (gVerbose) DisplayHierarchy(lScene); // 层级结构

        FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");
        if (gVerbose) DisplayContent(lScene); // 节点内容

        FBXSDK_printf("\n\n----\nPose\n----\n\n");
        if (gVerbose) DisplayPose(lScene); // 姿势信息

        FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");
        if (gVerbose) DisplayAnimation(lScene); // 动画信息

        // 显示通用信息
        FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
        if (gVerbose) DisplayGenericInfo(lScene); // 通用信息
    }

    // 销毁 FBX SDK 创建的所有对象
    DestroySdkObjects(lSdkManager, lResult);

    return 0;

}

void DisplayContent(FbxScene* pScene)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if(lNode)
    {
        for(i = 0; i < lNode->GetChildCount(); i++)
        {
            DisplayContent(lNode->GetChild(i));
        }
    }
}

void DisplayContent(FbxNode* pNode)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if(pNode->GetNodeAttribute() == NULL)
    {
        FBXSDK_printf("NULL Node Attribute\n\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
	    default:
	        break;
        case FbxNodeAttribute::eMarker:  
            DisplayMarker(pNode);
            break;

        case FbxNodeAttribute::eSkeleton:  
            DisplaySkeleton(pNode);
            break;

        case FbxNodeAttribute::eMesh:      
            DisplayMesh(pNode);
            break;

        case FbxNodeAttribute::eNurbs:      
            DisplayNurb(pNode);
            break;

        case FbxNodeAttribute::ePatch:     
            DisplayPatch(pNode);
            break;

        case FbxNodeAttribute::eCamera:    
            DisplayCamera(pNode);
            break;

        case FbxNodeAttribute::eLight:     
            DisplayLight(pNode);
            break;

        case FbxNodeAttribute::eLODGroup:
            DisplayLodGroup(pNode);
            break;
        }   
    }

    DisplayUserProperties(pNode);
    DisplayTarget(pNode);
    DisplayPivotsAndLimits(pNode);
    DisplayTransformPropagation(pNode);
    DisplayGeometricTransform(pNode);

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        DisplayContent(pNode->GetChild(i));
    }
}


void DisplayTarget(FbxNode* pNode)
{
    if(pNode->GetTarget() != NULL)
    {
        DisplayString("    Target Name: ", (char *) pNode->GetTarget()->GetName());
    }
}

void DisplayTransformPropagation(FbxNode* pNode)
{
    FBXSDK_printf("    Transformation Propagation\n");

    // 
    // Rotation Space
    //
    EFbxRotationOrder lRotationOrder;
    pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);

    FBXSDK_printf("        Rotation Space: ");

    switch (lRotationOrder)
    {
    case eEulerXYZ: 
        FBXSDK_printf("Euler XYZ\n");
        break;
    case eEulerXZY:
        FBXSDK_printf("Euler XZY\n");
        break;
    case eEulerYZX:
        FBXSDK_printf("Euler YZX\n");
        break;
    case eEulerYXZ:
        FBXSDK_printf("Euler YXZ\n");
        break;
    case eEulerZXY:
        FBXSDK_printf("Euler ZXY\n");
        break;
    case eEulerZYX:
        FBXSDK_printf("Euler ZYX\n");
        break;
    case eSphericXYZ:
        FBXSDK_printf("Spheric XYZ\n");
        break;
    }

    //
    // Use the Rotation space only for the limits
    // (keep using eEulerXYZ for the rest)
    //
    FBXSDK_printf("        Use the Rotation Space for Limit specification only: %s\n",
        pNode->GetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot) ? "Yes" : "No");


    //
    // Inherit Type
    //
    FbxTransform::EInheritType lInheritType;
    pNode->GetTransformationInheritType(lInheritType);

    FBXSDK_printf("        Transformation Inheritance: ");

    switch (lInheritType)
    {
    case FbxTransform::eInheritRrSs:
        FBXSDK_printf("RrSs\n");
        break;
    case FbxTransform::eInheritRSrs:
        FBXSDK_printf("RSrs\n");
        break;
    case FbxTransform::eInheritRrs:
        FBXSDK_printf("Rrs\n");
        break;
    }
}

void DisplayGeometricTransform(FbxNode* pNode)
{
    FbxVector4 lTmpVector;

    FBXSDK_printf("    Geometric Transformations\n");

    //
    // Translation
    //
    lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    FBXSDK_printf("        Translation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    //
    // Rotation
    //
    lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    FBXSDK_printf("        Rotation:    %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

    //
    // Scaling
    //
    lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
    FBXSDK_printf("        Scaling:     %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
}


void DisplayMetaData(FbxScene* pScene)
{
    FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
    if (sceneInfo)
    {
        FBXSDK_printf("\n\n--------------------\nMeta-Data\n--------------------\n\n");
        FBXSDK_printf("    Title: %s\n", sceneInfo->mTitle.Buffer());
        FBXSDK_printf("    Subject: %s\n", sceneInfo->mSubject.Buffer());
        FBXSDK_printf("    Author: %s\n", sceneInfo->mAuthor.Buffer());
        FBXSDK_printf("    Keywords: %s\n", sceneInfo->mKeywords.Buffer());
        FBXSDK_printf("    Revision: %s\n", sceneInfo->mRevision.Buffer());
        FBXSDK_printf("    Comment: %s\n", sceneInfo->mComment.Buffer());

        FbxThumbnail* thumbnail = sceneInfo->GetSceneThumbnail();
        if (thumbnail)
        {
            FBXSDK_printf("    Thumbnail:\n");

            switch (thumbnail->GetDataFormat())
            {
            case FbxThumbnail::eRGB_24:
                FBXSDK_printf("        Format: RGB\n");
                break;
            case FbxThumbnail::eRGBA_32:
                FBXSDK_printf("        Format: RGBA\n");
                break;
            }

            switch (thumbnail->GetSize())
            {
	        default:
	            break;
            case FbxThumbnail::eNotSet:
                FBXSDK_printf("        Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
                break;
            case FbxThumbnail::e64x64:
                FBXSDK_printf("        Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
                break;
            case FbxThumbnail::e128x128:
                FBXSDK_printf("        Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
            }
        }
    }
}

