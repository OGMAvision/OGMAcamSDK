import java.io.IOException;

public class simplest {
    static ogmacam _cam = null;
    static byte[] _buf = null;
    static int _total = 0;

    private static class ImplEventCallback implements ogmacam.IEventCallback {
        /* the vast majority of callbacks come from ogmacam.dll/so/dylib internal threads */
        @Override
        public void onEvent(int nEvent) {
            switch (nEvent) {
                case ogmacam.EVENT_IMAGE:
                    try {
                        _cam.PullImage(_buf, 0, 24, -1, null);
                        ++_total;
                        System.out.println("pull image ok: " + _total + ", " + String.format("%02x", _buf[_buf.length / 2]));
                    } catch (ogmacam.HRESULTException e) {
                        System.out.println("pull image exception: " + e);
                    }
                    break;
                default:
                    System.out.println("event callback: " + nEvent);
                    break;
            }
        }
    }
    
    public static void main(String[] args) {
        ogmacam.DeviceV2[] arr = ogmacam.EnumV2();
        if (arr.length == 0)
            System.out.println("no camera found");
        else {
            System.out.println(arr[0].displayname + ": 0x" + Long.toHexString(arr[0].model.flag) + ", preview = " + arr[0].model.preview + ", still = " + arr[0].model.still);
            for (int i = 0; i < arr[0].model.res.length; ++i)
                System.out.println(arr[0].model.res[i].width + " x " + arr[0].model.res[i].height);

            _cam = ogmacam.Open(arr[0].id);
            if (_cam != null) {
                try {
                    int[] s = _cam.get_Size();
                    int bufsize = ogmacam.TDIBWIDTHBYTES(s[0] * 24) * s[1];
                    System.out.println("width = " + s[0] + ", height = " + s[1] + ", bufsize = " + bufsize);
                    _buf = new byte[bufsize];
                    _cam.StartPullModeWithCallback(new ImplEventCallback());
                    System.out.println("Press Enter to exit");
                    try {
                        System.in.read();
                    } catch (IOException e) {
                    }
                } catch (ogmacam.HRESULTException e) {
                    System.out.println("start camera exception: " + e);
                } finally {
                    _cam.close();
                    _cam = null;
                    _buf = null;
                }
            }
        }
    }
}
