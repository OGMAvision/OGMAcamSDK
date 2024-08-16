#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <conio.h>
#else
#include <curses.h>
#endif
#include "ogmacam.h"

HOgmacam g_hcam = NULL;
void* g_pImageData = NULL;
unsigned g_total = 0, g_totalstill = 0;
bool g_bBitdepth = false, g_bSave = false;

static void SaveRaw(const char* filePrefix, unsigned num, unsigned width, unsigned height, const void* pData, unsigned length)
{
    char filename[1024];
    sprintf(filename, "%s_%ux%u_%u.raw", filePrefix, width, height, num);
    FILE* fp = fopen(filename, "wb");
    if (fp)
    {
        fwrite(pData, 1, length, fp);
        fclose(fp);
    }
    printf("saveraw: %s\n", filename);
}

static void __stdcall EventCallback(unsigned nEvent, void* pCallbackCtx)
{
    if (OGMACAM_EVENT_IMAGE == nEvent)
    {
        OgmacamFrameInfoV4 info = { 0 };
        const HRESULT hr = Ogmacam_PullImageV4(g_hcam, g_pImageData, 0, 0, 0, &info);
        if (FAILED(hr))
            printf("failed to pull image, hr = 0x%08x\n", hr);
        else
        {
            printf("pull image ok, total = %u, resolution = %u x %u\n", ++g_total, info.v3.width, info.v3.height);
            if (g_bSave)
            {
                g_bSave = false;
                SaveRaw("demoraw", g_total, info.v3.width, info.v3.height, g_pImageData, info.v3.width * info.v3.height * (g_bBitdepth ? 2 : 1));
            }
        }
    }
    else if (OGMACAM_EVENT_STILLIMAGE == nEvent)
    {
        OgmacamFrameInfoV4 info = { 0 };
        const HRESULT hr = Ogmacam_PullImageV4(g_hcam, g_pImageData, 1, 0, 0, &info);
        if (FAILED(hr))
            printf("failed to pull still image, hr = 0x%08x\n", hr);
        else
        {
            printf("pull still image ok, total = %u, resolution = %u x %u\n", ++g_totalstill, info.v3.width, info.v3.height);
            SaveRaw("demorawstill", g_totalstill, info.v3.width, info.v3.height, g_pImageData, info.v3.width * info.v3.height * (g_bBitdepth ? 2 : 1));
        }
    }
    else
    {
        printf("event callback: 0x%04x\n", nEvent);
    }
}

int main(int, char**)
{
    OgmacamDeviceV2 tdev[OGMACAM_MAX] = { 0 };
    const unsigned ncam = Ogmacam_EnumV2(tdev);
    if (0 == ncam)
    {
        printf("no camera found\n");
        return -1;
    }
    
    g_hcam = Ogmacam_Open(tdev[0].id);
    if (NULL == g_hcam)
    {
        printf("failed to open camera\n");
        return -1;
    }
    
    HRESULT hr = Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_RAW, 1); /* use raw image data */
    printf("put option raw: hr = 0x%08x\n", hr);

    if (tdev[0].model->preview > 1)
        Ogmacam_put_eSize(g_hcam, 1);

    if (tdev[0].model->flag & (OGMACAM_FLAG_RAW10 | OGMACAM_FLAG_RAW12 | OGMACAM_FLAG_RAW14 | OGMACAM_FLAG_RAW16))  /* bitdepth supported */
    {
        Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_BITDEPTH, 1); /* enable bitdepth */
        g_bBitdepth = true;
    }

    int nMaxWidth = 0, nMaxHeight = 0;
    hr = Ogmacam_get_Resolution(g_hcam, 0, &nMaxWidth, &nMaxHeight);
    if (FAILED(hr))
        printf("failed to get size, hr = 0x%08x\n", hr);
    else
    {
        g_pImageData = malloc(nMaxWidth * nMaxHeight * (g_bBitdepth ? 2 : 1));
        if (NULL == g_pImageData)
            printf("failed to malloc\n");
        else
        {
            hr = Ogmacam_StartPullModeWithCallback(g_hcam, EventCallback, NULL);
            if (FAILED(hr))
                printf("failed to start camera, hr = 0x%08x\n", hr);
            else
            {
                bool bloop = true;
                while (bloop)
                {
                    printf("press s to save raw, any other key to exit\n");
                    switch (getch())
                    {
                    case 'x':
                    case 'X':
                        bloop = false;
                        break;
                    case 's':
                    case 'S':
                        g_bSave = true;
                        break;
                    default:
                        if (tdev[0].model->still)             /* snap supported */
                            Ogmacam_Snap(g_hcam, 0);
                        else
                            bloop = false;
                        break;
                    }
                }
            }
        }
    }
    
    /* cleanup */
    Ogmacam_Close(g_hcam);
    if (g_pImageData)
        free(g_pImageData);
    return 0;
}
