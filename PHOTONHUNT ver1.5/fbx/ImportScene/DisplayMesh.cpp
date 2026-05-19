/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayMesh.h"

#include "DisplayMaterial.h"
#include "DisplayTexture.h"
#include "DisplayLink.h"
#include "DisplayShape.h"
#include "DisplayCache.h"
#include "../../Utility.h"
#include "../../texture.h"
#include "../../player.h"
#include "DisplaySkeleton.h"

#if defined (FBXSDK_ENV_MAC)
// disable the 吐ormat not a string literal and no format arguments・warning since
// the FBXSDK_printf calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#define MAT_HEADER_LENGTH 200

void DisplayControlsPoints(FbxMesh* pMesh);
void DisplayPolygons(FbxMesh* pMesh, std::vector<BoneInfo> boneinfo);
void DisplayMaterialMapping(FbxMesh* pMesh);
void DisplayTextureMapping(FbxMesh* pMesh);
void DisplayTextureNames( FbxProperty &pProperty, FbxString& pConnectionString );
void DisplayMaterialConnections(FbxMesh* pMesh);
void DisplayMaterialTextureConnections( FbxSurfaceMaterial* pMaterial, 
                                       char * header, int pMatId, int l );

std::vector<BoneInfo> GetBoneInfoLine(FbxMesh* mesh);

FbxMesh* lMesh;
FbxMesh* gSkinMesh = nullptr;
void DisplayMesh(FbxNode* pNode)
{
	// 从节点属性中取出 Mesh 数据
	// 注意：这里只在 AttributeType == eMesh 时才会进来
	lMesh = (FbxMesh*)pNode->GetNodeAttribute();

	{

		DisplayPolygons(lMesh,GetBoneInfoLine(lMesh));
	}

}

std::vector<BoneInfo> GetBoneInfoLine(FbxMesh* mesh)
{
	  std::unordered_map<FbxNode*, BoneInfo*> boneMap;

	for (auto& b : Getbones())
	{
		boneMap[b.node] = &b;
	}

	std::vector<BoneInfo> sortedBones;

	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

	for (int s = 0; s < skinCount; ++s)
	{

		FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(s, FbxDeformer::eSkin);
		int clusterCount = skin->GetClusterCount();

		std::cout << "Skin " << s
			<< " clusterCount = "
			<< clusterCount << std::endl;

		for (int c = 0; c < clusterCount; ++c)
		{
			FbxCluster* cluster = skin->GetCluster(c);
			FbxNode* boneNode = cluster->GetLink();

			auto it = boneMap.find(boneNode);
			if (it == boneMap.end())
			{
				printf("⚠ missing bone: %s\n", boneNode->GetName());
				continue;
			}

			BoneInfo bone = *it->second; // 拷贝已有 BoneInfo（含 node）

			// ===============================
			// 1️⃣ 取绑定时「骨骼全局矩阵」
			// ===============================
			FbxAMatrix bindBoneGlobal;
			cluster->GetTransformLinkMatrix(bindBoneGlobal);

			// ===============================
			// 2️⃣ 取绑定时「Mesh 全局矩阵」
			// （mesh 不在原点时必须）
			// ===============================
			FbxAMatrix bindMeshGlobal;
			cluster->GetTransformMatrix(bindMeshGlobal);

			// 绑定姿态下，骨骼相对于 mesh 的矩阵
			FbxAMatrix bindFinal =
				bindBoneGlobal * bindMeshGlobal.Inverse();

			// ===============================
			// 3️⃣ FBX → XMMATRIX
			// ===============================
			DirectX::XMMATRIX bindDX = DirectX::XMMatrixSet(
				(float)bindFinal[0][0], (float)bindFinal[0][1], (float)bindFinal[0][2], (float)bindFinal[0][3],
				(float)bindFinal[1][0], (float)bindFinal[1][1], (float)bindFinal[1][2], (float)bindFinal[1][3],
				(float)bindFinal[2][0], (float)bindFinal[2][1], (float)bindFinal[2][2], (float)bindFinal[2][3],
				(float)bindFinal[3][0], (float)bindFinal[3][1], (float)bindFinal[3][2], (float)bindFinal[3][3]
			);

			const DirectX::XMMATRIX FlipZ =
				DirectX::XMMatrixScaling(1.0f, 1.0f, -1.0f);

			// ===============================
			// 4️⃣ 坐标系修正（与你 current 用同一套）
			// ===============================
			bindDX = FlipZ * bindDX * FlipZ;

			// ===============================
			// 5️⃣ 求逆并存起来（一次性）
			// ===============================
			DirectX::XMMATRIX invBindDX =
				DirectX::XMMatrixInverse(nullptr, bindDX);

			DirectX::XMStoreFloat4x4(&bone.invBindPose, invBindDX);

			// 按 Cluster 顺序 push（= GPU bone index）
			sortedBones.push_back(bone);
		}
	}

	return sortedBones;
}


void DisplayControlsPoints(FbxMesh* pMesh)
{
	// 控制点数量（几何点池大小）
	int i, lControlPointsCount = pMesh->GetControlPointsCount();

	// 获取控制点数组
	// 👉 每一个 Control Point 本质上是一个 3D 坐标（FbxVector4，w 通常不用）
	FbxVector4* lControlPoints = pMesh->GetControlPoints();

	DisplayString("    Control Points");

	// 遍历所有控制点
	for (i = 0; i < lControlPointsCount; i++)
	{
		// 输出控制点编号
		DisplayInt("        Control Point ", i);

		// 输出控制点的坐标
		// 👉 这是 FBX 中“最原始的几何坐标”
		Display3DVector("            Coordinates: ", lControlPoints[i]);

		// ===== 下面这一段是在处理「法线」 =====
		// 一个 Mesh 里可能有多个 Normal 元素（很少见，但 FBX 支持）
		for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
		{
			// 取出第 j 组法线数据
			FbxGeometryElementNormal* leNormals = pMesh->GetElementNormal(j);

			// 判断法线是“按控制点存”的
			// eByControlPoint 表示：每个 Control Point 对应一个法线
			if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				char header[100];
				FBXSDK_sprintf(header, 100, "            Normal Vector: ");

				// 判断引用方式
				// eDirect：法线数组与控制点一一对应
				if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					// 输出第 i 个控制点对应的法线
					Display3DVector(header, leNormals->GetDirectArray().GetAt(i));
				}
			}
		}
	}

	DisplayString("");
}


std::vector<Object_3D> fbx_obj;
static RenderStateDesc desc;
FbxMesh* GetlMesh()
{
	return gSkinMesh;
}
std::vector<Object_3D>& GetFBXOBJ(void)
{
	return fbx_obj;
}

std::vector<int> bbb;
bool ok = true;
void DisplayPolygons(FbxMesh* pMesh, std::vector<BoneInfo> boneinfo)
{

	auto cpWeights = BuildCPWeights(pMesh);


	Object_3D obj;
	if (boneinfo.size() != 0)
	{

		for (int i = 0;i < boneinfo.size();i++)
		{
			Object_3D::BoneInfo_3D BoneInfos;
			BoneInfos.invBindPose = boneinfo[i].invBindPose;
			BoneInfos.node= boneinfo[i].node;
			BoneInfos.skinMatrix = boneinfo[i].skinMatrix;
			obj.BoneInfos.push_back(BoneInfos);
		}
		D3D11_BUFFER_DESC desc{};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(SkinningCB);
		desc.Usage = D3D11_USAGE_DEFAULT;

		Win->Gfx().pDevice->CreateBuffer(&desc, nullptr, obj.skinCBuf.GetAddressOf());
	}

	desc.cullBack = false;
	obj.pPixelShader = pPixelShader_3D;
	//obj.pVertexShader = GetpVS();
	//obj.pVertexShader = pVertexShader_3D;
	obj.type = RenderLayer::Opaque;
	obj.pRasterizerState = CreateRasterizerState_3D(desc);
	obj.pTextures.push_back(LoadTexture_("rom/color/uv_2k.png"));
	std::vector<VertexStatic3D> vectexs;
	std::vector<uint32_t> indices_32;    // 索引列表

	// mesh 里一共有多少个“面”（三角形 / 四边形）
	int i, j, lPolygonCount = pMesh->GetPolygonCount();

	// 所有“几何位置”的总仓库
	// 注意：这里只是点的位置，还不是最终顶点
	FbxVector4* lControlPoints = pMesh->GetControlPoints();

	char header[100];

	//DisplayString("    Polygons");

	// FBX 里没有真正的“顶点序号”
	// vertexId 是我们自己数的：第几个“面角”
	int vertexId = 0;
	// ===============================
   // 第一层：按“面”遍历
   // ===============================
    for (i = 0; i < lPolygonCount; i++)
    {
        //DisplayInt("        Polygon ", i);
        int l;

		// ===============================
		 // 这个面属于哪个“分组 / 材质”
		 // ===============================
		for (l = 0; l < pMesh->GetElementPolygonGroupCount(); l++)
		{
			FbxGeometryElementPolygonGroup* lePolgrp = pMesh->GetElementPolygonGroup(l);

			// FBX 的意思是：一个面 → 一个分组编号
			switch (lePolgrp->GetMappingMode())
			{
			case FbxGeometryElement::eByPolygon:
				if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
				{
					// 这个面的分组编号
					int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
					//DisplayInt("        Assigned to group: ", polyGroupId);
				}
				break;
			default:
				//DisplayString("        \"unsupported group assignment\"");
				break;
			}
		}

		// 当前这个面有几个“角”
		int lPolygonSize = pMesh->GetPolygonSize(i);

		// ===============================
		// 第二层：遍历这个面的每一个“角”
		// ===============================
		for (j = 0; j < lPolygonSize; j++)
		{
			VertexStatic3D vectex;
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);

			if (lControlPointIndex < 0)
			{
				//DisplayString("            Coordinates: Invalid index found!");
				continue;
			}
			else
			{
				// 通过 Control Point 索引，拿到真正的位置
				//Display3DVector("            Coordinates: ", lControlPoints[lControlPointIndex]);
				DirectX::XMFLOAT4 v = Display3Dvector(lControlPoints[lControlPointIndex]);
				vectex.v = { v.x,v.y,-v.z };
			}

			{
				// 1. 清零
				for (int n = 0; n < 4; ++n)
				{
					vectex.boneIndex[n] = 0;
					vectex.boneWeight[n] = 0.0f;
				}

				// 2. 拿 Control Point 权重
				auto& weights = cpWeights[lControlPointIndex];
				if (weights.empty())
					return; // 没有骨骼影响

				// 3. 按权重排序（关键）
				std::sort(weights.begin(), weights.end(),
					[](const BoneWeight& a, const BoneWeight& b)
					{
						return a.weight > b.weight;
					});

				// 4. 取前 4 个
				int count = (std::min)(4, (int)weights.size());
				float sum = 0.0f;
				
				for (int n = 0; n < count; ++n)
				{
					vectex.boneIndex[n] = weights[n].boneIndex;
					vectex.boneWeight[n] = weights[n].weight;
					sum += weights[n].weight;
				}


			}
			// ===============================
			// 顶点颜色
			// ===============================

			{
				//for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
				//{
				//	FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor( l);
				//	FBXSDK_sprintf(header, 100, "            Color vertex: "); 
				//	// 颜色到底是“跟点走”还是“跟面角走”
				//	switch (leVtxc->GetMappingMode())
				//	{
				//	default:
				//	    break;
				//	case FbxGeometryElement::eByControlPoint:
				//		// 每个 Control Point 一个颜色
				//		switch (leVtxc->GetReferenceMode())
				//		{
				//		case FbxGeometryElement::eDirect:
				//			DisplayColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
				//			break;
				//		case FbxGeometryElement::eIndexToDirect:
				//			{
				//				int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
				//				DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
				//			}
				//			break;
				//		default:
				//			break; // other reference modes not shown here!
				//		}
				//		break;

				//	case FbxGeometryElement::eByPolygonVertex:
				//		{// 每一个“面角”一个颜色
				//			switch (leVtxc->GetReferenceMode())
				//			{
				//			case FbxGeometryElement::eDirect:
				//				DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
				//				break;
				//			case FbxGeometryElement::eIndexToDirect:
				//				{
				//					int id = leVtxc->GetIndexArray().GetAt(vertexId);
				//					DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
				//				}
				//				break;
				//			default:
				//				break; // other reference modes not shown here!
				//			}
				//		}
				//		break;

				//	case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
				//	case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
				//	case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
				//		break;
				//	}
				//}
			}

			// ===============================
			// UV（贴图坐标）
			// ===============================
			for (l = 0; l < pMesh->GetElementUVCount(); ++l)
			{
				// 取第 l 组 UV（一个 mesh 里可能有多套 UV）
				FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);

				// 用来显示用的文字，这里对你后续逻辑没影响
				//FBXSDK_sprintf(header, 100, "            Texture UV: ");

				// -------------------------------
				// 判断：UV 是“跟谁走的”
				// -------------------------------
				switch (leUV->GetMappingMode())
				{
				default:
					// 不是我们关心的存法，直接不处理
					break;

					// ===============================
					// 情况 1：UV 跟 Control Point 走
					// ===============================
				case FbxGeometryElement::eByControlPoint:
				{
					// 也就是说：同一个位置点，用同一个 UV
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					{
						// UV 直接按 Control Point 索引取
						//Display2DVector(header,leUV->GetDirectArray().GetAt(lControlPointIndex));
						vectex.vt = Display2DVector(leUV->GetDirectArray().GetAt(lControlPointIndex));
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						// Control Point 先拿一个 index
						int id = leUV->GetIndexArray().GetAt(lControlPointIndex);

						// 再用这个 index 去 UV 表里找真正的 UV
						//Display2DVector(header,leUV->GetDirectArray().GetAt(id));
						vectex.vt = Display2DVector(leUV->GetDirectArray().GetAt(id));
					}
					break;

					default:
						break; // 其它情况这里不处理
					}
				}
					break;

					// ===============================
					// 情况 2：UV 跟“面角”走（最常见）
					// ===============================
				case FbxGeometryElement::eByPolygonVertex:
				{
					// 这个函数的意思是：
					// “当前这个面 + 当前这个角，对应哪一个 UV”
					int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);

					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					case FbxGeometryElement::eIndexToDirect:
						// 不管是哪种，这里最终都是用 lTextureUVIndex
						// 去 UV 数组里拿真正的 UV
						//Display2DVector(header,leUV->GetDirectArray().GetAt(lTextureUVIndex));
						vectex.vt = Display2DVector(leUV->GetDirectArray().GetAt(lTextureUVIndex));
						break;

					default:
						break; // 其它情况不处理
					}
				}
				break;

				// ===============================
				// 下面这些情况：基本不用管
				// ===============================
				case FbxGeometryElement::eByPolygon:
				case FbxGeometryElement::eAllSame:
				case FbxGeometryElement::eNone:
					// UV 很少用这些方式存
					break;
				}
			}

			// ===============================
			// 法线
			// ===============================
			for( l = 0; l < pMesh->GetElementNormalCount(); ++l)
			{
				FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal( l);
				//FBXSDK_sprintf(header, 100, "            Normal: "); 
				// 法线一般是按“面角”来的
				if(leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						//Display3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));
						DirectX::XMFLOAT4 v = Display3Dvector(leNormal->GetDirectArray().GetAt(vertexId));
						vectex.vn = { v.x,v.y,-v.z };
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							//Display3DVector(header, leNormal->GetDirectArray().GetAt(id));
							DirectX::XMFLOAT4 v = Display3Dvector(leNormal->GetDirectArray().GetAt(id));
							vectex.vn = { v.x,v.y,-v.z };
						}
						break;
					default:
						break; // other reference modes not shown here!
					}
				}

			}


			vectexs.push_back(vectex);
			vertexId++;
		} // for polygonSize



		if (lPolygonSize == 3)
		{
			for (j = 0; j < lPolygonSize; j++)
			{
				indices_32.push_back(vectexs.size() - lPolygonSize + j);
			}
		}
		else if (lPolygonSize == 4)
		{
			float vv[4];
			for (j = 0; j < lPolygonSize; j++)
			{
				vv[j] = vectexs.size() - lPolygonSize + j;
			}

			indices_32.push_back(vv[0]);
			indices_32.push_back(vv[1]);
			indices_32.push_back(vv[2]);
			indices_32.push_back(vv[0]);
			indices_32.push_back(vv[2]);
			indices_32.push_back(vv[3]);
		}
    } // for polygonCount

		// ===============================
	// 边是否可见（和渲染关系不大）
	// ===============================
    //check visibility for the edges of the mesh
	for(int l = 0; l < pMesh->GetElementVisibilityCount(); ++l)
	{
		FbxGeometryElementVisibility* leVisibility=pMesh->GetElementVisibility(l);
		//FBXSDK_sprintf(header, 100, "    Edge Visibility : ");
		DisplayString(header);
		switch(leVisibility->GetMappingMode())
		{
		default:
		    break;
			//should be eByEdge
		case FbxGeometryElement::eByEdge:
			//should be eDirect
			for(j=0; j!=pMesh->GetMeshEdgeCount();++j)
			{
				//DisplayInt("        Edge ", j);
				//DisplayBool("              Edge visibility: ", leVisibility->GetDirectArray().GetAt(j));
			}

			break;
		}
	}


	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(vectexs.size() * sizeof(VertexStatic3D));
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
	vertexDataDesc.pSysMem = vectexs.data();


	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, obj.pVertexBuffer.GetAddressOf())))
	{
		MessageBox(NULL, L"Failed to create vertex buffer.", L"Error", MB_OK | MB_ICONERROR);
	}


	D3D11_BUFFER_DESC indexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA indexData = {};

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0u;
	indexBufferDesc.ByteWidth = indices_32.size() * sizeof(uint32_t);
	indexBufferDesc.StructureByteStride = sizeof(uint32_t);

	indexData.pSysMem = indices_32.data();

	obj.indicesCount = indices_32.size();

	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&indexBufferDesc, &indexData, obj.pIndexBuffer.GetAddressOf()))) {
		MessageBox(NULL, L"Failed to create index buffer.", L"Error", MB_OK | MB_ICONERROR);
	}
	fbx_obj.push_back(obj);
    DisplayString("");
}

void DisplayTextureNames( FbxProperty &pProperty, FbxString& pConnectionString )
{
    int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
    if(lLayeredTextureCount > 0)
    {
        for(int j=0; j<lLayeredTextureCount; ++j)
        {
            FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
            int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
            pConnectionString += " Texture ";

            for(int k =0; k<lNbTextures; ++k)
            {
                //lConnectionString += k;
                pConnectionString += "\"";
                pConnectionString += (char*)lLayeredTexture->GetName();
                pConnectionString += "\"";
                pConnectionString += " ";
            }
            pConnectionString += "of ";
            pConnectionString += pProperty.GetName();
            pConnectionString += " on layer ";
            pConnectionString += j;
        }
        pConnectionString += " |";
    }
    else
    {
        //no layered texture simply get on the property
        int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();

        if(lNbTextures > 0)
        {
            pConnectionString += " Texture ";
            pConnectionString += " ";

            for(int j =0; j<lNbTextures; ++j)
            {
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if(lTexture)
                {
                    pConnectionString += "\"";
                    pConnectionString += (char*)lTexture->GetName();
                    pConnectionString += "\"";
                    pConnectionString += " ";
                }
            }
            pConnectionString += "of ";
            pConnectionString += pProperty.GetName();
            pConnectionString += " |";
        }
    }
}

void DisplayMaterialTextureConnections( FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l )
{
    if(!pMaterial)
        return;

    FbxString lConnectionString = "            Material %d -- ";
    //Show all the textures

    FbxProperty lProperty;
    //Diffuse Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
    DisplayTextureNames(lProperty, lConnectionString);

    //DiffuseFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Emissive Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
    DisplayTextureNames(lProperty, lConnectionString);

    //EmissiveFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor);
    DisplayTextureNames(lProperty, lConnectionString);


    //Ambient Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
    DisplayTextureNames(lProperty, lConnectionString); 

    //AmbientFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
    DisplayTextureNames(lProperty, lConnectionString);          

    //Specular Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
    DisplayTextureNames(lProperty, lConnectionString);  

    //SpecularFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Shininess Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
    DisplayTextureNames(lProperty, lConnectionString);

    //Bump Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sBump);
    DisplayTextureNames(lProperty, lConnectionString);

    //Normal Map Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
    DisplayTextureNames(lProperty, lConnectionString);

    //Transparent Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor);
    DisplayTextureNames(lProperty, lConnectionString);

    //TransparencyFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Reflection Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflection);
    DisplayTextureNames(lProperty, lConnectionString);

    //ReflectionFactor Textures
    lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
    DisplayTextureNames(lProperty, lConnectionString);

    //Update header with material info
	bool lStringOverflow = (lConnectionString.GetLen() + 10 >= MAT_HEADER_LENGTH); // allow for string length and some padding for "%d"
	if (lStringOverflow)
	{
		// Truncate string!
		lConnectionString = lConnectionString.Left(MAT_HEADER_LENGTH - 10);
		lConnectionString = lConnectionString + "...";
	}
	FBXSDK_sprintf(header, MAT_HEADER_LENGTH, lConnectionString.Buffer(), pMatId, l);
	DisplayString(header);
}

void DisplayMaterialConnections(FbxMesh* pMesh)
{
    int i, l, lPolygonCount = pMesh->GetPolygonCount();

    char header[MAT_HEADER_LENGTH];

    DisplayString("    Polygons Material Connections");

    //check whether the material maps with only one mesh
    bool lIsAllSame = true;
    for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
    {

        FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
		if( lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon) 
		{
			lIsAllSame = false;
			break;
		}
    }

    //For eAllSame mapping type, just out the material and texture mapping info once
    if(lIsAllSame)
    {
        for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
        {

            FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial( l);
			if( lMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame) 
			{
				FbxSurfaceMaterial* lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(0));    
				int lMatId = lMaterialElement->GetIndexArray().GetAt(0);
				if(lMatId >= 0)
				{
					DisplayInt("        All polygons share the same material in mesh ", l);
					DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
				}
			}
        }

		//no material
		if(l == 0)
			DisplayString("        no material applied");
    }

    //For eByPolygon mapping type, just out the material and texture mapping info once
    else
    {
        for (i = 0; i < lPolygonCount; i++)
        {
            DisplayInt("        Polygon ", i);

            for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
            {

                FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial( l);
				FbxSurfaceMaterial* lMaterial = NULL;
				int lMatId = -1;
				lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
				lMatId = lMaterialElement->GetIndexArray().GetAt(i);

				if(lMatId >= 0)
				{
					DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
				}
            }
        }
    }
}


void DisplayMaterialMapping(FbxMesh* pMesh)
{
    const char* lMappingTypes[] = { "None", "By Control Point", "By Polygon Vertex", "By Polygon", "By Edge", "All Same" };
    const char* lReferenceMode[] = { "Direct", "Index", "Index to Direct"};

    int lMtrlCount = 0;
    FbxNode* lNode = NULL;
    if(pMesh){
        lNode = pMesh->GetNode();
        if(lNode)
            lMtrlCount = lNode->GetMaterialCount();    
    }

    for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
    {
        FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial( l);
        if (leMat)
        {
            char header[100];
            FBXSDK_sprintf(header, 100, "    Material Element %d: ", l); 
            DisplayString(header);


            DisplayString("           Mapping: ", lMappingTypes[leMat->GetMappingMode()]);
            DisplayString("           ReferenceMode: ", lReferenceMode[leMat->GetReferenceMode()]);

            int lMaterialCount = 0;
            FbxString lString;

            if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect ||
                leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
            {
                lMaterialCount = lMtrlCount;
            }

            if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex ||
                leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
            {
                int i;

                lString = "           Indices: ";

                int lIndexArrayCount = leMat->GetIndexArray().GetCount(); 
                for (i = 0; i < lIndexArrayCount; i++)
                {
                    lString += leMat->GetIndexArray().GetAt(i);

                    if (i < lIndexArrayCount - 1)
                    {
                        lString += ", ";
                    }
                }

                lString += "\n";

                FBXSDK_printf(lString);
            }
        }
    }

    DisplayString("");
}
