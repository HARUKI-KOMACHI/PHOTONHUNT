#include "DX_Spine.h"
#include "Utility.h"
#include "texture.h"

std::vector< Object_2D> Object_2D_spine;
std::vector<Spine*> object_spine_list_Darw;

Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_Spine;//创建像素shader
Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_Spine;// 创建顶点shader
Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout_Spine;       // 输入布局


Microsoft::WRL::ComPtr<ID3D11Buffer> m_spineCB;       //对象常量

void LoadSpineAtlas(Atlas& atlas, std::string path)
{
    std::ifstream file(path);
    std::string line;
    std::string atlasDir = path.substr(0, path.find_last_of("/\\") + 1);


    std::string currentPageName="";
    AtlasPage currentPage;
    AtlasRegion currentRegion;
    bool inRegion = false;
    bool inPage = false;

    while (std::getline(file, line)) {
        // 去除左右空白
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) {
            inRegion = false;
            continue;
        }

        // 页面起点（xxx.png）
        if (line.find(".png") != std::string::npos) {
            // 先保存上一页
            if (!currentPageName.empty()) {
                atlas.pages[currentPageName] = currentPage;
            }
            currentPageName = line;
            currentPage = {};
            currentPage.imageFile = currentPageName;
            // ✅ 关键：拼完整路径
            std::string nammm = atlasDir + currentPageName;
            currentPage.pTexture = LoadTexture_(atlasDir + currentPageName);

            inPage = true;
            continue;
        }

        // 键值对
        if (line.find(":") != std::string::npos) {
            auto pos = line.find(":");
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));

            if (inPage) {
                if (key == "size") {
                    sscanf(value.c_str(), "%d,%d", &currentPage.width, &currentPage.height);
                }
                // 其他页面字段可扩展...
            }
            else if (inRegion) {
                if (key == "rotate") {
                    currentRegion.rotated = (value == "true");
                }
                else if (key == "xy") {
                    sscanf(value.c_str(), "%d,%d", &currentRegion.x, &currentRegion.y);
                }
                else if (key == "size") {
                    sscanf(value.c_str(), "%d,%d", &currentRegion.width, &currentRegion.height);
                }
                else if (key == "orig") {
                    sscanf(value.c_str(), "%d,%d", &currentRegion.origWidth, &currentRegion.origHeight);
                }
                else if (key == "offset") {
                    sscanf(value.c_str(), "%d,%d", &currentRegion.offsetX, &currentRegion.offsetY);
                }
            }

            continue;
        }

        // region 名字行
        if (!line.empty()) {
            // 如果已在 region 块，说明是新 region 的开始，要保存当前 region
            if (inRegion && !currentRegion.name.empty()) {
                atlas.regions[currentRegion.name] = currentRegion;
            }

            currentRegion = {};
            currentRegion.name = line;
            currentRegion.pageImage = currentPageName;
            inRegion = true;
            inPage = false;
        }
    }

    // 末尾收尾
    if (inRegion && !currentRegion.name.empty()) {
        atlas.regions[currentRegion.name] = currentRegion;
    }

    if (!currentPageName.empty()) {
        atlas.pages[currentPageName] = currentPage;
        atlas.name = currentPageName;
    }


}

Spine::Spine(std::string path)
{

    std::string jsonPath = ChangeExtension(path, ".json");
    std::string atlasPath = ChangeExtension(path, ".atlas");
    std::string imagePath = ChangeExtension(path, ".png");

    LoadSpineAtlas(atlas, atlasPath);
    pTexture = LoadTexture_(imagePath);
    /////////////////////////////////////////////////////////////
    //读取层对象
    std::ifstream file(jsonPath);
    j = json::parse(file);


    if (!j.contains("slots") || !j["slots"].is_array()) {
        std::cerr << "❌ JSON 中没有 slots 或格式错误" << std::endl;
        return;
    }

    for (const auto& slotJson : j["slots"]) {
        Spine::Slot slot;

        if (slotJson.contains("name")) {
            slot.name = slotJson["name"].get<std::string>();
        }

        if (slotJson.contains("bone")) {
            slot.bone = slotJson["bone"].get<std::string>();
        }

        if (slotJson.contains("attachment")) {
            slot.attachment = slotJson["attachment"].get<std::string>();
        }
        else {
            slot.attachment = ""; // 没有的话留空
        }

        slots.push_back(slot);
    }
////////////////////////////////////////////////////////////////////////////////
   //读取骨骼
    if (!j.contains("bones")) return ;

    for (const auto& boneJson : j["bones"]) {
        Spine::Bone bone;
        std::string name= boneJson.value("name", "");

        //bone.parent = boneJson.value("parent", "");
        bone.pos.x = boneJson.value("x", 0.0f);
        bone.pos.y = boneJson.value("y", 0.0f);
        bone.rotation = boneJson.value("rotation", 0.0f);
        bone.scale.x = boneJson.value("scaleX", 1.0f);
        bone.scale.y = boneJson.value("scaleY", 1.0f);
        bones[name] = bone;
        bones_vector.push_back(bone);
    }

    ////////////////////////////////////////////////////////
    //读取对象

    if (!j.contains("skins") || !j["skins"].is_array()) return;

    for (const auto& skin : j["skins"]) {
        if (!skin.contains("attachments") || !skin["attachments"].is_object()) continue;

        const auto& skinAttachments = skin["attachments"];

        for (const auto& [slotName, slotMap] : skinAttachments.items()) {
            for (const auto& [attachmentName, innerData] : slotMap.items()) {

                const auto& data = innerData;  // ✅ 真正的属性结构

                Attachment a;
                a.slotName = slotName;
                a.attachmentName = attachmentName;
                //a.path = data.value("path", attachmentName);
                std::vector<float> vertices;
                std::vector<float> uvs;
                if (data.contains("type") && data["type"] == "mesh") {
                    //a.isMesh = true;
                    a.size.x = data.value("width", 0.0f);
                    a.size.y = data.value("height", 0.0f);
                    a.rotation = data.value("rotation", 0.0f);

                    for (auto v : data.value("vertices", json::array()))
                        vertices.push_back(v.get<float>());

                    for (auto uv : data.value("uvs", json::array()))
                        uvs.push_back(uv.get<float>());

                    for (auto t : data.value("triangles", json::array()))
                        a.indices_32.push_back(static_cast<uint32_t>(t.get<int>()));

                    bool hasWeight = (vertices.size() != uvs.size());

                    if (!hasWeight)
                    {

                        for (int i = 0; i < uvs.size(); i += 2)
                        {
                            Vector_Spine point{};
                            point.position = { vertices[i], vertices[i + 1],0.0f, 0.0f };
                            point.texcoord = { uvs[i],uvs[i + 1] };

                            // 单骨骼绑定（默认）
                            point.boneIndex[0] = 0;
                            point.boneWeight[0] = 1.0f;

                            a.points.push_back(point);
                        }
                    }
                    else // hasWeight == true
                    {
                        a.indices_32.clear();
                        //int v = 0;                       // vertices 游标

                        //for (int i = 0; i < uvs.size(); i += 2)
                        //{
                        //    Vector_Spine point{};
                        //    point.texcoord = { uvs[i],uvs[i + 1] };

                        //    int boneCount = vertices[v++];

                        //    // 只取前 4 个影响（你的 Vector_Spine 正好支持）

                        //    int used = (std::min)(boneCount, 4);

                        //    for (int b = 0; b < used; b++)
                        //    {
                        //        int boneIndex = (int)vertices[v++];
                        //        float x = vertices[v++];
                        //        float y = vertices[v++];
                        //        float weight = vertices[v++];

                        //        point.boneIndex[b] = boneIndex;
                        //        point.boneWeight[b] = weight;

                        //        point.position.x += (x + bones_vector[boneIndex].pos.x) * weight;
                        //        point.position.y += (y + bones_vector[boneIndex].pos.y) * weight;
                        //    }

                        //    // ⭐ 跳过没用到的 influence（关键！）
                        //    int remain = boneCount - used;
                        //    v += remain * 4;


                        //    point.position.z = 0.0f;
                        //    point.position.w = 1.0f;

                        //    a.points.push_back(point);
                        //}

                        Vector_Spine point{};

                        // 单骨骼绑定（默认）
                        point.boneIndex[0] = 0;
                        point.boneWeight[0] = 1.0f;

                        point.texcoord = { 1,0 };
                        point.position = { a.size.x * -0.5f, a.size.y * -0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 0,0 };
                        point.position = { a.size.x * 0.5f,a.size.y * -0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 1,1 };
                        point.position = { a.size.x * -0.5f,a.size.y * 0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 0,1 };
                        point.position = { a.size.x * 0.5f,a.size.y * 0.5f,0.0f, 0.0f };
                        a.points.push_back(point);


                        a.indices_32.push_back(0);
                        a.indices_32.push_back(1);
                        a.indices_32.push_back(3);
                        a.indices_32.push_back(0);
                        a.indices_32.push_back(3);
                        a.indices_32.push_back(2);
                    }

                }
                else {
                    //a.isMesh = false;
                    a.pos.x = data.value("x", 0.0f);
                    a.pos.y = data.value("y", 0.0f);
                    a.size.x = data.value("width", 0.0f);
                    a.size.y = data.value("height", 0.0f);
                    a.rotation = data.value("rotation", 0.0f);

                    {
                        Vector_Spine point{};

                        // 单骨骼绑定（默认）
                        point.boneIndex[0] = 0;
                        point.boneWeight[0] = 1.0f;

                        point.texcoord = { 1,0 };
                        point.position = { a.size.x * -0.5f, a.size.y * -0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 0,0 };
                        point.position = { a.size.x * 0.5f,a.size.y * -0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 1,1 };
                        point.position = { a.size.x * -0.5f,a.size.y * 0.5f,0.0f, 0.0f };
                        a.points.push_back(point);

                        point.texcoord = { 0,1 };
                        point.position = { a.size.x * 0.5f,a.size.y * 0.5f,0.0f, 0.0f };
                        a.points.push_back(point);


                        a.indices_32.push_back(0);
                        a.indices_32.push_back(1);
                        a.indices_32.push_back(3);
                        a.indices_32.push_back(0);
                        a.indices_32.push_back(3);
                        a.indices_32.push_back(2);
                    }
                }
              
                if (a.points.empty()||a.indices_32.empty()) continue;
                AtlasRegion& region = atlas.regions[attachmentName];
                AtlasPage& page = atlas.pages[atlas.name];

                DirectX::XMFLOAT2 points[4];
                {
                    float texW = (float)page.width;
                    float texH = (float)page.height;

                    float u1 = region.x / texW;
                    float v1 = region.y / texH;
                    float u2 = (region.x + region.width) / texW;
                    float v2 = (region.y + region.height) / texH;

                    points[0] = { u1, v1 }; // 左上
                    points[1] = { u2, v1 }; // 右上
                    points[2] = { u1, v2 }; // 左下
                    points[3] = { u2, v2 }; // 右下

                    if (region.rotated)
                    {
                        DirectX::XMFLOAT2 ve[4] = {};
                        ve[0] = points[0];
                        ve[1] = points[1];
                        ve[2] = points[2];
                        ve[3] = points[3];

                        points[0] = ve[1];
                        points[1] = ve[3];
                        points[2] = ve[0];
                        points[3] = ve[2];


                    }
                }


                for (int i = 0;i < a.points.size();i++)
                {

                    //a.points[i].texcoord.x = (points[1].x - points[0].x) * a.points[i].texcoord.x + points[0].x;
                    //a.points[i].texcoord.y = (points[0].y - points[2].y) * a.points[i].texcoord.y + points[2].y;

                    float uMin = points[0].x;
                    float uMax = points[1].x;
                    float vMin = points[2].y;
                    float vMax = points[0].y;

                    float lu = a.points[i].texcoord.x; // local uv (0~1)
                    float lv = a.points[i].texcoord.y;

                    a.points[i].texcoord.x = uMin + lu * (uMax - uMin);
                    a.points[i].texcoord.y = vMin + lv * (vMax - vMin);

                }

               

                // 设置顶点缓冲区描述
                D3D11_BUFFER_DESC vertexBufferDesc = {};
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                vertexBufferDesc.ByteWidth = static_cast<UINT>(a.points.size() * sizeof(Vector_Spine));
                vertexBufferDesc.CPUAccessFlags = 0;
                vertexBufferDesc.StructureByteStride = 0;

                D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
                vertexDataDesc.pSysMem = a.points.data();

                if (!Win) {
                    MessageBox(nullptr, L"Win is null", L"Error", MB_OK);
                    return ;
                }

                if (!Win->Gfx().pDevice) {
                    MessageBox(nullptr, L"pDevice is null", L"Error", MB_OK);
                    return ;
                }

                if (FAILED(Win->Gfx().pDevice->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, a.pVertexBuffer.GetAddressOf())))
                {
                    MessageBox(NULL, L"Failed to create vertex buffer.", L"Error", MB_OK | MB_ICONERROR);
                    return ;
                }


                D3D11_BUFFER_DESC indexBufferDesc = {};
                D3D11_SUBRESOURCE_DATA indexData = {};

                indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                indexBufferDesc.CPUAccessFlags = 0u;
                indexBufferDesc.ByteWidth = a.indices_32.size() * sizeof(uint32_t);
                indexBufferDesc.StructureByteStride = sizeof(uint32_t);

                indexData.pSysMem = a.indices_32.data();

                if (FAILED(Win->Gfx().pDevice->CreateBuffer(&indexBufferDesc, &indexData, a.pIndexBuffer.GetAddressOf()))) {
                    MessageBox(NULL, L"Failed to create index buffer.", L"Error", MB_OK | MB_ICONERROR);
                    return ;
                }


                a.indicesCount = a.indices_32.size();
                if (a.indices_32.size() == 171)
                {
                    //return;
                }
                // ✅ 创建物体用的常量缓冲 (即使是空的，也先占一个位置)
                D3D11_BUFFER_DESC cbd = {};
                cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                cbd.Usage = D3D11_USAGE_DYNAMIC;
                cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                cbd.ByteWidth = sizeof(MatrixBuffer_new);   // 这里 MatrixBuffer = 你的物体常量结构

                HRESULT hr = Win->Gfx().pDevice->CreateBuffer(&cbd, nullptr, a.pConstantBuffer.GetAddressOf());
                if (FAILED(hr))
                {
                    MessageBox(NULL, L"Failed to create constant buffer.", L"Error", MB_OK | MB_ICONERROR);
                }

                attachments[attachmentName] = a;

            }
        }
    }



    /////////////////////////////////////////////////////
    //读动画
    

    for (const auto& [animationName, animationData] : j["animations"].items())
    {
        // animationName = "test01"
        // animationData = { "bones": { ... } }
        Animation animation_;
        if (!animationData.contains("bones")) continue;

        const auto& bones = animationData["bones"];

        for (const auto& [boneName, boneAnim] : bones.items())
        {
            // boneName = "root"
            // boneAnim = { "translate": [ ... ] }
            Animation::animation_bone animation_done;

            if (boneAnim.contains("translate") && boneAnim["translate"].is_array())
            {
                for (const auto& frame : boneAnim["translate"])
                {
                    Animation::Translate translate;
                    if (frame.empty())
                    {
                        animation_done.translate.push_back(translate);
                        // {} → 初始帧（time = 0，使用默认值）
                        continue;
                    }

                    translate.time = frame.value("time", 0.0f);
                    translate.pos.x = frame.value("x", 0.0f);
                    translate.pos.y = frame.value("y", 0.0f);
                    animation_done.translate.push_back(translate);
                    // 这里处理这一帧
                }
            }


            if (boneAnim.contains("rotate") && boneAnim["rotate"].is_array())
            {
                for (const auto& frame : boneAnim["rotate"])
                {
                    Animation::Rotate rotate;
                    if (frame.empty())
                    {
                        animation_done.rotate.push_back(rotate);
                        continue; // 初始帧
                    }

                    rotate.time = frame.value("time", 0.0f);
                    rotate.rotation = frame.value("angle", 0.0f);
                    animation_done.rotate.push_back(rotate);
                    // 处理这一帧旋转
                }
            }

            if (boneAnim.contains("scale") && boneAnim["scale"].is_array())
            {
                for (const auto& frame : boneAnim["scale"])
                {
                    Animation::Scale scale;
                    if (frame.empty())
                    {
                        animation_done.scale.push_back(scale);
                        continue; // 初始帧
                    }

                    scale.time = frame.value("time", 0.0f);
                    scale.scale.x = frame.value("x", 1.0f);
                    scale.scale.y = frame.value("y", 1.0f);
                    animation_done.scale.push_back(scale);
                    // 处理这一帧缩放
                }
            }

            animation_.bones[boneName] = animation_done;
        }
        animations[animationName] = animation_;
    }


}

Spine::~Spine(void)
{
}


void DrawObject_Spine(Spine* spine)
{
    object_spine_list_Darw.push_back(spine);

    Object_2D object2d;
    object2d.Transform = spine->Transform;
    object2d.texNo = spine->texNo;
    object2d.spine = object_spine_list_Darw.size() - 1;
    Object_2D_spine.push_back(object2d);
    object_2D_list_Darw.push_back(&Object_2D_spine.back());
}

void inti_Object_2D_spine()
{
    Object_2D_spine.clear();
}

void inti_spine()
{

    // 添加着色器shader
    Microsoft::WRL::ComPtr<ID3DBlob> pPixelBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pVertexBlob;

    // 编译像素着色器
    if (FAILED(D3DReadFileToBlob(L"PS_Object_Spine.cso", &pPixelBlob))) {
        MessageBox(NULL, L"Failed to load pixel shader.", L"Error", MB_OK | MB_ICONERROR);
        return ;
    }

    if (FAILED(Win->Gfx().pDevice->CreatePixelShader(pPixelBlob->GetBufferPointer(), pPixelBlob->GetBufferSize(), nullptr, &pPixelShader_Spine))) {
        MessageBox(NULL, L"Failed to create pixel shader.", L"Error", MB_OK | MB_ICONERROR);
        return ;
    }

    // 编译顶点着色器
    if (FAILED(D3DReadFileToBlob(L"VS_Object_Spine.cso", &pVertexBlob))) {
        MessageBox(NULL, L"Failed to load vertex shader.", L"Error", MB_OK | MB_ICONERROR);
        return ;
    }

    if (FAILED(Win->Gfx().pDevice->CreateVertexShader(pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize(), nullptr, &pVertexShader_Spine))) {
        MessageBox(NULL, L"Failed to create vertex shader.", L"Error", MB_OK | MB_ICONERROR);
        return ;
    }

    // 定义输入布局
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        // position : float4
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vector_Spine, position),D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // texcoord : float2
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vector_Spine, texcoord),D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // boneIndex : uint4
        { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0,offsetof(Vector_Spine, boneIndex),D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // boneWeight : float4
        { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,offsetof(Vector_Spine, boneWeight),D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };



    // 使用顶点着色器的字节码创建输入布局
    if (FAILED(Win->Gfx().pDevice->CreateInputLayout(
        layout,
        UINT(std::size(layout)),
        pVertexBlob->GetBufferPointer(),
        pVertexBlob->GetBufferSize(),
        &pInputLayout_Spine)))
    {
        MessageBox(NULL, L"Failed to create input layout.", L"Error", MB_OK | MB_ICONERROR);
        return ;
    }



    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(Spine::Bone);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    HRESULT hr = Win->Gfx().pDevice->CreateBuffer(&desc, nullptr, &m_spineCB);
    assert(SUCCEEDED(hr));

}

void  DrawSpine(int n)
{
    Win->Gfx().SetBlendState(BLENDSTATE::BLENDSTATE_ALFA);

    auto& obj = object_spine_list_Darw[n];
    for (int i = 0;i < obj->slots.size();i++)
    {
        if (obj->slots[i].attachment == "") continue;

        if (obj->atlas.pages.empty()) continue;

        AtlasPage& page = obj->atlas.pages[obj->atlas.name];

        Win->Gfx().pContext->PSSetShaderResources(0, 1, page.pTexture.GetAddressOf());
        
        //Win->Gfx().pContext->PSSetShaderResources(0, 1, obj->pTexture.GetAddressOf());
        auto& obj_att = obj->attachments[obj->slots[i].attachment];
        obj_att.slotName;
        const UINT stride = sizeof(Vector);
        const UINT offset = 0u;
        Win->Gfx().pContext->IASetVertexBuffers(0u, 1u, obj_att.pVertexBuffer.GetAddressOf(), &stride, &offset);
        Win->Gfx().pContext->IASetIndexBuffer(
            obj_att.pIndexBuffer.Get(),
            DXGI_FORMAT_R32_UINT,
            0u
        );



        Win->Gfx().pContext->VSSetShader(pVertexShader_Spine.Get(), nullptr, 0u);
        Win->Gfx().pContext->PSSetShader(pPixelShader_Spine.Get(), nullptr, 0u);
        Win->Gfx().pContext->IASetInputLayout(pInputLayout_Spine.Get());

        Win->Gfx().pContext->OMSetRenderTargets(1u, Win->Gfx().pTarget.GetAddressOf(), nullptr);
        Win->Gfx().pContext->RSSetState(pRasterizerState.Get());
        //常量更新
        ResetConstantBuffer(Win->Gfx().pContext.Get());
        ResetspineCB(obj_att.pos, obj_att.size, obj_att.rotation);
        //常量绑定
        Win->Gfx().pContext->VSSetConstantBuffers(0u, 1u, g_pConstantBuffer.GetAddressOf());
        Win->Gfx().pContext->VSSetConstantBuffers(1u, 1u, m_spineCB.GetAddressOf());
        Win->Gfx().pContext->RSSetViewports(1u, &(*viewport));


        Win->Gfx().pContext->DrawIndexed(obj_att.indicesCount, 0u, 0u);
        //Win->Gfx().pContext->DrawIndexed(obj_att.indices_32.size(), 0u, 0u);
    }
    
}



void ResetspineCB(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, float rotation)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    Win->Gfx().pContext->Map(m_spineCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    Spine::Bone* cb = reinterpret_cast<Spine::Bone*>(mapped.pData);

    // ======== 复原所有参数 ========
    cb->pos = pos;
    cb->scale = scale;
    cb->rotation = rotation;


    Win->Gfx().pContext->Unmap(m_spineCB.Get(), 0);
}