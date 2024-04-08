#ifndef __imagepro_uvcham_H__
#define __imagepro_uvcham_H__

#if defined(_WIN32)
#pragma pack(push, 8)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32)
IMAGEPRO_API(HRESULT) imagepro_stitch_pullham(HImageproStitch handle, HUvcham h, int bFeed, int width, int height, void* pFrameBuffer);
IMAGEPRO_API(HRESULT) imagepro_edf_pullham(HImageproEdf handle, HUvcham h, int bFeed, int width, int height, void* pFrameBuffer);
#endif

#ifdef __cplusplus
}
#endif

#if defined(_WIN32)
#pragma pack(pop)
#endif

#endif
