#include "XFile.h"
#include "Utility.h"
#include "MeshHierarchy.h"
#include <winnt.h>

/* X�ļ��� */

// �����л����ʱ��
const float cMoveTransitionTime = 0.25f;

// ���캯��������
CXFile::CXFile(LPDIRECT3DDEVICE9 d3dDevice) : m_d3dDevice(d3dDevice),m_speedAdjust(1.0f),m_firstMesh(0),
	m_currentTrack(0),m_currentTime(0),m_numAnimationSets(0),m_currentAnimationSet(0),m_maxBones(0),m_sphereRadius(0),
	m_sphereCentre(0,0,0),m_boneMatrices(0),m_aimeRun(true)
{}

CXFile::~CXFile(void) {
	if (m_animController) {
		m_animController->Release();
		m_animController = 0;
	}
	if (m_frameRoot) {
		CMeshHierarchy memoryAllocator;
		D3DXFrameDestroy(m_frameRoot, &memoryAllocator);
		m_frameRoot = 0;
	}
	if (m_boneMatrices) {
		delete []m_boneMatrices;
		m_boneMatrices = 0;
	}
}

// ��ȡX�ļ�
bool CXFile::Load(const std::string &filename) {
	// ��ȡ�ã���ʱ����
	CMeshHierarchy *memoryAllocator = new CMeshHierarchy;

	// ��ȡ��ǰĿ¼
	std::string currentDirectory = CUtility::GetTheCurrentDirectory();

	std::string xfilePath;
	CUtility::SplitPath(filename,&xfilePath,&m_filename);

	SetCurrentDirectory(xfilePath.c_str());

	// ��D3DXLoadMeshHierarchyFromX��ȡX�ļ�
	HRESULT hr = D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, m_d3dDevice, 
		memoryAllocator, NULL, &m_frameRoot, &m_animController);

	// �ͷŲ��õ���ʱ����
	delete memoryAllocator;
	memoryAllocator = NULL;	//Ұָ��

	SetCurrentDirectory(currentDirectory.c_str());
	
	if (CUtility::FailedHr(hr))
		return false; 

	// ����ж��������涯������
	if(m_animController)
		m_numAnimationSets = m_animController->GetMaxNumAnimationSets();

	// ����й���֡
	if(m_frameRoot) {
		// ����
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)m_frameRoot, NULL);

		// ������������
		m_boneMatrices  = new D3DXMATRIX[m_maxBones];
		// �ÿ�
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		// �����Χ��
		D3DXFrameCalculateBoundingSphere(m_frameRoot, &m_sphereCentre, &m_sphereRadius);
	}

	return true;
}

// ���ݰ�Χ���ȡ��ʼ�����λ��
D3DXVECTOR3 CXFile::GetInitialCameraPosition() const
{
	D3DXVECTOR3 cameraPos(0.0f,m_sphereCentre.y,-(m_sphereRadius*3));
	return cameraPos;
}

// ���ù�������
void CXFile::SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix) {
	// Cast to our extended structure first
	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)pFrame->pMeshContainer;

	// ���������
	if(pMesh) {
		// ��¼���������
		if(!m_firstMesh)
			m_firstMesh = pMesh;
		
		// �������Ƥ
		if(pMesh->pSkinInfo && pMesh->MeshData.pMesh)
		{
			// ����һ����Ƥ
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
			if (FAILED(pMesh->MeshData.pMesh->GetDeclaration(Declaration)))
				return;

			pMesh->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, 
				Declaration, m_d3dDevice, 
				&pMesh->exSkinMesh);

			m_maxBones = max(m_maxBones,(int)pMesh->pSkinInfo->GetNumBones());

			// �ֱ���ÿ������
			for (unsigned int i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
			{   
				// �ҵ�����ָ��������֡
				D3DXFRAME_EXTENDED* pTempFrame = (D3DXFRAME_EXTENDED*)D3DXFrameFind(m_frameRoot, 
						pMesh->pSkinInfo->GetBoneName(i));

				// ����֡
				pMesh->exFrameCombinedMatrixPointer[i] = &pTempFrame->exCombinedTransformationMatrix;
			}

		}
	}

	// �ݹ鴦���ֵ�֡
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameSibling, pParentMatrix);

	// �ݹ���֡
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameFirstChild, &pFrame->exCombinedTransformationMatrix);
}


// ����ʱ�����֡
void CXFile::FrameMove(float elapsedTime,const D3DXMATRIX *matWorld) {
	// ������ǰ������ʱ���
    if (m_animController != NULL)
        m_animController->AdvanceTime(elapsedTime, NULL);

	m_currentTime+= elapsedTime;

	// ���¾���
    UpdateFrameMatrices(m_frameRoot, matWorld);

	// �������Ƥ
	D3DXMESHCONTAINER_EXTENDED* pMesh = m_firstMesh;
	if(pMesh && pMesh->pSkinInfo) {
		// ���¶���
		unsigned int Bones = pMesh->pSkinInfo->GetNumBones();

		// �����������
		for (unsigned int i = 0; i < Bones; ++i)
			D3DXMatrixMultiply(&m_boneMatrices[i],&pMesh->exBoneOffsets[i], pMesh->exFrameCombinedMatrixPointer[i]);

		// �������㻺����
		void *srcPtr = 0;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr = 0;
		pMesh->exSkinMesh->LockVertexBuffer(0, (void**)&destPtr);

		// �����µĹ������������Ƥ����
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		// ����
		pMesh->exSkinMesh->UnlockVertexBuffer();
		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

// �жϹ����Ƿ�����ͣ�б���
bool CXFile::InPauseList(LPSTR name) {
	if (strcmp(name, "TORSO") == 0) return true;
	//else if (strcmp(name, "Neck") == 0) return true;
	//else if (strcmp(name, "") == 0) return true;
	else return false;
}

// ���¹�������
void CXFile::UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix) {
    D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	// �����������ͣ������������������ͣ�б���Ͳ������������ľ���
	if (!m_aimeRun && currentFrame->Name != NULL && InPauseList(currentFrame->Name)) {
		if (parentMatrix != NULL)
			D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, D3DXMatrixIdentity(&currentFrame->TransformationMatrix), parentMatrix);
	} else {
		// ��������ı仯������и��ڵ�Ҫ�˸��ڵ�ı任����
		if (parentMatrix != NULL)
			D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, &currentFrame->TransformationMatrix, parentMatrix);
		else
			currentFrame->exCombinedTransformationMatrix = currentFrame->TransformationMatrix;
	}

	// �ݹ��ֵ�֡
    if (currentFrame->pFrameSibling != NULL)
        UpdateFrameMatrices(currentFrame->pFrameSibling, parentMatrix);

	// �ݹ���֡
    if (currentFrame->pFrameFirstChild != NULL)
        UpdateFrameMatrices(currentFrame->pFrameFirstChild, &currentFrame->exCombinedTransformationMatrix);
}

// ��Ⱦ����
void CXFile::Render() const
{
	if (m_frameRoot)
		DrawFrame(m_frameRoot);
}

// ��Ⱦ֡
void CXFile::DrawFrame(LPD3DXFRAME frame) const
{
	// ����������������
    LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
    while (meshContainer)
    {
		// ����
        DrawMeshContainer(meshContainer, frame);
        meshContainer = meshContainer->pNextMeshContainer;
    }

	// �ݹ��ֵ�֡
    if (frame->pFrameSibling != NULL)
        DrawFrame(frame->pFrameSibling);

    // �ݹ���֡
	if (frame->pFrameFirstChild != NULL)
        DrawFrame(frame->pFrameFirstChild);
}

// ������������
void CXFile::DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase) const
{
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameBase;
	D3DXMESHCONTAINER_EXTENDED *meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	
	// �����ƶ�
    m_d3dDevice->SetTransform(D3DTS_WORLD, &frame->exCombinedTransformationMatrix);

	// ��������
    for (unsigned int iMaterial = 0; iMaterial < meshContainer->NumMaterials; iMaterial++)
    {
		// ���ò���
		m_d3dDevice->SetMaterial( &meshContainer->exMaterials[iMaterial] );
		m_d3dDevice->SetTexture( 0, meshContainer->exTextures[iMaterial] );

		// ������Ƥ����
		LPD3DXMESH pDrawMesh = (meshContainer->pSkinInfo) ? meshContainer->exSkinMesh: meshContainer->MeshData.pMesh;
        pDrawMesh->DrawSubset(iMaterial);
    }
}

// ���ö�����
void CXFile::SetAnimationSet(unsigned int index) {
	// ��ש���ģ�������
	if (index == m_currentAnimationSet)
		return;

	if (index>= m_numAnimationSets)
		index = 0;

	m_currentAnimationSet = index;

	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );	

	DWORD newTrack = ( m_currentTrack == 0 ? 1 : 0 );

	m_animController->SetTrackAnimationSet( newTrack, set );
    set->Release();	

	m_animController->UnkeyAllTrackEvents( m_currentTrack );
    m_animController->UnkeyAllTrackEvents( newTrack );

    m_animController->KeyTrackEnable( m_currentTrack, FALSE, m_currentTime + cMoveTransitionTime );
    m_animController->KeyTrackSpeed( m_currentTrack, 0.0f, m_currentTime, cMoveTransitionTime, D3DXTRANSITION_LINEAR );
    m_animController->KeyTrackWeight( m_currentTrack, 0.0f, m_currentTime, cMoveTransitionTime, D3DXTRANSITION_LINEAR );

    m_animController->SetTrackEnable( newTrack, TRUE );
    m_animController->KeyTrackSpeed( newTrack, 1.0f, m_currentTime, cMoveTransitionTime, D3DXTRANSITION_LINEAR );
    m_animController->KeyTrackWeight( newTrack, 1.0f, m_currentTime, cMoveTransitionTime, D3DXTRANSITION_LINEAR );

    m_currentTrack = newTrack;
}

// ��һ������
void CXFile::NextAnimation() {	
	unsigned int newAnimationSet = m_currentAnimationSet+1;
	if (newAnimationSet >= m_numAnimationSets)
		newAnimationSet = 0;

	SetAnimationSet(newAnimationSet);
}

//��ȡ����������
std::string CXFile::GetAnimationSetName(unsigned int index) {
	if (index >= m_numAnimationSets)
		return "Error: No set exists";

	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	std::string nameString(set->GetName());

	set->Release();

	return nameString;
}
