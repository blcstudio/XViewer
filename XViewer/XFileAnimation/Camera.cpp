#include "Camera.h"

/* 摄像机类 */

CCamera::CCamera() : m_position(0,0,0),m_rotY(0),m_rotX(0),m_rotZ(0) {
	// 默认值
	m_up = D3DXVECTOR3(0.0f,1.0f,0.0f);
	m_look = D3DXVECTOR3(0.0f,0.0f,1.0f);
	m_right = D3DXVECTOR3(1.0f,0.0f,0.0f);
}

CCamera::CCamera(D3DXVECTOR3 startPos) : m_position(startPos),m_rotY(0),m_rotX(0),m_rotZ(0) {
	// 默认值
	m_up = D3DXVECTOR3(0.0f,1.0f,0.0f);
	m_look = D3DXVECTOR3(0.0f,0.0f,1.0f);
	m_right = D3DXVECTOR3(1.0f,0.0f,0.0f);
}

// setter
void CCamera::SetRotate(float _rotY,float _rotX, float _rotZ) {
	m_rotY = _rotY;
	m_rotX = _rotX;
	m_rotZ = _rotZ;
}

// 计算矩阵
void CCamera::CalculateViewMatrix(D3DXMATRIX *viewMatrix) {
	// 默认值
	m_up = D3DXVECTOR3(0.0f,1.0f,0.0f);
	m_look = D3DXVECTOR3(0.0f,0.0f,1.0f);
	m_right = D3DXVECTOR3(1.0f,0.0f,0.0f);

	// y方向
	D3DXMATRIX yMatrix;
	D3DXMatrixRotationAxis(&yMatrix, &m_up, m_rotY);
	D3DXVec3TransformCoord(&m_look, &m_look, &yMatrix); 
	D3DXVec3TransformCoord(&m_right, &m_right, &yMatrix); 
	
	// x方向
	D3DXMATRIX xMatrix;
	D3DXMatrixRotationAxis(&xMatrix, &m_right, m_rotX);
	D3DXVec3TransformCoord(&m_look, &m_look, &xMatrix); 
	D3DXVec3TransformCoord(&m_up, &m_up, &xMatrix); 
		
	// z方向
	D3DXMATRIX zMatrix;
	D3DXMatrixRotationAxis(&zMatrix, &m_look, m_rotZ);
	D3DXVec3TransformCoord(&m_right, &m_right, &zMatrix); 
	D3DXVec3TransformCoord(&m_up, &m_up, &zMatrix); 
	
	D3DXMatrixIdentity(viewMatrix);

	viewMatrix->_11 = m_right.x; viewMatrix->_12 = m_up.x; viewMatrix->_13 = m_look.x;
	viewMatrix->_21 = m_right.y; viewMatrix->_22 = m_up.y; viewMatrix->_23 = m_look.y;
	viewMatrix->_31 = m_right.z; viewMatrix->_32 = m_up.z; viewMatrix->_33 = m_look.z;
	
	viewMatrix->_41 = - D3DXVec3Dot( &m_position,&m_right); 
	viewMatrix->_42 = - D3DXVec3Dot( &m_position,&m_up);
	viewMatrix->_43 = - D3DXVec3Dot( &m_position,&m_look);
}

// 把角度计算成0-360
float CCamera::RestrictAngleTo360Range(float angle) const
{
	while(angle>2*D3DX_PI)
		angle-= 2*D3DX_PI;

	while(angle<0)
		angle+= 2*D3DX_PI;

	return angle;
}