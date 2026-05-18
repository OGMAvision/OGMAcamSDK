#include <mex.h>
#include "ogmacam.h"

#define PARAM_CLOSE        0x0000
#define PARAM_SNAP         0x0001
#define PARAM_AEXP         0x0002
#define PARAM_EXPTIME      0x0003
#define PARAM_WBONCE       0x0004
#define PARAM_TEMP         0x0005
#define PARAM_TINT         0x0006
#define PARAM_HUE          0x0007
#define PARAM_SATURATION   0x0008
#define PARAM_BRIGHTNESS   0x0009
#define PARAM_CONTRAST     0x000a
#define PARAM_GAMMA        0x000b
#define PARAM_WBDFT        0x000c
#define PARAM_COLORDFT     0x000d
#define PARAM_BBONCE       0x000e
#define PARAM_ROFFSET      0x000f
#define PARAM_GOFFSET      0x0010
#define PARAM_BOFFSET      0x0011
#define PARAM_BBDFT        0x0012
#define PARAM_CHROME       0x0013
#define PARAM_FFCONCE      0x0014
#define PARAM_DFCONCE      0x0015
#define PARAM_POWERSUPPLY  0x0016
#define PARAM_VFLIP        0x0017
#define PARAM_HFLIP        0x0018
#define PARAM_ROTATE       0x0019
#define PARAM_SHARPENING   0x001a
#define PARAM_BITDEPTH     0x001b
#define PARAM_BINNING      0x001c
#define PARAM_LINEAR       0x001d
#define PARAM_CURVE        0x001e

static unsigned char* g_imgData = nullptr;
static HOgmacam g_hCam = nullptr;
static int g_captureIndex = 0;

static void saveDataAsBmp(const void* pData, const BITMAPINFOHEADER* pHeader)
{
    char str[256];
    sprintf(str, "%04d.bmp", ++g_captureIndex);
    FILE* fp = fopen(str, "wb");
    if (fp)
    {
        BITMAPFILEHEADER fh = { 0 };
        fh.bfType = 'M' << 8 | 'B';
        fh.bfSize = sizeof(fh) + sizeof(BITMAPINFOHEADER) + pHeader->biSizeImage;
        fh.bfOffBits = sizeof(fh) + sizeof(BITMAPINFOHEADER);
        fwrite(&fh, sizeof(fh), 1, fp);
        fwrite(pHeader, sizeof(BITMAPINFOHEADER), 1, fp);
        fwrite(pData, pHeader->biSizeImage, 1, fp);
        fclose(fp);
    }
}

static void DataCallback(const void* pData, const BITMAPINFOHEADER* pHeader, BOOL bSnap, void* pCallbackCtx)
{
    if (bSnap)
        saveDataAsBmp(pData, pHeader);
    else if (g_imgData)
    {
        if (TDIBWIDTHBYTES(pHeader->biWidth * 24) == pHeader->biWidth * 3)          // row pitch
            memcpy(g_imgData, pData, pHeader->biWidth * pHeader->biHeight * 3);
        else
        {
            unsigned char* pTmp = (unsigned char*)pData;
            for (int i = 0; i < pHeader->biHeight; ++i)
                memcpy(g_imgData + pHeader->biWidth * 3 * i, pTmp + TDIBWIDTHBYTES(pHeader->biWidth * 24) * i, pHeader->biWidth * 3);
        }
    }
}

static void EnumCameras(mxArray* plhs[])
{
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
    double* y = mxGetPr(plhs[0]);
    OgmacamDeviceV2 ti[OGMACAM_MAX];
    int ret = Ogmacam_EnumV2(ti);
    *y = ret;
    if (ret)
    {
        mwSize dims[2] = {1, ret};
        const char* field_names[] = {"name", "id"};
        plhs[1] = mxCreateStructArray(2, dims, 2, field_names);
        int name_field = mxGetFieldNumber(plhs[1], "name");
        int id_field = mxGetFieldNumber(plhs[1], "id");
        for (int i = 0; i < ret; i++)
        {
            char name[64] = { 0 }, id[64] = { 0 };
            wcstombs(name, ti[i].displayname, sizeof(name));
            wcstombs(id, ti[i].id, sizeof(id));
            mxSetFieldByNumber(plhs[1], i, name_field, mxCreateString(name));
            mxSetFieldByNumber(plhs[1], i, id_field, mxCreateString(id));
        }
    }
    else
    {
        plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
    }
}

static int Init(unsigned nRes, unsigned short nSpeed, char* ID)
{
    wchar_t id[64] = { 0 };
    if (ID && ID[0])
        mbstowcs(id, ID, sizeof(id));
    g_hCam = Ogmacam_Open(id);
    if (g_hCam)
    {
        Ogmacam_put_Speed(g_hCam, nSpeed);
        Ogmacam_put_eSize(g_hCam, nRes);
        Ogmacam_StartPushMode(g_hCam, DataCallback, nullptr);
        return S_OK;
    }
    return S_FALSE;
}

static void ConstParams(mxArray* plhs[])
{
    const char* field_names[] = {"OGMACAM_TEMP_DEF", "OGMACAM_TEMP_MIN", "OGMACAM_TEMP_MAX", "OGMACAM_TINT_DEF", "OGMACAM_TINT_MIN",
                                "OGMACAM_TINT_MAX", "OGMACAM_HUE_DEF", "OGMACAM_HUE_MIN", "OGMACAM_HUE_MAX", "OGMACAM_SATURATION_DEF", "OGMACAM_SATURATION_MIN",
                                "OGMACAM_SATURATION_MAX", "OGMACAM_BRIGHTNESS_DEF", "OGMACAM_BRIGHTNESS_MIN", "OGMACAM_BRIGHTNESS_MAX", "OGMACAM_CONTRAST_DEF",
                                "OGMACAM_CONTRAST_MIN", "OGMACAM_CONTRAST_MAX", "OGMACAM_GAMMA_DEF", "OGMACAM_GAMMA_MIN", "OGMACAM_GAMMA_MAX", "OGMACAM_AETARGET_DEF",
                                "OGMACAM_AETARGET_MIN", "OGMACAM_AETARGET_MAX", "OGMACAM_WBGAIN_DEF", "OGMACAM_WBGAIN_MIN", "OGMACAM_WBGAIN_MAX", "OGMACAM_BLACKLEVEL_MIN",
                                "OGMACAM_BLACKLEVEL8_MAX", "OGMACAM_BLACKLEVEL10_MAX", "OGMACAM_BLACKLEVEL12_MAX", "OGMACAM_BLACKLEVEL14_MAX", "OGMACAM_BLACKLEVEL16_MAX",
                                "OGMACAM_SHARPENING_STRENGTH_DEF", "OGMACAM_SHARPENING_STRENGTH_MIN", "OGMACAM_SHARPENING_STRENGTH_MAX", "OGMACAM_SHARPENING_RADIUS_DEF",
                                "OGMACAM_SHARPENING_RADIUS_MIN", "OGMACAM_SHARPENING_RADIUS_MAX", "OGMACAM_SHARPENING_THRESHOLD_DEF", "OGMACAM_SHARPENING_THRESHOLD_MIN",
                                "OGMACAM_SHARPENING_THRESHOLD_MAX"};
    int OGMACAM_TEMP_DEF_field, OGMACAM_TEMP_MIN_field, OGMACAM_TEMP_MAX_field, OGMACAM_TINT_DEF_field, OGMACAM_TINT_MIN_field,
                                OGMACAM_TINT_MAX_field, OGMACAM_HUE_DEF_field, OGMACAM_HUE_MIN_field, OGMACAM_HUE_MAX_field, OGMACAM_SATURATION_DEF_field,
                                OGMACAM_SATURATION_MIN_field, OGMACAM_SATURATION_MAX_field, OGMACAM_BRIGHTNESS_DEF_field, OGMACAM_BRIGHTNESS_MIN_field,
                                OGMACAM_BRIGHTNESS_MAX_field, OGMACAM_CONTRAST_DEF_field, OGMACAM_CONTRAST_MIN_field, OGMACAM_CONTRAST_MAX_field,
                                OGMACAM_GAMMA_DEF_field, OGMACAM_GAMMA_MIN_field, OGMACAM_GAMMA_MAX_field, OGMACAM_AETARGET_DEF_field, OGMACAM_AETARGET_MIN_field,
                                OGMACAM_AETARGET_MAX_field, OGMACAM_WBGAIN_DEF_field, OGMACAM_WBGAIN_MIN_field, OGMACAM_WBGAIN_MAX_field,
                                OGMACAM_BLACKLEVEL_MIN_field, OGMACAM_BLACKLEVEL8_MAX_field, OGMACAM_BLACKLEVEL10_MAX_field, OGMACAM_BLACKLEVEL12_MAX_field,
                                OGMACAM_BLACKLEVEL14_MAX_field, OGMACAM_BLACKLEVEL16_MAX_field, OGMACAM_SHARPENING_STRENGTH_DEF_field, OGMACAM_SHARPENING_STRENGTH_MIN_field,
                                OGMACAM_SHARPENING_STRENGTH_MAX_field, OGMACAM_SHARPENING_RADIUS_DEF_field, OGMACAM_SHARPENING_RADIUS_MIN_field,
                                OGMACAM_SHARPENING_RADIUS_MAX_field, OGMACAM_SHARPENING_THRESHOLD_DEF_field, OGMACAM_SHARPENING_THRESHOLD_MIN_field,
                                OGMACAM_SHARPENING_THRESHOLD_MAX_field;
    plhs[3] = mxCreateStructMatrix(1, 1, 42, field_names);
    OGMACAM_TEMP_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_TEMP_DEF");
    OGMACAM_TEMP_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_TEMP_MIN");
    OGMACAM_TEMP_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_TEMP_MAX");
    OGMACAM_TINT_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_TINT_DEF");
    OGMACAM_TINT_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_TINT_MIN");
    OGMACAM_TINT_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_TINT_MAX");
    OGMACAM_HUE_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_HUE_DEF");
    OGMACAM_HUE_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_HUE_MIN");
    OGMACAM_HUE_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_HUE_MAX");
    OGMACAM_SATURATION_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_SATURATION_DEF");
    OGMACAM_SATURATION_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_SATURATION_MIN");
    OGMACAM_SATURATION_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_SATURATION_MAX");
    OGMACAM_BRIGHTNESS_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_BRIGHTNESS_DEF");
    OGMACAM_BRIGHTNESS_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_BRIGHTNESS_MIN");
    OGMACAM_BRIGHTNESS_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BRIGHTNESS_MAX");
    OGMACAM_CONTRAST_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_CONTRAST_DEF");
    OGMACAM_CONTRAST_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_CONTRAST_MIN");
    OGMACAM_CONTRAST_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_CONTRAST_MAX");
    OGMACAM_GAMMA_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_GAMMA_DEF");
    OGMACAM_GAMMA_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_GAMMA_MIN");
    OGMACAM_GAMMA_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_GAMMA_MAX");
    OGMACAM_AETARGET_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_AETARGET_DEF");
    OGMACAM_AETARGET_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_AETARGET_MIN");
    OGMACAM_AETARGET_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_AETARGET_MAX");
    OGMACAM_WBGAIN_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_WBGAIN_DEF");
    OGMACAM_WBGAIN_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_WBGAIN_MIN");
    OGMACAM_WBGAIN_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_WBGAIN_MAX");
    OGMACAM_BLACKLEVEL_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL_MIN");
    OGMACAM_BLACKLEVEL8_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL8_MAX");
    OGMACAM_BLACKLEVEL10_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL10_MAX");
    OGMACAM_BLACKLEVEL12_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL12_MAX");
    OGMACAM_BLACKLEVEL14_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL14_MAX");
    OGMACAM_BLACKLEVEL16_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_BLACKLEVEL16_MAX");
    OGMACAM_SHARPENING_STRENGTH_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_STRENGTH_DEF");
    OGMACAM_SHARPENING_STRENGTH_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_STRENGTH_MIN");
    OGMACAM_SHARPENING_STRENGTH_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_STRENGTH_MAX");
    OGMACAM_SHARPENING_RADIUS_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_RADIUS_DEF");
    OGMACAM_SHARPENING_RADIUS_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_RADIUS_MIN");
    OGMACAM_SHARPENING_RADIUS_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_RADIUS_MAX");
    OGMACAM_SHARPENING_THRESHOLD_DEF_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_THRESHOLD_DEF");
    OGMACAM_SHARPENING_THRESHOLD_MIN_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_THRESHOLD_MIN");
    OGMACAM_SHARPENING_THRESHOLD_MAX_field = mxGetFieldNumber(plhs[3], "OGMACAM_SHARPENING_THRESHOLD_MAX");
    mxArray *OGMACAM_TEMP_DEF_value, *OGMACAM_TEMP_MIN_value, *OGMACAM_TEMP_MAX_value, *OGMACAM_TINT_DEF_value, *OGMACAM_TINT_MIN_value,
            *OGMACAM_TINT_MAX_value, *OGMACAM_HUE_DEF_value, *OGMACAM_HUE_MIN_value, *OGMACAM_HUE_MAX_value, *OGMACAM_SATURATION_DEF_value,
            *OGMACAM_SATURATION_MIN_value, *OGMACAM_SATURATION_MAX_value, *OGMACAM_BRIGHTNESS_DEF_value, *OGMACAM_BRIGHTNESS_MIN_value,
            *OGMACAM_BRIGHTNESS_MAX_value, *OGMACAM_CONTRAST_DEF_value, *OGMACAM_CONTRAST_MIN_value, *OGMACAM_CONTRAST_MAX_value,
            *OGMACAM_GAMMA_DEF_value, *OGMACAM_GAMMA_MIN_value, *OGMACAM_GAMMA_MAX_value, *OGMACAM_AETARGET_DEF_value, *OGMACAM_AETARGET_MIN_value,
            *OGMACAM_AETARGET_MAX_value, *OGMACAM_WBGAIN_DEF_value, *OGMACAM_WBGAIN_MIN_value, *OGMACAM_WBGAIN_MAX_value,
            *OGMACAM_BLACKLEVEL_MIN_value, *OGMACAM_BLACKLEVEL8_MAX_value, *OGMACAM_BLACKLEVEL10_MAX_value, *OGMACAM_BLACKLEVEL12_MAX_value,
            *OGMACAM_BLACKLEVEL14_MAX_value, *OGMACAM_BLACKLEVEL16_MAX_value, *OGMACAM_SHARPENING_STRENGTH_DEF_value, *OGMACAM_SHARPENING_STRENGTH_MIN_value,
            *OGMACAM_SHARPENING_STRENGTH_MAX_value, *OGMACAM_SHARPENING_RADIUS_DEF_value, *OGMACAM_SHARPENING_RADIUS_MIN_value,
            *OGMACAM_SHARPENING_RADIUS_MAX_value, *OGMACAM_SHARPENING_THRESHOLD_DEF_value, *OGMACAM_SHARPENING_THRESHOLD_MIN_value,
            *OGMACAM_SHARPENING_THRESHOLD_MAX_value;
    OGMACAM_TEMP_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_TEMP_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_TEMP_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_TINT_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_TINT_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_TINT_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_HUE_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_HUE_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_HUE_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SATURATION_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SATURATION_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SATURATION_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BRIGHTNESS_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BRIGHTNESS_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BRIGHTNESS_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_CONTRAST_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_CONTRAST_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_CONTRAST_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_GAMMA_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_GAMMA_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_GAMMA_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_AETARGET_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_AETARGET_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_AETARGET_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_WBGAIN_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_WBGAIN_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_WBGAIN_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL8_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL10_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL12_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL14_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_BLACKLEVEL16_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_STRENGTH_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_STRENGTH_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_STRENGTH_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_RADIUS_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_RADIUS_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_RADIUS_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_THRESHOLD_DEF_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_THRESHOLD_MIN_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    OGMACAM_SHARPENING_THRESHOLD_MAX_value = mxCreateDoubleMatrix(1, 1, mxREAL);
    *mxGetPr(OGMACAM_TEMP_DEF_value) = static_cast<double>(OGMACAM_TEMP_DEF);
    *mxGetPr(OGMACAM_TEMP_MIN_value) = static_cast<double>(OGMACAM_TEMP_MIN);
    *mxGetPr(OGMACAM_TEMP_MAX_value) = static_cast<double>(OGMACAM_TEMP_MAX);
    *mxGetPr(OGMACAM_TINT_DEF_value) = static_cast<double>( OGMACAM_TINT_DEF);
    *mxGetPr(OGMACAM_TINT_MIN_value) = static_cast<double>(OGMACAM_TINT_MIN);
    *mxGetPr(OGMACAM_TINT_MAX_value) = static_cast<double>(OGMACAM_TINT_MAX);
    *mxGetPr(OGMACAM_HUE_DEF_value) = static_cast<double>(OGMACAM_HUE_DEF);
    *mxGetPr(OGMACAM_HUE_MIN_value) = static_cast<double>(OGMACAM_HUE_MIN);
    *mxGetPr(OGMACAM_HUE_MAX_value) = static_cast<double>(OGMACAM_HUE_MAX);
    *mxGetPr(OGMACAM_SATURATION_DEF_value) = static_cast<double>(OGMACAM_SATURATION_DEF);
    *mxGetPr(OGMACAM_SATURATION_MIN_value) = static_cast<double>(OGMACAM_SATURATION_MIN);
    *mxGetPr(OGMACAM_SATURATION_MAX_value) = static_cast<double>(OGMACAM_SATURATION_MAX);
    *mxGetPr(OGMACAM_BRIGHTNESS_DEF_value) = static_cast<double>(OGMACAM_BRIGHTNESS_DEF);
    *mxGetPr(OGMACAM_BRIGHTNESS_MIN_value) = static_cast<double>(OGMACAM_BRIGHTNESS_MIN);
    *mxGetPr(OGMACAM_BRIGHTNESS_MAX_value) = static_cast<double>(OGMACAM_BRIGHTNESS_MAX);
    *mxGetPr(OGMACAM_CONTRAST_DEF_value) = static_cast<double>(OGMACAM_CONTRAST_DEF);
    *mxGetPr(OGMACAM_CONTRAST_MIN_value) = static_cast<double>(OGMACAM_CONTRAST_MIN);
    *mxGetPr(OGMACAM_CONTRAST_MAX_value) = static_cast<double>(OGMACAM_CONTRAST_MAX);
    *mxGetPr(OGMACAM_GAMMA_DEF_value) = static_cast<double>(OGMACAM_GAMMA_DEF);
    *mxGetPr(OGMACAM_GAMMA_MIN_value) = static_cast<double>(OGMACAM_GAMMA_MIN);
    *mxGetPr(OGMACAM_GAMMA_MAX_value) = static_cast<double>(OGMACAM_GAMMA_MAX);
    *mxGetPr(OGMACAM_AETARGET_DEF_value) = static_cast<double>(OGMACAM_AETARGET_DEF);
    *mxGetPr(OGMACAM_AETARGET_MIN_value) = static_cast<double>(OGMACAM_AETARGET_MIN);
    *mxGetPr(OGMACAM_AETARGET_MAX_value) = static_cast<double>(OGMACAM_AETARGET_MAX);
    *mxGetPr(OGMACAM_WBGAIN_DEF_value) = static_cast<double>(OGMACAM_WBGAIN_DEF);
    *mxGetPr(OGMACAM_WBGAIN_MIN_value) = static_cast<double>(OGMACAM_WBGAIN_MIN);
    *mxGetPr(OGMACAM_WBGAIN_MAX_value) = static_cast<double>(OGMACAM_WBGAIN_MAX);
    *mxGetPr(OGMACAM_BLACKLEVEL_MIN_value) = static_cast<double>(OGMACAM_BLACKLEVEL_MIN );
    *mxGetPr(OGMACAM_BLACKLEVEL8_MAX_value) = static_cast<double>( OGMACAM_BLACKLEVEL8_MAX);
    *mxGetPr(OGMACAM_BLACKLEVEL10_MAX_value) = static_cast<double>(OGMACAM_BLACKLEVEL10_MAX);
    *mxGetPr(OGMACAM_BLACKLEVEL12_MAX_value) = static_cast<double>(OGMACAM_BLACKLEVEL12_MAX);
    *mxGetPr(OGMACAM_BLACKLEVEL14_MAX_value) = static_cast<double>(OGMACAM_BLACKLEVEL14_MAX);
    *mxGetPr(OGMACAM_BLACKLEVEL16_MAX_value) = static_cast<double>(OGMACAM_BLACKLEVEL16_MAX);
    *mxGetPr(OGMACAM_SHARPENING_STRENGTH_DEF_value) = static_cast<double>(OGMACAM_SHARPENING_STRENGTH_DEF);
    *mxGetPr(OGMACAM_SHARPENING_STRENGTH_MIN_value) = static_cast<double>(OGMACAM_SHARPENING_STRENGTH_MIN);
    *mxGetPr(OGMACAM_SHARPENING_STRENGTH_MAX_value) = static_cast<double>(OGMACAM_SHARPENING_STRENGTH_MAX);
    *mxGetPr(OGMACAM_SHARPENING_RADIUS_DEF_value) = static_cast<double>(OGMACAM_SHARPENING_RADIUS_DEF);
    *mxGetPr(OGMACAM_SHARPENING_RADIUS_MIN_value) = static_cast<double>(OGMACAM_SHARPENING_RADIUS_MIN);
    *mxGetPr(OGMACAM_SHARPENING_RADIUS_MAX_value) = static_cast<double>(OGMACAM_SHARPENING_RADIUS_MAX);
    *mxGetPr(OGMACAM_SHARPENING_THRESHOLD_DEF_value) = static_cast<double>(OGMACAM_SHARPENING_THRESHOLD_DEF);
    *mxGetPr(OGMACAM_SHARPENING_THRESHOLD_MIN_value) = static_cast<double>(OGMACAM_SHARPENING_THRESHOLD_MIN);
    *mxGetPr(OGMACAM_SHARPENING_THRESHOLD_MAX_value) = static_cast<double>(OGMACAM_SHARPENING_THRESHOLD_MAX);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TEMP_DEF_field, OGMACAM_TEMP_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TEMP_MIN_field, OGMACAM_TEMP_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TEMP_MAX_field, OGMACAM_TEMP_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TINT_DEF_field, OGMACAM_TINT_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TINT_MIN_field, OGMACAM_TINT_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_TINT_MAX_field, OGMACAM_TINT_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_HUE_DEF_field, OGMACAM_HUE_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_HUE_MIN_field, OGMACAM_HUE_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_HUE_MAX_field, OGMACAM_HUE_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SATURATION_DEF_field, OGMACAM_SATURATION_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SATURATION_MIN_field, OGMACAM_SATURATION_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SATURATION_MAX_field, OGMACAM_SATURATION_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BRIGHTNESS_DEF_field, OGMACAM_BRIGHTNESS_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BRIGHTNESS_MIN_field, OGMACAM_BRIGHTNESS_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BRIGHTNESS_MAX_field, OGMACAM_BRIGHTNESS_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_CONTRAST_DEF_field, OGMACAM_CONTRAST_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_CONTRAST_MIN_field, OGMACAM_CONTRAST_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_CONTRAST_MAX_field, OGMACAM_CONTRAST_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_GAMMA_DEF_field, OGMACAM_GAMMA_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_GAMMA_MIN_field, OGMACAM_GAMMA_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_GAMMA_MAX_field, OGMACAM_GAMMA_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_AETARGET_DEF_field, OGMACAM_AETARGET_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_AETARGET_MIN_field, OGMACAM_AETARGET_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_AETARGET_MAX_field, OGMACAM_AETARGET_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_WBGAIN_DEF_field, OGMACAM_WBGAIN_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_WBGAIN_MIN_field, OGMACAM_WBGAIN_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_WBGAIN_MAX_field, OGMACAM_WBGAIN_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL_MIN_field, OGMACAM_BLACKLEVEL_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL8_MAX_field, OGMACAM_BLACKLEVEL8_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL10_MAX_field, OGMACAM_BLACKLEVEL10_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL12_MAX_field, OGMACAM_BLACKLEVEL12_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL14_MAX_field, OGMACAM_BLACKLEVEL14_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_BLACKLEVEL16_MAX_field, OGMACAM_BLACKLEVEL16_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_STRENGTH_DEF_field, OGMACAM_SHARPENING_STRENGTH_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_STRENGTH_MIN_field, OGMACAM_SHARPENING_STRENGTH_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_STRENGTH_MAX_field, OGMACAM_SHARPENING_STRENGTH_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_RADIUS_DEF_field, OGMACAM_SHARPENING_RADIUS_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_RADIUS_MIN_field, OGMACAM_SHARPENING_RADIUS_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_RADIUS_MAX_field, OGMACAM_SHARPENING_RADIUS_MAX_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_THRESHOLD_DEF_field, OGMACAM_SHARPENING_THRESHOLD_DEF_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_THRESHOLD_MIN_field, OGMACAM_SHARPENING_THRESHOLD_MIN_value);
    mxSetFieldByNumber(plhs[3], 0, OGMACAM_SHARPENING_THRESHOLD_MAX_field, OGMACAM_SHARPENING_THRESHOLD_MAX_value);
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    if (nrhs == 0)
        EnumCameras(plhs);
    else if (nrhs == 4)
    {
        unsigned nRes = static_cast<unsigned>(*(mxGetPr(prhs[0])));
        unsigned short nSpeed = static_cast<unsigned short>(*(mxGetPr(prhs[1])));
        char ID[64] = { 0 };
        mxGetString(prhs[2], ID, sizeof(ID));
        if (S_OK == Init(nRes, nSpeed, ID))
        {
            int srcW = 0, srcH = 0;
            Ogmacam_get_Resolution(g_hCam, 0, &srcW, &srcH);
            mwSize dims[2] = {srcW * 3, srcH};
            plhs[0] = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
            g_imgData = static_cast<unsigned char*>(mxGetData(plhs[0]));
            mexMakeMemoryPersistent(g_imgData);
            plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
            plhs[2] = mxCreateDoubleMatrix(1, 1, mxREAL);
            double* w = mxGetPr(plhs[1]);
            double* h = mxGetPr(plhs[2]);
            *w = static_cast<double>(srcW);
            *h = static_cast<double>(srcH);
            ConstParams(plhs);
        }
    }
    else if (nrhs == 2)
    {
        if (nullptr == g_hCam)
            return;
        const int value = static_cast<int>(*mxGetPr(prhs[1]));
        switch (static_cast<int>(*mxGetPr(prhs[0])))
        {
            case PARAM_CLOSE:
                Ogmacam_Close(g_hCam);
                g_hCam = nullptr;
                break;
            case PARAM_SNAP:
                Ogmacam_Snap(g_hCam, 0);
                break;
            case PARAM_AEXP:
                {
                    BOOL bAutoExposure;
                    unsigned nTime;
                    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
                    plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
                    double* y = mxGetPr(plhs[0]);
                    double* time = mxGetPr(plhs[1]);
                    Ogmacam_get_AutoExpoEnable(g_hCam, &bAutoExposure);
                    Ogmacam_put_AutoExpoEnable(g_hCam, !bAutoExposure);
                    Ogmacam_get_ExpoTime(g_hCam, &nTime);
                    *y = !bAutoExposure;
                    *time = nTime;
                }
                break;
            case PARAM_EXPTIME:
                Ogmacam_put_ExpoTime(g_hCam, static_cast<unsigned>(value));
                break;
            case PARAM_WBONCE:
                {
                    int nTemp, nTint;
                    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
                    plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
                    double* temp = mxGetPr(plhs[0]);
                    double* tint = mxGetPr(plhs[1]);
                    Ogmacam_AwbOnce(g_hCam, nullptr, nullptr);
                    Ogmacam_get_TempTint(g_hCam, &nTemp, &nTint);
                    *temp = (double)nTemp;
                    *tint = (double)nTint;
                }
                break;
            case PARAM_TEMP:
                {
                    int nTemp = 0, nTint = 0;
                    Ogmacam_get_TempTint(g_hCam, &nTemp, &nTint);
                    Ogmacam_put_TempTint(g_hCam, value, nTint);
                }
                break;
            case PARAM_TINT:
                {
                    int nTemp = 0, nTint = 0;
                    Ogmacam_get_TempTint(g_hCam, &nTemp, &nTint);
                    Ogmacam_put_TempTint(g_hCam, nTemp, value);
                }
                break;
            case PARAM_HUE:
                Ogmacam_put_Hue(g_hCam, value);
                break;
            case PARAM_SATURATION:
                Ogmacam_put_Saturation(g_hCam, value);
                break;
            case PARAM_BRIGHTNESS:
                Ogmacam_put_Brightness(g_hCam, value);
                break;
            case PARAM_CONTRAST:
                Ogmacam_put_Contrast(g_hCam, value);
                break;
            case PARAM_GAMMA:
                Ogmacam_put_Gamma(g_hCam, value);
                break;
            case PARAM_WBDFT:
                Ogmacam_put_TempTint(g_hCam, OGMACAM_TEMP_DEF, OGMACAM_TINT_DEF);
                break;
            case PARAM_COLORDFT:
                Ogmacam_put_Hue(g_hCam, OGMACAM_HUE_DEF);
                Ogmacam_put_Saturation(g_hCam, OGMACAM_SATURATION_DEF);
                Ogmacam_put_Brightness(g_hCam, OGMACAM_BRIGHTNESS_DEF);
                Ogmacam_put_Contrast(g_hCam, OGMACAM_CONTRAST_DEF);
                Ogmacam_put_Gamma(g_hCam, OGMACAM_GAMMA_DEF);
                break;
            case PARAM_BBONCE:
                {
                    unsigned short aSub[3] = { 0 };
                    Ogmacam_AbbOnce(g_hCam, nullptr, nullptr);
                    Ogmacam_get_BlackBalance(g_hCam, aSub);
                    plhs[0] = mxCreateDoubleMatrix(1, 3, mxREAL);
                    double* plaSub = mxGetPr(plhs[0]);
                    plaSub[0] = aSub[0];
                    plaSub[1] = aSub[1];
                    plaSub[2] = aSub[2];
                }
                break;
            case PARAM_ROFFSET:
                {
                    unsigned short aSub[3] = { 0 };
                    Ogmacam_get_BlackBalance(g_hCam, aSub);
                    aSub[0] = value;
                    Ogmacam_put_BlackBalance(g_hCam, aSub);
                    Ogmacam_get_BlackBalance(g_hCam, aSub);
                }
                break;
            case PARAM_GOFFSET:
                {
                    unsigned short aSub[3] = { 0 };
                    Ogmacam_get_BlackBalance(g_hCam, aSub);
                    aSub[1] = value;
                    Ogmacam_put_BlackBalance(g_hCam, aSub);
                }
                break;
            case PARAM_BOFFSET:
                {
                    unsigned short aSub[3] = { 0 };
                    Ogmacam_get_BlackBalance(g_hCam, aSub);
                    aSub[2] = value;
                    Ogmacam_put_BlackBalance(g_hCam, aSub);
                }
                break;
            case PARAM_BBDFT:
                {
                    unsigned short aSub[3] = { 0 };
                    Ogmacam_put_BlackBalance(g_hCam, aSub);
                }
                break;
            case PARAM_CHROME:
                Ogmacam_put_Chrome(g_hCam, value);
                break;
            case PARAM_FFCONCE:
                Ogmacam_FfcOnce(g_hCam);
                break;
            case PARAM_DFCONCE:
                Ogmacam_DfcOnce(g_hCam);
                break;
            case PARAM_POWERSUPPLY:
                Ogmacam_put_HZ(g_hCam, value);
                break;
            case PARAM_VFLIP:
                {
                    int bVFlip = 0;
                    Ogmacam_get_VFlip(g_hCam, &bVFlip);  /* vertical flip */
                    Ogmacam_put_VFlip(g_hCam, !bVFlip);
                }
                break;
            case PARAM_HFLIP:
                {
                    int bHFlip = 0;
                    Ogmacam_get_HFlip(g_hCam, &bHFlip);  /* vertical flip */
                    Ogmacam_put_HFlip(g_hCam, !bHFlip);
                }
                break;
            case PARAM_ROTATE:
                {
                    int iValue = 0;
                    Ogmacam_get_Option(g_hCam, OGMACAM_OPTION_ROTATE, &iValue);
                    iValue = 180 - iValue;
                    Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_ROTATE, iValue);
                }
                break;
            case PARAM_SHARPENING:
                Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_SHARPENING, value);
                break;
            case PARAM_BITDEPTH:
                Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_BITDEPTH, value);
                break;
            case PARAM_BINNING:
                Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_BINNING, value);
                break;
            case PARAM_LINEAR:
                Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_LINEAR, value);
                break;
            case PARAM_CURVE:
                Ogmacam_put_Option(g_hCam, OGMACAM_OPTION_CURVE, value);
                break;
            default:
                break;
        }
    }
    else
    {
        mexErrMsgIdAndTxt("MATLAB:mexFun:InvalidInput", "Invalid Input.");
    }
}
