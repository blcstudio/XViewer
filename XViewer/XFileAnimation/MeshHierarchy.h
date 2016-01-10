#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "meshstructures.h"

// ʵ������������������D3DXFRAME�����������������Ľӿ�
class CMeshHierarchy : public ID3DXAllocateHierarchy
{
public:	
	// ����D3DXFRAME
    STDMETHOD( CreateFrame )(LPCSTR Name, LPD3DXFRAME *retNewFrame );
	// ����D3DXFRAME
    STDMETHOD( DestroyFrame )(LPD3DXFRAME frameToFree );

	// ������������
    STDMETHOD( CreateMeshContainer )(LPCSTR Name, CONST D3DXMESHDATA * meshData, 
                            CONST D3DXMATERIAL * materials, CONST D3DXEFFECTINSTANCE * effectInstances,
                            DWORD numMaterials, CONST DWORD * adjacency, LPD3DXSKININFO skinInfo, 
                            LPD3DXMESHCONTAINER * retNewMeshContainer );
	// ������������
    STDMETHOD( DestroyMeshContainer )(LPD3DXMESHCONTAINER meshContainerToFree );
};