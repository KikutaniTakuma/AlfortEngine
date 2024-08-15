#include "AirSkyBox.h"
#include <array>
#include "Math/MathCommon.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "Engine/Graphics/PipelineManager/PipelineManager.h"

#include "Engine/Graphics/Shader/ShaderManager/ShaderManager.h"

AirSkyBox::~AirSkyBox()
{
    if (shaderData_) {
        Lamb::SafePtr heap = CbvSrvUavHeap::GetInstance();
        heap->ReleaseView(shaderData_->GetHandleUINT());
        heap->ReleaseView(atmosphericParams_->GetHandleUINT());
    }
}

void AirSkyBox::Load() {
    std::array vertexData = {
        Vector4(1.0f,  1.0f, -1.0f, 1.0f),
        Vector4(1.0f, -1.0f, -1.0f, 1.0f),
        Vector4(1.0f,  1.0f,  1.0f, 1.0f),
        Vector4(1.0f, -1.0f,  1.0f, 1.0f),
        Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
        Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
        Vector4(-1.0f,  1.0f,  1.0f, 1.0f),
        Vector4(-1.0f, -1.0f,  1.0f, 1.0f),
    };

    std::array indexData = {
        0_u16, 2_u16, 4_u16,
        3_u16, 7_u16, 2_u16,
        7_u16, 5_u16, 6_u16,
        5_u16, 7_u16, 1_u16,
        1_u16, 3_u16, 0_u16,
        5_u16, 1_u16, 4_u16,
        2_u16, 6_u16, 4_u16,
        7_u16, 6_u16, 2_u16,
        5_u16, 4_u16, 6_u16,
        7_u16, 3_u16, 1_u16,
        3_u16, 2_u16, 0_u16,
        1_u16, 0_u16, 4_u16,
    };

    Lamb::SafePtr directXDevice = DirectXDevice::GetInstance();


    uint32_t indexSizeInBytes = static_cast<uint32_t>(sizeof(uint16_t) * indexData.size());
    uint32_t vertexSizeInBytes = static_cast<uint32_t>(sizeof(Vector4) * vertexData.size());

    indexResource_ = directXDevice->CreateBufferResuorce(indexSizeInBytes);

    Lamb::SafePtr<uint16_t> indexMap = nullptr;
    indexResource_->Map(0, nullptr, indexMap.GetPtrAdress());
    std::copy(indexData.begin(), indexData.end(), indexMap);
    indexResource_->Unmap(0, nullptr);

    indexView_.SizeInBytes = indexSizeInBytes;
    indexView_.Format = DXGI_FORMAT_R16_UINT;
    indexView_.BufferLocation = indexResource_->GetGPUVirtualAddress();


    vertexResource_ = directXDevice->CreateBufferResuorce(vertexSizeInBytes);

    Lamb::SafePtr<Vector4> vertMap = nullptr;
    vertexResource_->Map(0, nullptr, vertMap.GetPtrAdress());
    std::copy(vertexData.begin(), vertexData.end(), vertMap);
    vertexResource_->Unmap(0, nullptr);

    vertexView_.SizeInBytes = vertexSizeInBytes;
    vertexView_.StrideInBytes = static_cast<uint32_t>(sizeof(Vector4));
    vertexView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();

    Lamb::SafePtr heap = CbvSrvUavHeap::GetInstance();
    if (shaderData_) {
        shaderData_.reset();
        heap->ReleaseView(shaderData_->GetHandleUINT());
    }

    shaderData_ = std::make_unique<ConstantBuffer<ShaderData>>();
    atmosphericParams_ = std::make_unique<ConstantBuffer<AtmosphericParams>>();

    heap->BookingHeapPos(2u);
    heap->CreateView(*shaderData_);
    heap->CreateView(*atmosphericParams_);

    CreateGraphicsPipeline();
}

void AirSkyBox::Draw(const Mat4x4& worldMat, const Mat4x4& cameraMat, uint32_t color) {
    (*shaderData_)->worldMat = worldMat;
    (*shaderData_)->viewProjectionMat = cameraMat;
    (*shaderData_)->color = color;

    // コマンドリスト
    ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

    // パイプライン設定
    pipeline_->Use();

    // ライト構造体
    commandlist->SetGraphicsRootDescriptorTable(0, shaderData_->GetHandleGPU());
    commandlist->SetGraphicsRootDescriptorTable(1, atmosphericParams_->GetHandleGPU());

    // 頂点バッファセット
    commandlist->IASetVertexBuffers(0, 1, &vertexView_);
    // インデックスバッファセット
    commandlist->IASetIndexBuffer(&indexView_);
    // ドローコール
    commandlist->DrawIndexedInstanced(kIndexNumber_, 1, 0, 0, 0);
}

void AirSkyBox::SetAtmosphericParams(const AtmosphericParams& atmosphericParams)
{
    **atmosphericParams_ = atmosphericParams;
}

void AirSkyBox::CreateGraphicsPipeline() {
    Shader shader = {};

    ShaderManager* const shaderMaanger = ShaderManager::GetInstance();


    shader.vertex = shaderMaanger->LoadVertexShader("./Resources/Shaders/SkyBoxShader/SkyBox.VS.hlsl");
    shader.pixel = shaderMaanger->LoadPixelShader("./Resources/Shaders/SkyBoxShader/AirSkyBox.PS.hlsl");

    std::array<D3D12_DESCRIPTOR_RANGE, 1> cbvRange = {};
    cbvRange[0].NumDescriptors = 1;
    cbvRange[0].BaseShaderRegister = 0;
    cbvRange[0].OffsetInDescriptorsFromTableStart = D3D12_APPEND_ALIGNED_ELEMENT;
    cbvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    std::array<D3D12_DESCRIPTOR_RANGE, 1> atmosphericParamsRange = {};
    atmosphericParamsRange[0].NumDescriptors = 1;
    atmosphericParamsRange[0].BaseShaderRegister = 1;
    atmosphericParamsRange[0].OffsetInDescriptorsFromTableStart = D3D12_APPEND_ALIGNED_ELEMENT;
    atmosphericParamsRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;


    std::array<D3D12_ROOT_PARAMETER, 2> rootPrams = {};
    rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootPrams[0].DescriptorTable.NumDescriptorRanges = UINT(cbvRange.size());
    rootPrams[0].DescriptorTable.pDescriptorRanges = cbvRange.data();
    rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootPrams[1].DescriptorTable.NumDescriptorRanges = UINT(atmosphericParamsRange.size());
    rootPrams[1].DescriptorTable.pDescriptorRanges = atmosphericParamsRange.data();
    rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


    RootSignature::Desc desc;
    desc.rootParameter = rootPrams.data();
    desc.rootParameterSize = rootPrams.size();
    desc.samplerDeacs.push_back(
        CreateLinearSampler()
    );

    auto pipelineManager = PipelineManager::GetInstance();
    Pipeline::Desc pipelineDesc;
    pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc, true);
    pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
    pipelineDesc.shader = shader;
    pipelineDesc.isDepth = false;
    pipelineDesc.blend[0] = Pipeline::None;
    pipelineDesc.rtvFormtat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    pipelineDesc.solidState = Pipeline::SolidState::Solid;
    pipelineDesc.cullMode = Pipeline::CullMode::Back;
    pipelineDesc.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.numRenderTarget = 1;
    pipelineManager->SetDesc(pipelineDesc);
    pipeline_ = pipelineManager->CreateCubeMap();
    pipelineManager->StateReset();
}
