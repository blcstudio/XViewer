#pragma once

#include <d3dx9.h>

/* 摄像机类 */

class CCamera
{
private:
	D3DXVECTOR3 m_position; // 位置
	float m_rotY;			// y轴旋转
	float m_rotX;			// x轴旋转
	float m_rotZ;			// z轴旋转
	D3DXVECTOR3 m_up,m_look,m_right; // 摄像机对准方向

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

	// 移动位置
	void MoveForward(float amount) {m_position+= m_look*amount;}
	void MoveRight(float amount) {m_position+= m_right*amount;}
	void MoveUp(float amount) {m_position+= m_up*amount;}

};
