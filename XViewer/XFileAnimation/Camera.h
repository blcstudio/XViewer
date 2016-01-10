#pragma once

#include <d3dx9.h>

/* ������� */

class CCamera
{
private:
	D3DXVECTOR3 m_position; // λ��
	float m_rotY;			// y����ת
	float m_rotX;			// x����ת
	float m_rotZ;			// z����ת
	D3DXVECTOR3 m_up,m_look,m_right; // �������׼����

	float RestrictAngleTo360Range(float angle) const;
public:
	CCamera();
	CCamera(D3DXVECTOR3 startPos);
	~CCamera(void){};

	void CalculateViewMatrix(D3DXMATRIX *viewMatrix);

	// setter
	void SetPosition(const D3DXVECTOR3 &pos) {m_position = pos;}
	void SetRotate(float _rotY,float _rotX, float _rotZ);	

	// getter
	float GetRotY() const {return m_rotY;}
	float GetRotX() const {return m_rotX;}
	float GetRotZ() const {return m_rotZ;}
	D3DXVECTOR3 GetPosition() const {return m_position;}	

	// �ƶ�λ��
	void MoveForward(float amount) {m_position+= m_look*amount;}
	void MoveRight(float amount) {m_position+= m_right*amount;}
	void MoveUp(float amount) {m_position+= m_up*amount;}

};
