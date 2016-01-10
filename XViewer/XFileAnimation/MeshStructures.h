#pragma once

struct D3DXMESHCONTAINER_EXTENDED: public D3DXMESHCONTAINER
{
    IDirect3DTexture9**  exTextures;
	D3DMATERIAL9*		 exMaterials;
                                
	ID3DXMesh*           exSkinMesh;
	D3DXMATRIX*			 exBoneOffsets;
	D3DXMATRIX**		 exFrameCombinedMatrixPointer;
};

struct D3DXFRAME_EXTENDED: public D3DXFRAME
{
    D3DXMATRIX exCombinedTransformationMatrix;
};