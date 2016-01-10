#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "meshstructures.h"

// 实现用来创建或者销毁D3DXFRAME对象和网格容器对象的接口
class CMeshHierarchy : public ID3DXAllocateHierarchy
{
public:	
	// 创建D3DXFRAME
    STDMETHOD( CreateFrame )(LPCSTR Name, LPD3DXFRAME *retNewFrame );
	// 销毁D3DXFRAME
    STDMETHOD( DestroyFrame )(LPD3DXFRAME frameToFree );

	// 创建网格容器
    STDMETHOD( CreateMeshContainer )(LPCSTR Name, CONST D3DXMESHDATA * meshData, 
                            CONST D3DXMATERIAL * materials, CONST D3DXEFFECTINSTANCE * effectInstances,
                            DWORD numMaterials, CONST DWORD * adjacency, LPD3DXSKININFO skinInfo, 
                            LPD3DXMESHCONTAINER * retNewMeshContainer );
	// 销毁网格容器
    STDMETHOD( DestroyMeshContainer )(LPD3DXMESHCONTAINER meshContainerToFree );
};