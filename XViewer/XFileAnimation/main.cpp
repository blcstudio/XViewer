#include <windows.h>
#include "Graphics.h"
#include "Camera.h"
#include "Utility.h"

/* 入口文件 */

// Grapics 对象
CGraphics *gGraphics = NULL;

// 回掉函数，处理按键
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	// 摄像机移动的速度
	const float camMoveSpeed = 0.05f;
	// 判断消息
	switch (message) 
	{
	case WM_KEYDOWN:
		// 判断按键
		switch(wParam)
		{
			// 移动相机
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

			// 切换动画			
			case VK_SPACE:
				gGraphics->NextAnimation();
			break;
			// 暂停部分骨骼
			case 'P':
				gGraphics->ToggleAnimeRun();
				break;
			// 切换到后台备用骨骼
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

// win32入口
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 注册窗口类
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

    // 窗口大小
	RECT rc;
	SetRect(&rc, 0, 0, 800, 600);  
	AdjustWindowRectEx( &rc, WS_OVERLAPPEDWINDOW,false,0);  

	// 创建窗口
	HWND hWnd = CreateWindow("MainWindowClass", "XViewer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	if (hWnd == 0)
		return 0;

	// 创建Graphics类的全局实例
	gGraphics = new CGraphics();
	if (!gGraphics->Initialise(hWnd)) {
		// 创建失败直接退出程序
		delete gGraphics;
		return 0;
	}

	// 读取.x文件
	gGraphics->LoadXFile(CUtility::GetTheCurrentDirectory()+"/model/bones_all.x",1);

	// 显示窗口
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 消息循环
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

	// 退出
	delete gGraphics;
	return (int)msg.wParam;
}

