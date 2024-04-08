#ifndef __imagepro_uvcsam_H__
#define __imagepro_uvcsam_H__

#if defined(_WIN32)
#pragma pack(push, 8)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WIN32)
IMAGEPRO_API(HRESULT) imagepro_stitch_pullsam(HImageproStitch handle, HUvcsam h, int bFeed, int width, int height, void* pFrameBuffer);
IMAGEPRO_API(HRESULT) imagepro_edf_pullsam(HImageproEdf handle, HUvcsam h, int bFeed, int width, int height, void* pFrameBuffer);
#endif

#ifdef __cplusplus
}
#endif

#if defined(_WIN32)
#pragma pack(pop)
#endif

#endif
