#include "Sprite.h"
#include<d3dcompiler.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3dcompiler.lib")

using namespace Microsoft::WRL;


//�f�t�H���g�e�N�X�`���i�[�f�B���N�g��
std::string Sprite::kDefaultTextureDirectoryPath = "Resources/";

void Sprite::Initialize(DirectXCommon*dxCommon_ ,int window_width, int window_height)
{

	//���_�f�[�^
	Vertex vertices[] = {
		{{  0.0f,size_.y,0.0f},{0.0f,1.0f,}},//����  �C���f�b�N�X0
		{{  0.0f,  0.0f,0.0f},{0.0f,0.0f,}},//����  �C���f�b�N�X1
		{{size_.x,size_.y,0.0f},{1.0f,1.0f,}},//�E��  �C���f�b�N�X2
		{{size_.x,0.0f,0.0f},{1.0f,0.0f,}},//�E��  �C���f�b�N�X3
	};

	//���_�f�[�^�̑S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	//���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};        //�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;  //GPU�ւ̓]���p

	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB; //���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//���_�o�b�t�@�̍쐬
	result = dxCommon_->GetDevice()->CreateCommittedResource
	(
		&heapProp,  //�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,   //���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff_)
	);
	assert(SUCCEEDED(result));

	//GPU��̃o�b�t�@�ɑΉ��������z������(���C����������)���擾
	Vertex* vertMap = nullptr;
	result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));

	//�O���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];   //���W���R�s�[
	}

	//�q���������
	vertBuff_->Unmap(0, nullptr);

	//GPU���z�A�h���X
	vbView.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	//���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	//���_����̃f�[�^�T�C�Y
	vbView.StrideInBytes = sizeof(vertices[0]);

	//CBV,SRV,UAV��1���̃T�C�Y���擾
	UINT descriptorSize = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ComPtr < ID3DBlob> vsBlob;    //���_�V�F�[�_�I�u�W�F�N�g
	ComPtr < ID3DBlob> psBlob;    //�s�N�Z���V�F�[�_�I�u�W�F�N�g
	ComPtr < ID3DBlob> errorBlob; //�G���[�I�u�W�F�N�g

	//�摜�C���[�W�f�[�^�z��
	//XMFLOAT4* imageData = new XMFLOAT4[imageDataCount];

	//�S�s�N�Z���̐F��������
	//for (size_t i = 0; i < imageDataCount; i++)
	//{
	//	imageData[i].x = 0.0f; // R
	//	imageData[i].y = 1.0f; // G
	//	imageData[i].z = 0.0f; // B
	//	imageData[i].w = 1.0f; // A
	//}

	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	//�ݒ�����Ƃ�SRV�p�f�X�N���v�^�q�[�v�𐶐�
	result = dxCommon_->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(result));

	////�n���h����1�i�߂�iSRV�̈ʒu�j
	//srvHandle.ptr += descriptorSize * 1;

	////CBV(�R���X�^���g�o�b�t�@�r���[)�̐ݒ�
	//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	////cbvDesc�̒l�ݒ�i�ȗ��j
	////�萔�o�b�t�@�r���[����
	//dxCommon_->GetDevice()->CreateConstantBufferView(&cbvDesc, srvHandle);

	//�萔�o�b�t�@�p�f�[�^�\����(�}�e���A��)
	struct ConstBufferDataMaterial {
		XMFLOAT4 color;//�F(RGBA)
	};

	//�萔�o�b�t�@�p�f�[�^�\����(3D�ϊ��s��)
	struct ConstBufferDataTransform {
		XMMATRIX mat; //3D�ϊ��s��
	};

	//�}�e���A��//

	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;  //GPU�ւ̓]���p
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;  //256�o�C�g�A���C�������g
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//�萔�o�b�t�@�̐���
	result = dxCommon_->GetDevice()->CreateCommittedResource(
		&cbHeapProp,//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial_));
	assert(SUCCEEDED(result));

	//�萔�o�b�t�@�̃}�b�s���O
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial_->Map(0, nullptr, (void**)&constMapMaterial);//�}�b�s���O
	assert(SUCCEEDED(result));

	//�l���������ނƎ����I�ɓ]�������
	constMapMaterial->color = XMFLOAT4(1, 0, 0, 0.5f);  //RGBA�Ŕ������̐�

	////3D�ϊ��s��////
	ConstBufferDataTransform* constMapTransform = nullptr; //�萔�o�b�t�@�̃}�b�s���O�p�|�C���^

	{
		//�q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES cbHeapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;		//GPU�ւ̓]���p
		//���\�[�X�ݒ�
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff; //256�o�C�g�A���C�������g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		//�萔�o�b�t�@�̐���
		result = dxCommon_->GetDevice()->CreateCommittedResource(
			&cbHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuffTransform_));
		assert(SUCCEEDED(result));

		//�萔�o�b�t�@�̃}�b�s���O
		result = constBuffTransform_->Map(0, nullptr, (void**)&constMapTransform);  //�}�b�s���O
		assert(SUCCEEDED(result));

		//�P�ʍs�����
		constMapTransform->mat = XMMatrixIdentity();

		//���S���W��ϊ�
		constMapTransform->mat.r[0].m128_f32[0] = 2.0f / window_width;
		constMapTransform->mat.r[1].m128_f32[1] = -2.0f / window_height;

		//���S���W������ɂ��炷
		constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
		constMapTransform->mat.r[3].m128_f32[1] = 1.0f;

	}

		//�f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;               //��x�̕`��Ɏg���e�N�X�`����1���Ȃ̂�1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;           //�e�N�X�`�����W�X�^0��
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	//�萔�o�b�t�@0��
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor.ShaderRegister = 0;
	rootParams[0].Descriptor.RegisterSpace = 0;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//�e�N�X�`�����W�X�^0��
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  //���
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;        //�f�X�N���v�^�����W
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;                     //�f�t�H���g�l
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;              //�S�ẴV�F�[�_���猩����
	//�萔�o�b�t�@1��
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //���
	rootParams[2].Descriptor.ShaderRegister = 1;                   //�f�X�N���v�^�����W
	rootParams[2].Descriptor.RegisterSpace = 0;                    //�f�t�H���g�l
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  //�S�ẴV�F�[�_���猩����

	//�e�N�X�`���T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���J��Ԃ�(�^�C�����O)
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //�c�J��Ԃ�(�^�C�����O)
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���s�J��Ԃ�(�^�C�����O)
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //�S�ă��j�A�⊮
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;                                              //�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //�s�N�Z���V�F�[�_��̂ݎg�p�\


	//���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile
	(
		L"Resources/shaders/SpriteVS.hlsl",//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�\�ɂ���
		"main", "vs_5_0",  //�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�O�p�w��
		0,
		&vsBlob, &errorBlob
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//�s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile
	(
		L"Resources/shaders/SpritePS.hlsl",//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�\�ɂ���
		"main", "ps_5_0",  //�G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�O�p�w��
		0,
		&psBlob, &errorBlob
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			//xyz���W(1�s�ŏ������ق������₷��)
		   "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		   D3D12_APPEND_ALIGNED_ELEMENT,
		   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{
			//uv���W(1�s�ŏ������ق������₷��)
		   "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
		   D3D12_APPEND_ALIGNED_ELEMENT,
		   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	//�O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	//�V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	//�T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//���X�^���C�U�̐ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;   //�J�����O���Ȃ�
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;  //�|���S�����h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true;            //�[�x�N���b�s���O��L����

	//�u�����h�X�e�[�g
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//���ʐݒ�(�A���t�@�l)	
	blenddesc.BlendEnable = true; //�u�����h��L���ɂ���	
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; //���Z	
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE; //�\�[�X�̒l��100% �g��	
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO; //�f�X�g�̒l�� 0% �g��	

	//����������	
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD; //���Z	
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA; //�\�[�X�̃A���t�@�l	
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; //1.0f-�\�[�X�̃A���t�@�l

	//���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	//�}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//���̑��̐ݒ�
	pipelineDesc.NumRenderTargets = 1;  //�`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;  //1�s�N�Z���ɂ�1��T���v�����O

	//���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{ };
	rootSignatureDesc.Flags = 
	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams;//���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams); //���[�g�p�����[�^��
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//���[�g�V�O�l�`���̃V���A���C�Y
	ComPtr<ID3DBlob> rootSigBlob_;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob_, &errorBlob);
	assert(SUCCEEDED(result));
	result = dxCommon_->GetDevice()->CreateRootSignature(0, rootSigBlob_->GetBufferPointer(), rootSigBlob_->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(result));
	//rootSigBlob_->Release();
	//�p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature_.Get();

	//�p�C�v���C���X�e�[�g�𐶐�
	result = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(result));
}

void Sprite::Update()
{
	//XMMATRIX matTrans; //���s�ړ��s��
	//matTrans = XMMatrixTranslation(position_.x, position_.y, 0.0f);//���s�ړ�
}

void Sprite::LoadTexture(uint32_t index, const wchar_t* fileName, DirectXCommon* dxCommon_)
{

	//�摜�ǂݍ���
	TexMetadata metadata{};
	ScratchImage scratchImg{};
	//WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile
	(
		fileName,
		WIC_FLAGS_NONE,
		&metadata, scratchImg
	);

	ScratchImage mipChain{};

	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	//�ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty =
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;  //��
	textureResourceDesc.Height = (UINT)metadata.height;//����
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;

	//�e�N�X�`���o�b�t�@�̐���
	result = dxCommon_->GetDevice()->CreateCommittedResource
	(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureBuffers_[index])
	);

	//�e�N�X�`���o�b�t�@�ւ̉摜�f�[�^�]��
	for (size_t i = 0; i < metadata.mipLevels; i++)
	{
		//�~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = textureBuffers_[index]->WriteToSubresource
		(
			(UINT)i,
			nullptr,			   //�S�̈�փR�s�[
			img->pixels,		   //���f�[�^�A�h���X
			(UINT)img->rowPitch,   //1���C���T�C�Y
			(UINT)img->slicePitch  //1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}

	//�V�F�[�_�[���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};//�ݒ�\����
	D3D12_RESOURCE_DESC resDesc = textureBuffers_[index]->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;

	//SRV�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();

	//�n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�쐬
	dxCommon_->GetDevice()->CreateShaderResourceView(textureBuffers_[index].Get(),
		&srvDesc, srvHandle);
}

void Sprite::SetTextureCommands(uint32_t index, DirectXCommon* dxCommon_)
{
	//�p�C�v���C���X�e�[�g�ƃ��[�g�V�O�l�`���̐ݒ�R�}���h
	dxCommon_->GetCommandList()->SetPipelineState(pipelineState_.Get());
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//�v���~�e�B�u�`��̐ݒ�R�}���h
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//�f�X�N���v�^�q�[�v�̃Z�b�g
	ID3D12DescriptorHeap* srvHeapTemp;
	srvHeapTemp = srvHeap_.Get();
	//SRV�q�[�v�̐ݒ�R�}���h
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, &srvHeapTemp);
	//SRV�q�[�v�̐擪�n���h�����擾(SRV���w���Ă���͂�)
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle =
		srvHeap_->GetGPUDescriptorHandleForHeapStart();
	//SRV�q�[�v�̐擪�ɂ���SRV�����[�g�p�����[�^1�Ԃɐݒ�
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
}

void Sprite::Draw(DirectXCommon* dxCommon_)
{
	Update();

	SetTextureCommands(textureIndex_, dxCommon_);

	//���_�o�b�t�@�r���[�̐ݒ�R�}���h
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView);
	//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, constBuffMaterial_->GetGPUVirtualAddress());
	//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(2, constBuffTransform_->GetGPUVirtualAddress());
	//�`��R�}���h
	dxCommon_->GetCommandList()->DrawInstanced(4, 1, 0, 0);  //�S�Ă̒��_���g���ĕ`��
}