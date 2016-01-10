#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Camera.h"
#include "XFile.h"

/* ��װ�� */

// ��װ������dx����XFile���ȡX�ļ�
class CGraphics
{
private:
	// d3d����
	IDirect3D9*	m_d3dObject;
	IDirect3DDevice9* m_d3dDevice;

	// �������X�ļ�����
	CXFile* m_entity;
	CCamera* m_camera;	

	// λ�ú���ת
	D3DXVECTOR3 m_entityPosition;
	float m_entityRotY,m_entityRotX,m_entityRotZ;

	bool m_displayControls;
public:
	CGraphics(void);
	~CGraphics(void);

	bool Initialise(HWND hWnd);
	void Update();
	bool LoadXFile(const std::string &filename,int startAnimation = 0);
	CCamera *GetCamera() const;
	void ToggleControlsDisplay() {m_displayControls = !m_displayControls;}
	void ToggleAnimeRun() {if(m_entity) m_entity->ToggleAnimeRun();}

	void NextAnimation();
};
