#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <string>

#include "MeshStructures.h"

/* X�ļ��� */

class CXFile
{
private:
	LPDIRECT3DDEVICE9 m_d3dDevice;

	// �������
	LPD3DXFRAME                 m_frameRoot;
    LPD3DXANIMATIONCONTROLLER   m_animController;
	D3DXMESHCONTAINER_EXTENDED* m_firstMesh;
	unsigned int m_currentAnimationSet;	
	unsigned int m_numAnimationSets;
	unsigned int m_currentTrack;
	float m_currentTime;
	float m_speedAdjust;
	bool m_aimeRun;

	// �������
	D3DXMATRIX *m_boneMatrices;
	int m_maxBones;
	
	// 3D�ռ��Χ��
	D3DXVECTOR3 m_sphereCentre;
	float m_sphereRadius;

	std::string m_filename;

	void UpdateFrameMatrices(const D3DXFRAME *frameBase, const D3DXMATRIX *parentMatrix);
	void DrawFrame(LPD3DXFRAME frame) const;
	void DrawMeshContainer(LPD3DXMESHCONTAINER meshContainerBase, LPD3DXFRAME frameBase) const;
	void SetupBoneMatrices(D3DXFRAME_EXTENDED *pFrame, LPD3DXMATRIX pParentMatrix);	
	bool InPauseList(LPSTR name);
public:
	CXFile(LPDIRECT3DDEVICE9 d3dDevice);
	~CXFile(void);

	bool Load(const std::string &filename);
	void FrameMove(float elapsedTime,const D3DXMATRIX *matWorld);
	
	void Render() const;
	void SetAnimationSet(unsigned int index);

	void NextAnimation();

	D3DXVECTOR3 GetInitialCameraPosition() const;
	unsigned int GetCurrentAnimationSet() const {return m_currentAnimationSet;}
	std::string GetAnimationSetName(unsigned int index);
	std::string GetFilename() const {return m_filename;}

	void ToggleAnimeRun() {m_aimeRun = !m_aimeRun;}
};
