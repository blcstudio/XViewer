#include "XFile.h"
#include "Utility.h"
#include "MeshHierarchy.h"
#include <winnt.h>

/* X文件类 */

// 动画切换间隔时间
const float cMoveTransitionTime = 0.25f;

// 构造函数与析构
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

// 读取X文件
bool CXFile::Load(const std::string &filename) {
	// 读取用，临时变量
	CMeshHierarchy *memoryAllocator = new CMeshHierarchy;

	// 获取当前目录
	std::string currentDirectory = CUtility::GetTheCurrentDirectory();

	std::string xfilePath;
	CUtility::SplitPath(filename,&xfilePath,&m_filename);

	SetCurrentDirectory(xfilePath.c_str());

	// 用D3DXLoadMeshHierarchyFromX读取X文件
	HRESULT hr = D3DXLoadMeshHierarchyFromX(filename.c_str(), D3DXMESH_MANAGED, m_d3dDevice, 
		memoryAllocator, NULL, &m_frameRoot, &m_animController);

	// 释放不用的临时变量
	delete memoryAllocator;
	memoryAllocator = NULL;	//野指针

	SetCurrentDirectory(currentDirectory.c_str());
	
	if (CUtility::FailedHr(hr))
		return false; 

	// 如果有动画，保存动画集合
	if(m_animController)
		m_numAnimationSets = m_animController->GetMaxNumAnimationSets();

	// 如果有骨骼帧
	if(m_frameRoot) {
		// 设置
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)m_frameRoot, NULL);

		// 创建骨骼矩阵
		m_boneMatrices  = new D3DXMATRIX[m_maxBones];
		// 置空
		ZeroMemory(m_boneMatrices, sizeof(D3DXMATRIX)*m_maxBones);

		// 计算包围球
		D3DXFrameCalculateBoundingSphere(m_frameRoot, &m_sphereCentre, &m_sphereRadius);
	}

	return true;
}

// 根据包围球获取初始摄像机位置
D3DXVECTOR3 CXFile::GetInitialCameraPosition() const
{
	D3DXVECTOR3 cameraPos(0.0f,m_sphereCentre.y,-(m_sphereRadius*3));
	return cameraPos;
}

// 设置骨骼矩阵
void CXFile::SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix) {
	// Cast to our extended structure first
	D3DXMESHCONTAINER_EXTENDED* pMesh = (D3DXMESHCONTAINER_EXTENDED*)pFrame->pMeshContainer;

	// 如果有网格
	if(pMesh) {
		// 记录最初的网格
		if(!m_firstMesh)
			m_firstMesh = pMesh;
		
		// 如果有蒙皮
		if(pMesh->pSkinInfo && pMesh->MeshData.pMesh)
		{
			// 拷贝一份蒙皮
			D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];
			if (FAILED(pMesh->MeshData.pMesh->GetDeclaration(Declaration)))
				return;

			pMesh->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED, 
				Declaration, m_d3dDevice, 
				&pMesh->exSkinMesh);

			m_maxBones = max(m_maxBones,(int)pMesh->pSkinInfo->GetNumBones());

			// 分别处理每根骨骼
			for (unsigned int i = 0; i < pMesh->pSkinInfo->GetNumBones(); i++)
			{   
				// 找到包含指定骨骼的帧
				D3DXFRAME_EXTENDED* pTempFrame = (D3DXFRAME_EXTENDED*)D3DXFrameFind(m_frameRoot, 
						pMesh->pSkinInfo->GetBoneName(i));

				// 处理帧
				pMesh->exFrameCombinedMatrixPointer[i] = &pTempFrame->exCombinedTransformationMatrix;
			}

		}
	}

	// 递归处理兄弟帧
	if(pFrame->pFrameSibling)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameSibling, pParentMatrix);

	// 递归子帧
	if(pFrame->pFrameFirstChild)
		SetupBoneMatrices((D3DXFRAME_EXTENDED*)pFrame->pFrameFirstChild, &pFrame->exCombinedTransformationMatrix);
}


// 根据时间更新帧
void CXFile::FrameMove(float elapsedTime,const D3DXMATRIX *matWorld) {
	// 调整当前动画的时间点
    if (m_animController != NULL)
        m_animController->AdvanceTime(elapsedTime, NULL);

	m_currentTime+= elapsedTime;

	// 更新矩阵
    UpdateFrameMatrices(m_frameRoot, matWorld);

	// 如果有蒙皮
	D3DXMESHCONTAINER_EXTENDED* pMesh = m_firstMesh;
	if(pMesh && pMesh->pSkinInfo) {
		// 更新顶点
		unsigned int Bones = pMesh->pSkinInfo->GetNumBones();

		// 计算骨骼矩阵
		for (unsigned int i = 0; i < Bones; ++i)
			D3DXMatrixMultiply(&m_boneMatrices[i],&pMesh->exBoneOffsets[i], pMesh->exFrameCombinedMatrixPointer[i]);

		// 锁定顶点缓冲区
		void *srcPtr = 0;
		pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&srcPtr);

		void *destPtr = 0;
		pMesh->exSkinMesh->LockVertexBuffer(0, (void**)&destPtr);

		// 根据新的骨骼矩阵更新蒙皮网格
		pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices, NULL, srcPtr, destPtr);

		// 解锁
		pMesh->exSkinMesh->UnlockVertexBuffer();
		pMesh->MeshData.pMesh->UnlockVertexBuffer();
	}
}

// 判断骨骼是否在暂停列表里
bool CXFile::InPauseList(LPSTR name) {
	if (strcmp(name, "TORSO") == 0) return true;
	//else if (strcmp(name, "Neck") == 0) return true;
	//else if (strcmp(name, "") == 0) return true;
	else return false;
}

// 更新骨骼矩阵
void CXFile::UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix) {
    D3DXFRAME_EXTENDED *currentFrame = (D3DXFRAME_EXTENDED*)frameBase;

	// 如果按下了暂停键，并且在这块骨骼暂停列表里，就不计算这块骨骼的矩阵
	if (!m_aimeRun && currentFrame->Name != NULL && InPauseList(currentFrame->Name)) {
		if (parentMatrix != NULL)
			D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, D3DXMatrixIdentity(&currentFrame->TransformationMatrix), parentMatrix);
	} else {
		// 计算骨骼的变化，如果有父节点要乘父节点的变换矩阵
		if (parentMatrix != NULL)
			D3DXMatrixMultiply(&currentFrame->exCombinedTransformationMatrix, &currentFrame->TransformationMatrix, parentMatrix);
		else
			currentFrame->exCombinedTransformationMatrix = currentFrame->TransformationMatrix;
	}

	// 递归兄弟帧
    if (currentFrame->pFrameSibling != NULL)
        UpdateFrameMatrices(currentFrame->pFrameSibling, parentMatrix);

	// 递归子帧
    if (currentFrame->pFrameFirstChild != NULL)
        UpdateFrameMatrices(currentFrame->pFrameFirstChild, &currentFrame->exCombinedTransformationMatrix);
}

// 渲染网格
void CXFile::Render() const
{
	if (m_frameRoot)
		DrawFrame(m_frameRoot);
}

// 渲染帧
void CXFile::DrawFrame(LPD3DXFRAME frame) const
{
	// 遍历所有网格容器
    LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;
    while (meshContainer)
    {
		// 绘制
        DrawMeshContainer(meshContainer, frame);
        meshContainer = meshContainer->pNextMeshContainer;
    }

	// 递归兄弟帧
    if (frame->pFrameSibling != NULL)
        DrawFrame(frame->pFrameSibling);

    // 递归子帧
	if (frame->pFrameFirstChild != NULL)
        DrawFrame(frame->pFrameFirstChild);
}

// 绘制网格容器
void CXFile::DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase) const
{
	D3DXFRAME_EXTENDED *frame = (D3DXFRAME_EXTENDED*)frameBase;
	D3DXMESHCONTAINER_EXTENDED *meshContainer = (D3DXMESHCONTAINER_EXTENDED*)meshContainerBase;
	
	// 设置移动
    m_d3dDevice->SetTransform(D3DTS_WORLD, &frame->exCombinedTransformationMatrix);

	// 遍历材质
    for (unsigned int iMaterial = 0; iMaterial < meshContainer->NumMaterials; iMaterial++)
    {
		// 设置材质
		m_d3dDevice->SetMaterial( &meshContainer->exMaterials[iMaterial] );
		m_d3dDevice->SetTexture( 0, meshContainer->exTextures[iMaterial] );

		// 绘制蒙皮网格
		LPD3DXMESH pDrawMesh = (meshContainer->pSkinInfo) ? meshContainer->exSkinMesh: meshContainer->MeshData.pMesh;
        pDrawMesh->DrawSubset(iMaterial);
    }
}

// 设置动画集
void CXFile::SetAnimationSet(unsigned int index) {
	// 搬砖来的，并不懂
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

// 下一个动画
void CXFile::NextAnimation() {	
	unsigned int newAnimationSet = m_currentAnimationSet+1;
	if (newAnimationSet >= m_numAnimationSets)
		newAnimationSet = 0;

	SetAnimationSet(newAnimationSet);
}

//获取动画集名称
std::string CXFile::GetAnimationSetName(unsigned int index) {
	if (index >= m_numAnimationSets)
		return "Error: No set exists";

	LPD3DXANIMATIONSET set;
	m_animController->GetAnimationSet(m_currentAnimationSet, &set );

	std::string nameString(set->GetName());

	set->Release();

	return nameString;
}
