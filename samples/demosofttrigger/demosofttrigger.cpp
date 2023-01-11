#include <stdio.h>
#include <stdlib.h>
#include "ogmacam.h"

HOgmacam g_hcam = NULL;
void* g_pImageData = NULL;
unsigned g_total = 0;

static void __stdcall EventCallback(unsigned nEvent, void* pCallbackCtx)
{
    if (OGMACAM_EVENT_IMAGE == nEvent)
    {
        OgmacamFrameInfoV3 info = { 0 };
        const HRESULT hr = Ogmacam_PullImageV3(g_hcam, g_pImageData, 0, 24, 0, &info);
        if (FAILED(hr))
            printf("failed to pull image, hr = %08x\n", hr);
        else
        {
            /* After we get the image data, we can do anything for the data we want to do */
            printf("pull image ok, total = %u, res = %u x %u\n", ++g_total, info.width, info.height);
        }
    }
    else
    {
        printf("event callback: 0x%04x\n", nEvent);
    }
}

int main(int, char**)
{
    g_hcam = Ogmacam_Open(NULL);
    if (NULL == g_hcam)
    {
        printf("no camera found or open failed\n");
        return -1;
    }
    if ((Ogmacam_query_Model(g_hcam)->flag & OGMACAM_FLAG_TRIGGER_SOFTWARE) == 0)
        printf("camera do NOT support software trigger, fallback to simulated trigger\n");

    int nWidth = 0, nHeight = 0;
    HRESULT hr = Ogmacam_get_Size(g_hcam, &nWidth, &nHeight);
    if (FAILED(hr))
        printf("failed to get size, hr = %08x\n", hr);
    else
    {
        g_pImageData = malloc(TDIBWIDTHBYTES(24 * nWidth) * nHeight);
        if (NULL == g_pImageData)
            printf("failed to malloc\n");
        else
        {
            Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_TRIGGER, 1);
            hr = Ogmacam_StartPullModeWithCallback(g_hcam, EventCallback, NULL);
            if (FAILED(hr))
                printf("failed to start camera, hr = %08x\n", hr);
            else
            {
                printf("'x' to exit, number to trigger\n");
                do {
                    char str[1024];
                    if (fgets(str, 1023, stdin))
                    {
                        if (('x' == str[0]) || ('X' == str[0]))
                            break;
                        int n = atoi(str);
                        if (n > 0)
                            Ogmacam_Trigger(g_hcam, n);
                    }
                } while (true);
            }
        }
    }
    
    /* cleanup */
    Ogmacam_Close(g_hcam);
    if (g_pImageData)
        free(g_pImageData);
    return 0;
}
