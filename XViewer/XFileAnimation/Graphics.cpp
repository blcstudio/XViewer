#include "Graphics.h"
#include "Utility.h"
#include "XFile.h"
#include "Camera.h"

/* ��װ�� */

// ���������
// MARK: ����ģ��λ��
/* tiny.x �õĲ���
CGraphics::CGraphics(void) : m_d3dObject(NULL),m_d3dDevice(NULL),m_entity(NULL),m_displayControls(true), 
	m_entityPosition(0,-150,0),m_entityRotY(0),m_entityRotX(-1.5707964f/2),m_entityRotZ(0)
*/
CGraphics::CGraphics(void) : m_d3dObject(0),m_d3dDevice(0),m_entity(0),m_displayControls(true), 
	m_entityPosition(0,0,0),m_entityRotY(0),m_entityRotX(0),m_entityRotZ(0)
{ m_camera = new CCamera(); }

CGraphics::~CGraphics(void) {
	if (m_entity) {
		delete m_entity;
		m_entity = 0;
	}
	if (m_camera) {
		delete m_camera;
		m_camera = 0;
	}
	if (m_d3dDevice) {
		m_d3dDevice->Release();
		m_d3dDevice = 0;
	}
	if (m_d3dObject) {
		m_d3dObject->Release();
		m_d3dObject = 0;
	}
}

// ��ʼ��������dx
bool CGraphics::Initialise(HWND hWnd) {
	m_d3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_d3dObject) {
		CUtility::DebugString("Could not create Direct3D object\n");
		return false;
	}

	D3DPRESENT_PARAMETERS presParams;
	memset(&presParams,0,sizeof(presParams));
	presParams.Windowed = TRUE;
	presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presParams.BackBufferFormat = D3DFMT_UNKNOWN;
	presParams.BackBufferCount = 1;
	presParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	presParams.EnableAutoDepthStencil = TRUE;
	presParams.AutoDepthStencilFormat = D3DFMT_D16;

	HRESULT hr = m_d3dObject->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &presParams, &m_d3dDevice);
	if (CUtility::FailedHr(hr)) {
		// ��֧��
		hr = m_d3dObject->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presParams, &m_d3dDevice);
		if (CUtility::FailedHr(hr))
		{
			CUtility::DebugString("Could not create Direct3D device\n");
			return false;
		}
	}

	m_d3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

	// ����d3d�豸
	m_d3dDevice->SetRenderState( D3DRS_LIGHTING,         TRUE );
	m_d3dDevice->SetRenderState( D3DRS_DITHERENABLE,     TRUE );
	m_d3dDevice->SetRenderState( D3DRS_SPECULARENABLE,	  FALSE );
	m_d3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
	m_d3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
	m_d3dDevice->SetRenderState( D3DRS_AMBIENT,          0x99999999 );
	m_d3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_d3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_d3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

	// ͶӰ����
	D3DXMATRIX matProj;	
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 800.0f/600.0f, 1.0f, 5000.0f );
	m_d3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	// �ƹ�
	m_d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

	// һ��ƽ�й�Դ
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;
	light.Diffuse.a  = 1.0f;
	light.Range      = 1000.0f;

	D3DXVECTOR3 vecDir;
	vecDir = D3DXVECTOR3(0.0f,-0.3f,0.5f);
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	m_d3dDevice->SetLight( 0, &light );
	m_d3dDevice->LightEnable( 0, TRUE );

	// ������Դ
	m_d3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(80,80,80));
	
	return true;
}

// getter
CCamera *CGraphics::GetCamera() const
{
	assert(m_camera);
	return m_camera;
}

// �����ø��º���
void CGraphics::Update() {
	static DWORD lastTime = timeGetTime();

	// ������������þ���
	D3DXMATRIX matView;
	m_camera->CalculateViewMatrix(&matView);
	m_d3dDevice->SetTransform( D3DTS_VIEW, &matView );
		
	// ��ջ�����
	HRESULT hr = m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0);
	if (CUtility::FailedHr(hr))
		return;

	// ��Ⱦ����
	if(SUCCEEDED(m_d3dDevice->BeginScene())) {
		// XFile�������
		if (m_entity)
		{
			float timeElapsed = 0.001f*(timeGetTime()-lastTime);
			lastTime = timeGetTime();

			// ������ת
			D3DXMATRIX matRotX,matRotY,matRotZ;
			D3DXMatrixRotationX( &matRotX, m_entityRotX );
			D3DXMatrixRotationY( &matRotY, m_entityRotY );  
			D3DXMatrixRotationZ( &matRotZ, m_entityRotZ ); 
			// �����ƶ�
			D3DXMATRIX matTrans;
			D3DXMatrixTranslation(&matTrans,m_entityPosition.x,m_entityPosition.y,m_entityPosition.z);
			// �����������
			D3DXMATRIX matWorld = (matRotX*matRotY*matRotZ)*matTrans;			

			// ����ʱ�����֡
			m_entity->FrameMove(timeElapsed,&matWorld);
			m_entity->Render();
		}

		m_d3dDevice->EndScene();
		m_d3dDevice->Present( NULL, NULL, NULL, NULL ); 
	}
}

// ��ȡX�ļ�
bool CGraphics::LoadXFile(const std::string &filename,int startAnimation) {
	// ����Ѵ���XFile����ɾ��
	if (m_entity) {
		delete m_entity;
		m_entity = 0;
	}

	// ����XFile����
	m_entity = new CXFile(m_d3dDevice);
	if (!m_entity->Load(filename)) {
		delete m_entity;
		m_entity = 0;
		return false;
	}

	m_entity->SetAnimationSet(startAnimation);

	// ���������
	D3DXVECTOR3 pos = m_entity->GetInitialCameraPosition();
	m_camera->SetPosition(pos);
	m_camera->SetRotate(0,0,0);

	return true;
}

// ��һ������
void CGraphics::NextAnimation() {
	if (m_entity)
		m_entity->NextAnimation();
}
