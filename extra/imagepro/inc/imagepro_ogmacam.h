#ifndef __imagepro_ogmacam_H__
#define __imagepro_ogmacam_H__

#if defined(_WIN32)
#pragma pack(push, 8)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32)
IMAGEPRO_API(HRESULT) imagepro_stitch_pull(HImageproStitch handle, HOgmacam h, int bFeed, void* pImageData, int bits, int rowPitch, OgmacamFrameInfoV2* pInfo);
IMAGEPRO_API(HRESULT) imagepro_stitch_pullV3(HImageproStitch handle, HOgmacam h, int bFeed, void* pImageData, int bits, int rowPitch, OgmacamFrameInfoV3* pInfo);

IMAGEPRO_API(HRESULT) imagepro_edf_pull(HImageproEdf handle, HOgmacam h, int bFeed, void* pImageData, int bits, int rowPitch, OgmacamFrameInfoV2* pInfo);
IMAGEPRO_API(HRESULT) imagepro_edf_pullV3(HImageproEdf handle, HOgmacam h, int bFeed, void* pImageData, int bits, int rowPitch, OgmacamFrameInfoV3* pInfo);
#endif

#ifdef __cplusplus
}
#endif

#if defined(_WIN32)
#pragma pack(pop)
#endif

#endif
