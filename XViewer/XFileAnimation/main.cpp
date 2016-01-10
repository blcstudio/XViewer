#include <windows.h>
#include "Graphics.h"
#include "Camera.h"
#include "Utility.h"

/* ����ļ� */

// Grapics ����
CGraphics *gGraphics = NULL;

// �ص�������������
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// ������ƶ����ٶ�
	const float camMoveSpeed = 0.05f;
	// �ж���Ϣ
	switch (message) 
	{
	case WM_KEYDOWN:
		// �жϰ���
		switch(wParam)
		{
			// �ƶ����
			case 'W':
			case VK_UP:
				gGraphics->GetCamera()->MoveForward(camMoveSpeed);
			break;
			case 'S':
			case VK_DOWN:
				gGraphics->GetCamera()->MoveForward(-camMoveSpeed);
			break;
			case 'D':
			case VK_RIGHT:
				gGraphics->GetCamera()->MoveRight(camMoveSpeed);
			break;
			case 'A':
			case VK_LEFT:
				gGraphics->GetCamera()->MoveRight(-camMoveSpeed);
			break;

			// �л�����			
			case VK_SPACE:
				gGraphics->NextAnimation();
			break;
			// ��ͣ���ֹ���
			case 'P':
				gGraphics->ToggleAnimeRun();
				break;
			// �л�����̨���ù���
			case 'C':
				gGraphics->ToggleModel();
				break;

			default:
				break;
		}
		break;
        case WM_DESTROY:
             PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// win32���
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// ע�ᴰ����
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc= (WNDPROC)WndProc;
	wcex.cbClsExtra= 0;
	wcex.cbWndExtra= 0;
	wcex.hInstance= hInstance;
	wcex.hIcon= 0;
	wcex.hCursor= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName= 0;
	wcex.lpszClassName= "MainWindowClass";
	wcex.hIconSm= 0;
	RegisterClassEx(&wcex);

    // ���ڴ�С
	RECT rc;
	SetRect(&rc, 0, 0, 800, 600);  
	AdjustWindowRectEx( &rc, WS_OVERLAPPEDWINDOW,false,0);  

	// ��������
	HWND hWnd = CreateWindow("MainWindowClass", "XViewer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	if (hWnd == 0)
		return 0;

	// ����Graphics���ȫ��ʵ��
	gGraphics = new CGraphics();
	if (!gGraphics->Initialise(hWnd)) {
		// ����ʧ��ֱ���˳�����
		delete gGraphics;
		return 0;
	}

	// ��ȡ.x�ļ�
	gGraphics->LoadXFile(CUtility::GetTheCurrentDirectory()+"/model/bones_all.x",1);

	// ��ʾ����
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// ��Ϣѭ��
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while( msg.message != WM_QUIT ) {
	   if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	   {
		  TranslateMessage( &msg );
		  DispatchMessage( &msg );
	   }
	   else
	   {
		   gGraphics->Update();
	   }  
	}

	// �˳�
	delete gGraphics;
	return (int)msg.wParam;
}

