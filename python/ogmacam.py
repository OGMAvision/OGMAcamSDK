"""Version: 53.22081.20230207
We use ctypes to call into the ogmacam.dll/libogmacam.so/libogmacam.dylib API,
the python class Ogmacam is a thin wrapper class to the native api of ogmacam.dll/libogmacam.so/libogmacam.dylib.
So the manual en.html(English) and hans.html(Simplified Chinese) are also applicable for programming with ogmacam.py.
See them in the 'doc' directory:
   (1) en.html, English
   (2) hans.html, Simplified Chinese
"""
import sys, ctypes, os.path

OGMACAM_MAX = 128

OGMACAM_FLAG_CMOS                = 0x00000001          # cmos sensor
OGMACAM_FLAG_CCD_PROGRESSIVE     = 0x00000002          # progressive ccd sensor
OGMACAM_FLAG_CCD_INTERLACED      = 0x00000004          # interlaced ccd sensor
OGMACAM_FLAG_ROI_HARDWARE        = 0x00000008          # support hardware ROI
OGMACAM_FLAG_MONO                = 0x00000010          # monochromatic
OGMACAM_FLAG_BINSKIP_SUPPORTED   = 0x00000020          # support bin/skip mode
OGMACAM_FLAG_USB30               = 0x00000040          # usb3.0
OGMACAM_FLAG_TEC                 = 0x00000080          # Thermoelectric Cooler
OGMACAM_FLAG_USB30_OVER_USB20    = 0x00000100          # usb3.0 camera connected to usb2.0 port
OGMACAM_FLAG_ST4                 = 0x00000200          # ST4
OGMACAM_FLAG_GETTEMPERATURE      = 0x00000400          # support to get the temperature of the sensor
OGMACAM_FLAG_HIGH_FULLWELL       = 0x00000800          # high fullwell capacity
OGMACAM_FLAG_RAW10               = 0x00001000          # pixel format, RAW 10bits
OGMACAM_FLAG_RAW12               = 0x00002000          # pixel format, RAW 12bits
OGMACAM_FLAG_RAW14               = 0x00004000          # pixel format, RAW 14bits
OGMACAM_FLAG_RAW16               = 0x00008000          # pixel format, RAW 16bits
OGMACAM_FLAG_FAN                 = 0x00010000          # cooling fan
OGMACAM_FLAG_TEC_ONOFF           = 0x00020000          # Thermoelectric Cooler can be turn on or off, support to set the target temperature of TEC
OGMACAM_FLAG_ISP                 = 0x00040000          # ISP (Image Signal Processing) chip
OGMACAM_FLAG_TRIGGER_SOFTWARE    = 0x00080000          # support software trigger
OGMACAM_FLAG_TRIGGER_EXTERNAL    = 0x00100000          # support external trigger
OGMACAM_FLAG_TRIGGER_SINGLE      = 0x00200000          # only support trigger single: one trigger, one image
OGMACAM_FLAG_BLACKLEVEL          = 0x00400000          # support set and get the black level
OGMACAM_FLAG_AUTO_FOCUS          = 0x00800000          # support auto focus
OGMACAM_FLAG_BUFFER              = 0x01000000          # frame buffer
OGMACAM_FLAG_DDR                 = 0x02000000          # use very large capacity DDR (Double Data Rate SDRAM) for frame buffer. The capacity is not less than one full frame
OGMACAM_FLAG_CG                  = 0x04000000          # support Conversion Gain mode: HCG, LCG
OGMACAM_FLAG_YUV411              = 0x08000000          # pixel format, yuv411
OGMACAM_FLAG_VUYY                = 0x10000000          # pixel format, yuv422, VUYY
OGMACAM_FLAG_YUV444              = 0x20000000          # pixel format, yuv444
OGMACAM_FLAG_RGB888              = 0x40000000          # pixel format, RGB888
OGMACAM_FLAG_RAW8                = 0x80000000          # pixel format, RAW 8 bits
OGMACAM_FLAG_GMCY8               = 0x0000000100000000  # pixel format, GMCY, 8 bits
OGMACAM_FLAG_GMCY12              = 0x0000000200000000  # pixel format, GMCY, 12 bits
OGMACAM_FLAG_UYVY                = 0x0000000400000000  # pixel format, yuv422, UYVY
OGMACAM_FLAG_CGHDR               = 0x0000000800000000  # Conversion Gain: HCG, LCG, HDR
OGMACAM_FLAG_GLOBALSHUTTER       = 0x0000001000000000  # global shutter
OGMACAM_FLAG_FOCUSMOTOR          = 0x0000002000000000  # support focus motor
OGMACAM_FLAG_PRECISE_FRAMERATE   = 0x0000004000000000  # support precise framerate & bandwidth, see OGMACAM_OPTION_PRECISE_FRAMERATE & OGMACAM_OPTION_BANDWIDTH
OGMACAM_FLAG_HEAT                = 0x0000008000000000  # support heat to prevent fogging up
OGMACAM_FLAG_LOW_NOISE           = 0x0000010000000000  # support low noise mode (Higher signal noise ratio, lower frame rate)
OGMACAM_FLAG_LEVELRANGE_HARDWARE = 0x0000020000000000  # hardware level range, put(get)_LevelRangeV2
OGMACAM_FLAG_EVENT_HARDWARE      = 0x0000040000000000  # hardware event, such as exposure start & stop
OGMACAM_FLAG_LIGHTSOURCE         = 0x0000080000000000  # light source
OGMACAM_FLAG_FILTERWHEEL         = 0x0000100000000000  # filter wheel
OGMACAM_FLAG_GIGE                = 0x0000200000000000  # GigE
OGMACAM_FLAG_10GIGE              = 0x0000400000000000  # 10 Gige

OGMACAM_EVENT_EXPOSURE           = 0x0001          # exposure time or gain changed
OGMACAM_EVENT_TEMPTINT           = 0x0002          # white balance changed, Temp/Tint mode
OGMACAM_EVENT_CHROME             = 0x0003          # reversed, do not use it
OGMACAM_EVENT_IMAGE              = 0x0004          # live image arrived, use PullImageXXXX to get this image
OGMACAM_EVENT_STILLIMAGE         = 0x0005          # snap (still) frame arrived, use PullStillImageXXXX to get this frame
OGMACAM_EVENT_WBGAIN             = 0x0006          # white balance changed, RGB Gain mode
OGMACAM_EVENT_TRIGGERFAIL        = 0x0007          # trigger failed
OGMACAM_EVENT_BLACK              = 0x0008          # black balance changed
OGMACAM_EVENT_FFC                = 0x0009          # flat field correction status changed
OGMACAM_EVENT_DFC                = 0x000a          # dark field correction status changed
OGMACAM_EVENT_ROI                = 0x000b          # roi changed
OGMACAM_EVENT_LEVELRANGE         = 0x000c          # level range changed
OGMACAM_EVENT_AUTOEXPO_CONV      = 0x000d          # auto exposure convergence
OGMACAM_EVENT_AUTOEXPO_CONVFAIL  = 0x000e          # auto exposure once mode convergence failed
OGMACAM_EVENT_ERROR              = 0x0080          # generic error
OGMACAM_EVENT_DISCONNECTED       = 0x0081          # camera disconnected
OGMACAM_EVENT_NOFRAMETIMEOUT     = 0x0082          # no frame timeout error
OGMACAM_EVENT_AFFEEDBACK         = 0x0083          # auto focus information feedback
OGMACAM_EVENT_FOCUSPOS           = 0x0084          # focus positon
OGMACAM_EVENT_NOPACKETTIMEOUT    = 0x0085          # no packet timeout
OGMACAM_EVENT_EXPO_START         = 0x4000          # hardware event: exposure start
OGMACAM_EVENT_EXPO_STOP          = 0x4001          # hardware event: exposure stop
OGMACAM_EVENT_TRIGGER_ALLOW      = 0x4002          # hardware event: next trigger allow
OGMACAM_EVENT_HEARTBEAT          = 0x4003          # hardware event: heartbeat, can be used to monitor whether the camera is alive
OGMACAM_EVENT_TRIGGER_IN         = 0x4004          # hardware event: trigger in
OGMACAM_EVENT_FACTORY            = 0x8001          # restore factory settings

OGMACAM_OPTION_NOFRAME_TIMEOUT        = 0x01       # no frame timeout: 0 => disable, positive value (>= NOFRAME_TIMEOUT_MIN) => timeout milliseconds. default: disable
OGMACAM_OPTION_THREAD_PRIORITY        = 0x02       # set the priority of the internal thread which grab data from the usb device.
                                                   #   Win: iValue: 0 = THREAD_PRIORITY_NORMAL; 1 = THREAD_PRIORITY_ABOVE_NORMAL; 2 = THREAD_PRIORITY_HIGHEST; 3 = THREAD_PRIORITY_TIME_CRITICAL; default: 1; see: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setthreadpriority
                                                   #   Linux & macOS: The high 16 bits for the scheduling policy, and the low 16 bits for the priority; see: https://linux.die.net/man/3/pthread_setschedparam
                                                   #
OGMACAM_OPTION_RAW                    = 0x04       # raw data mode, read the sensor "raw" data. This can be set only BEFORE Ogmacam_StartXXX(). 0 = rgb, 1 = raw, default value: 0
OGMACAM_OPTION_HISTOGRAM              = 0x05       # 0 = only one, 1 = continue mode
OGMACAM_OPTION_BITDEPTH               = 0x06       # 0 = 8 bits mode, 1 = 16 bits mode
OGMACAM_OPTION_FAN                    = 0x07       # 0 = turn off the cooling fan, [1, max] = fan speed
OGMACAM_OPTION_TEC                    = 0x08       # 0 = turn off the thermoelectric cooler, 1 = turn on the thermoelectric cooler
OGMACAM_OPTION_LINEAR                 = 0x09       # 0 = turn off the builtin linear tone mapping, 1 = turn on the builtin linear tone mapping, default value: 1
OGMACAM_OPTION_CURVE                  = 0x0a       # 0 = turn off the builtin curve tone mapping, 1 = turn on the builtin polynomial curve tone mapping, 2 = logarithmic curve tone mapping, default value: 2
OGMACAM_OPTION_TRIGGER                = 0x0b       # 0 = video mode, 1 = software or simulated trigger mode, 2 = external trigger mode, 3 = external + software trigger, default value = 0
OGMACAM_OPTION_RGB                    = 0x0c       # 0 => RGB24; 1 => enable RGB48 format when bitdepth > 8; 2 => RGB32; 3 => 8 Bits Grey (only for mono camera); 4 => 16 Bits Grey (only for mono camera when bitdepth > 8); 5 => 64(RGB64)
OGMACAM_OPTION_COLORMATIX             = 0x0d       # enable or disable the builtin color matrix, default value: 1
OGMACAM_OPTION_WBGAIN                 = 0x0e       # enable or disable the builtin white balance gain, default value: 1
OGMACAM_OPTION_TECTARGET              = 0x0f       # get or set the target temperature of the thermoelectric cooler, in 0.1 degree Celsius. For example, 125 means 12.5 degree Celsius, -35 means -3.5 degree Celsius
OGMACAM_OPTION_AUTOEXP_POLICY         = 0x10       # auto exposure policy:
                                                   #      0: Exposure Only
                                                   #      1: Exposure Preferred
                                                   #      2: Gain Only
                                                   #      3: Gain Preferred
                                                   #      default value: 1
                                                   #
OGMACAM_OPTION_FRAMERATE              = 0x11       # limit the frame rate, range=[0, 63], the default value 0 means no limit
OGMACAM_OPTION_DEMOSAIC               = 0x12       # demosaic method for both video and still image: BILINEAR = 0, VNG(Variable Number of Gradients) = 1, PPG(Patterned Pixel Grouping) = 2, AHD(Adaptive Homogeneity Directed) = 3, EA(Edge Aware) = 4, see https://en.wikipedia.org/wiki/Demosaicing, default value: 0
OGMACAM_OPTION_DEMOSAIC_VIDEO         = 0x13       # demosaic method for video
OGMACAM_OPTION_DEMOSAIC_STILL         = 0x14       # demosaic method for still image
OGMACAM_OPTION_BLACKLEVEL             = 0x15       # black level
OGMACAM_OPTION_MULTITHREAD            = 0x16       # multithread image processing
OGMACAM_OPTION_BINNING                = 0x17       # binning
                                                   #     0x01: (no binning)
                                                   #     n: (saturating add, n*n), 0x02(2*2), 0x03(3*3), 0x04(4*4), 0x05(5*5), 0x06(6*6), 0x07(7*7), 0x08(8*8). The Bitdepth of the data remains unchanged.
                                                   #     0x40 | n: (unsaturated add in RAW mode, n*n), 0x42(2*2), 0x43(3*3), 0x44(4*4), 0x45(5*5), 0x46(6*6), 0x47(7*7), 0x48(8*8). The Bitdepth of the data is increased. For example, the original data with bitdepth of 12 will increase the bitdepth by 2 bits and become 14 after 2*2 binning.
                                                   #     0x80 | n: (average, n*n), 0x82(2*2), 0x83(3*3), 0x84(4*4), 0x85(5*5), 0x86(6*6), 0x87(7*7), 0x88(8*8). The Bitdepth of the data remains unchanged.
                                                   # The final image size is rounded down to an even number, such as 640/3 to get 212
                                                   #
OGMACAM_OPTION_ROTATE                 = 0x18       # rotate clockwise: 0, 90, 180, 270
OGMACAM_OPTION_CG                     = 0x19       # Conversion Gain mode: 0 = LCG, 1 = HCG, 2 = HDR
OGMACAM_OPTION_PIXEL_FORMAT           = 0x1a       # pixel format
OGMACAM_OPTION_FFC                    = 0x1b       # flat field correction
                                                   #      set:
                                                   #          0: disable
                                                   #          1: enable
                                                   #          -1: reset
                                                   #          (0xff000000 | n): set the average number to n, [1~255]
                                                   #      get:
                                                   #          (val & 0xff): 0 => disable, 1 => enable, 2 => inited
                                                   #          ((val & 0xff00) >> 8): sequence
                                                   #          ((val & 0xff0000) >> 16): average number
                                                   #
OGMACAM_OPTION_DDR_DEPTH              = 0x1c       # the number of the frames that DDR can cache
                                                   #     1: DDR cache only one frame
                                                   #     0: Auto:
                                                   #         => one for video mode when auto exposure is enabled
                                                   #         => full capacity for others
                                                   #     1: DDR can cache frames to full capacity
                                                   #
OGMACAM_OPTION_DFC                    = 0x1d       # dark field correction
                                                   #     set:
                                                   #         0: disable
                                                   #         1: enable
                                                   #         -1: reset
                                                   #         (0xff000000 | n): set the average number to n, [1~255]
                                                   #     get:
                                                   #         (val & 0xff): 0 => disable, 1 => enable, 2 => inited
                                                   #         ((val & 0xff00) >> 8): sequence
                                                   #         ((val & 0xff0000) >> 16): average number
                                                   #
OGMACAM_OPTION_SHARPENING             = 0x1e       # Sharpening: (threshold << 24) | (radius << 16) | strength)
                                                   #     strength: [0, 500], default: 0 (disable)
                                                   #     radius: [1, 10]
                                                   #     threshold: [0, 255]
                                                   #
OGMACAM_OPTION_FACTORY                = 0x1f       # restore the factory settings
OGMACAM_OPTION_TEC_VOLTAGE            = 0x20       # get the current TEC voltage in 0.1V, 59 mean 5.9V; readonly
OGMACAM_OPTION_TEC_VOLTAGE_MAX        = 0x21       # TEC maximum voltage in 0.1V
OGMACAM_OPTION_DEVICE_RESET           = 0x22       # reset usb device, simulate a replug
OGMACAM_OPTION_UPSIDE_DOWN            = 0x23       # upsize down:
                                                   #     1: yes
                                                   #     0: no
                                                   #     default: 1 (win), 0 (linux/macos)
                                                   #
OGMACAM_OPTION_FOCUSPOS               = 0x24       # focus positon
OGMACAM_OPTION_AFMODE                 = 0x25       # auto focus mode (0:manul focus; 1:auto focus; 2:once focus; 3:conjugate calibration)
OGMACAM_OPTION_AFZONE                 = 0x26       # auto focus zone
OGMACAM_OPTION_AFFEEDBACK             = 0x27       # auto focus information feedback; 0:unknown; 1:focused; 2:focusing; 3:defocus; 4:up; 5:down
OGMACAM_OPTION_TESTPATTERN            = 0x28       # test pattern:
                                                   #     0: TestPattern Off
                                                   #     3: monochrome diagonal stripes
                                                   #     5: monochrome vertical stripes
                                                   #     7: monochrome horizontal stripes
                                                   #     9: chromatic diagonal stripes
                                                   #
OGMACAM_OPTION_AUTOEXP_THRESHOLD      = 0x29       # threshold of auto exposure, default value: 5, range = [2, 15]
OGMACAM_OPTION_BYTEORDER              = 0x2a       # Byte order, BGR or RGB: 0 => RGB, 1 => BGR, default value: 1(Win), 0(macOS, Linux, Android)
OGMACAM_OPTION_NOPACKET_TIMEOUT       = 0x2b       # no packet timeout: 0 => disable, positive value (>= NOPACKET_TIMEOUT_MIN) => timeout milliseconds. default: disable
OGMACAM_OPTION_MAX_PRECISE_FRAMERATE  = 0x2c       # get the precise frame rate maximum value in 0.1 fps, such as 115 means 11.5 fps. E_NOTIMPL means not supported
OGMACAM_OPTION_PRECISE_FRAMERATE      = 0x2d       # precise frame rate current value in 0.1 fps, range:[1~maximum]
OGMACAM_OPTION_BANDWIDTH              = 0x2e       # bandwidth, [1-100]%
OGMACAM_OPTION_RELOAD                 = 0x2f       # reload the last frame in trigger mode
OGMACAM_OPTION_CALLBACK_THREAD        = 0x30       # dedicated thread for callback
OGMACAM_OPTION_FRONTEND_DEQUE_LENGTH  = 0x31       # frontend (raw) frame buffer deque length, range: [2, 1024], default: 4
                                                   # All the memory will be pre-allocated when the camera starts, so, please attention to memory usage
                                                   #
OGMACAM_OPTION_FRAME_DEQUE_LENGTH     = 0x31       # alias of OGMACAM_OPTION_FRONTEND_DEQUE_LENGTH
OGMACAM_OPTION_MIN_PRECISE_FRAMERATE  = 0x32       # get the precise frame rate minimum value in 0.1 fps, such as 15 means 1.5 fps
OGMACAM_OPTION_SEQUENCER_ONOFF        = 0x33       # sequencer trigger: on/off
OGMACAM_OPTION_SEQUENCER_NUMBER       = 0x34       # sequencer trigger: number, range = [1, 255]
OGMACAM_OPTION_SEQUENCER_EXPOTIME     = 0x01000000 # sequencer trigger: exposure time, iOption = OGMACAM_OPTION_SEQUENCER_EXPOTIME | index, iValue = exposure time
                                                   #   For example, to set the exposure time of the third group to 50ms, call:
                                                   #     Ogmacam_put_Option(OGMACAM_OPTION_SEQUENCER_EXPOTIME | 3, 50000)
                                                   #
OGMACAM_OPTION_SEQUENCER_EXPOGAIN     = 0x02000000 # sequencer trigger: exposure gain, iOption = OGMACAM_OPTION_SEQUENCER_EXPOGAIN | index, iValue = gain
OGMACAM_OPTION_DENOISE                = 0x35       # denoise, strength range: [0, 100], 0 means disable
OGMACAM_OPTION_HEAT_MAX               = 0x36       # get maximum level: heat to prevent fogging up
OGMACAM_OPTION_HEAT                   = 0x37       # heat to prevent fogging up
OGMACAM_OPTION_LOW_NOISE              = 0x38       # low noise mode (Higher signal noise ratio, lower frame rate): 1 => enable
OGMACAM_OPTION_POWER                  = 0x39       # get power consumption, unit: milliwatt
OGMACAM_OPTION_GLOBAL_RESET_MODE      = 0x3a       # global reset mode
OGMACAM_OPTION_OPEN_USB_ERRORCODE     = 0x3b       # get the open usb error code
OGMACAM_OPTION_FLUSH                  = 0x3d       # 1 = hard flush, discard frames cached by camera DDR (if any)
                                                   # 2 = soft flush, discard frames cached by ogmacam.dll (if any)
                                                   # 3 = both flush
                                                   # Ogmacam_Flush means 'both flush'
                                                   # return the number of soft flushed frames if successful, HRESULT if failed
                                                   #
OGMACAM_OPTION_NUMBER_DROP_FRAME      = 0x3e       # get the number of frames that have been grabbed from the USB but dropped by the software
OGMACAM_OPTION_DUMP_CFG               = 0x3f       # 0 = when camera is stopped, do not dump configuration automatically
                                                   # 1 = when camera is stopped, dump configuration automatically
                                                   # -1 = explicitly dump configuration once
                                                   # default: 1
                                                   #
OGMACAM_OPTION_DEFECT_PIXEL           = 0x40       # Defect Pixel Correction: 0 => disable, 1 => enable; default: 1
OGMACAM_OPTION_BACKEND_DEQUE_LENGTH   = 0x41       # backend (pipelined) frame buffer deque length (Only available in pull mode), range: [2, 1024], default: 3
                                                   # All the memory will be pre-allocated when the camera starts, so, please attention to memory usage
                                                   #
OGMACAM_OPTION_LIGHTSOURCE_MAX        = 0x42       # get the light source range, [0 ~ max]
OGMACAM_OPTION_LIGHTSOURCE            = 0x43       # light source
OGMACAM_OPTION_HEARTBEAT              = 0x44       # Heartbeat interval in millisecond, range = [OGMACAM_HEARTBEAT_MIN, OGMACAM_HEARTBEAT_MAX], 0 = disable, default: disable
OGMACAM_OPTION_FRONTEND_DEQUE_CURRENT = 0x45       # get the current number in frontend deque
OGMACAM_OPTION_BACKEND_DEQUE_CURRENT  = 0x46       # get the current number in backend deque
OGMACAM_OPTION_EVENT_HARDWARE         = 0x04000000 # enable or disable hardware event: 0 => disable, 1 => enable; default: disable
                                                   #     (1) iOption = OGMACAM_OPTION_EVENT_HARDWARE, master switch for notification of all hardware events
                                                   #     (2) iOption = OGMACAM_OPTION_EVENT_HARDWARE | (event type), a specific type of sub-switch
                                                   # Only if both the master switch and the sub-switch of a particular type remain on are actually enabled for that type of event notification.
                                                   #
OGMACAM_OPTION_PACKET_NUMBER          = 0x47       # get the received packet number
OGMACAM_OPTION_FILTERWHEEL_SLOT       = 0x48       # filter wheel slot number
OGMACAM_OPTION_FILTERWHEEL_POSITION   = 0x49       # filter wheel position:
                                                   #     set:
                                                   #         -1: calibrate
                                                   #         val & 0xff: position between 0 and N-1, where N is the number of filter slots
                                                   #         (val >> 8) & 0x1: direction, 0 => clockwise spinning, 1 => auto direction spinning
                                                   #     get:
                                                   #         -1: in motion
                                                   #         val: position arrived
                                                   #
OGMACAM_OPTION_AUTOEXPOSURE_PERCENT   = 0x4a       # auto exposure percent to average:
                                                   #     1~99: peak percent average
                                                   #     0 or 100: full roi average
                                                   #
OGMACAM_OPTION_ANTI_SHUTTER_EFFECT    = 0x4b       # anti shutter effect: 1 => disable, 0 => disable; default: 1
OGMACAM_OPTION_CHAMBER_HT             = 0x4c       # get chamber humidity & temperature:
                                                   #     high 16 bits: humidity, in 0.1%, such as: 325 means humidity is 32.5%
                                                   #     low 16 bits: temperature, in 0.1 degrees Celsius, such as: 32 means 3.2 degrees Celsius
                                                   #
OGMACAM_OPTION_ENV_HT                 = 0x4d       # get environment humidity & temperature
OGMACAM_OPTION_EXPOSURE_PRE_DELAY     = 0x4e       # exposure signal pre-delay, microsecond
OGMACAM_OPTION_EXPOSURE_POST_DELAY    = 0x4f       # exposure signal post-delay, microsecond
OGMACAM_OPTION_AUTOEXPO_CONV          = 0x50       # get auto exposure convergence status: 1(YES) or 0(NO), -1(NA)
OGMACAM_OPTION_AUTOEXPO_TRIGGER       = 0x51       # auto exposure on trigger mode: 0 => disable, 1 => enable; default: 0
OGMACAM_OPTION_LINE_PRE_DELAY         = 0x52       # specified line signal pre-delay, microsecond
OGMACAM_OPTION_LINE_POST_DELAY        = 0x53       # specified line signal post-delay, microsecond
OGMACAM_OPTION_TEC_VOLTAGE_MAX_RANGE  = 0x54       # get the tec maximum voltage range:
                                                   #     high 16 bits: max
                                                   #     low 16 bits: min
OGMACAM_OPTION_HIGH_FULLWELL          = 0x55       # high fullwell capacity: 0 => disable, 1 => enable

OGMACAM_PIXELFORMAT_RAW8              = 0x00
OGMACAM_PIXELFORMAT_RAW10             = 0x01
OGMACAM_PIXELFORMAT_RAW12             = 0x02
OGMACAM_PIXELFORMAT_RAW14             = 0x03
OGMACAM_PIXELFORMAT_RAW16             = 0x04
OGMACAM_PIXELFORMAT_YUV411            = 0x05
OGMACAM_PIXELFORMAT_VUYY              = 0x06
OGMACAM_PIXELFORMAT_YUV444            = 0x07
OGMACAM_PIXELFORMAT_RGB888            = 0x08
OGMACAM_PIXELFORMAT_GMCY8             = 0x09
OGMACAM_PIXELFORMAT_GMCY12            = 0x0a
OGMACAM_PIXELFORMAT_UYVY              = 0x0b

OGMACAM_FRAMEINFO_FLAG_SEQ            = 0x0001   # frame sequence number
OGMACAM_FRAMEINFO_FLAG_TIMESTAMP      = 0x0002   # timestamp
OGMACAM_FRAMEINFO_FLAG_EXPOTIME       = 0x0004   # exposure time
OGMACAM_FRAMEINFO_FLAG_EXPOGAIN       = 0x0008   # exposure gain
OGMACAM_FRAMEINFO_FLAG_BLACKLEVEL     = 0x0010   # black level
OGMACAM_FRAMEINFO_FLAG_SHUTTERSEQ     = 0x0020   # sequence shutter counter
OGMACAM_FRAMEINFO_FLAG_STILL          = 0x8000   # still image

OGMACAM_IOCONTROLTYPE_GET_SUPPORTEDMODE         = 0x01  # 0x01 => Input, 0x02 => Output, (0x01 | 0x02) => support both Input and Output
OGMACAM_IOCONTROLTYPE_GET_GPIODIR               = 0x03  # 0x01 => Input, 0x02 => Output
OGMACAM_IOCONTROLTYPE_SET_GPIODIR               = 0x04
OGMACAM_IOCONTROLTYPE_GET_FORMAT                = 0x05  # 0x00 => not connected
                                                        # 0x01 => Tri-state: Tri-state mode (Not driven)
                                                        # 0x02 => TTL: TTL level signals
                                                        # 0x03 => LVDS: LVDS level signals
                                                        # 0x04 => RS422: RS422 level signals
                                                        # 0x05 => Opto-coupled
OGMACAM_IOCONTROLTYPE_SET_FORMAT                = 0x06
OGMACAM_IOCONTROLTYPE_GET_OUTPUTINVERTER        = 0x07  # boolean, only support output signal
OGMACAM_IOCONTROLTYPE_SET_OUTPUTINVERTER        = 0x08
OGMACAM_IOCONTROLTYPE_GET_INPUTACTIVATION       = 0x09  # 0x01 => Positive, 0x02 => Negative
OGMACAM_IOCONTROLTYPE_SET_INPUTACTIVATION       = 0x0a
OGMACAM_IOCONTROLTYPE_GET_DEBOUNCERTIME         = 0x0b  # debouncer time in microseconds, [0, 20000]
OGMACAM_IOCONTROLTYPE_SET_DEBOUNCERTIME         = 0x0c
OGMACAM_IOCONTROLTYPE_GET_TRIGGERSOURCE         = 0x0d  # 0x00 => Opto-isolated input
                                                        # 0x01 => GPIO0
                                                        # 0x02 => GPIO1
                                                        # 0x03 => Counter
                                                        # 0x04 => PWM
                                                        # 0x05 => Software
OGMACAM_IOCONTROLTYPE_SET_TRIGGERSOURCE         = 0x0e
OGMACAM_IOCONTROLTYPE_GET_TRIGGERDELAY          = 0x0f  # Trigger delay time in microseconds, [0, 5000000]
OGMACAM_IOCONTROLTYPE_SET_TRIGGERDELAY          = 0x10
OGMACAM_IOCONTROLTYPE_GET_BURSTCOUNTER          = 0x11  # Burst Counter, range: [1 ~ 65535]
OGMACAM_IOCONTROLTYPE_SET_BURSTCOUNTER          = 0x12
OGMACAM_IOCONTROLTYPE_GET_COUNTERSOURCE         = 0x13  # 0x00 => Opto-isolated input, 0x01 => GPIO0, 0x02 => GPIO1
OGMACAM_IOCONTROLTYPE_SET_COUNTERSOURCE         = 0x14
OGMACAM_IOCONTROLTYPE_GET_COUNTERVALUE          = 0x15  # Counter Value, range: [1 ~ 65535]
OGMACAM_IOCONTROLTYPE_SET_COUNTERVALUE          = 0x16
OGMACAM_IOCONTROLTYPE_SET_RESETCOUNTER          = 0x18
OGMACAM_IOCONTROLTYPE_GET_PWM_FREQ              = 0x19
OGMACAM_IOCONTROLTYPE_SET_PWM_FREQ              = 0x1a
OGMACAM_IOCONTROLTYPE_GET_PWM_DUTYRATIO         = 0x1b
OGMACAM_IOCONTROLTYPE_SET_PWM_DUTYRATIO         = 0x1c
OGMACAM_IOCONTROLTYPE_GET_PWMSOURCE             = 0x1d  # 0x00 => Opto-isolated input, 0x01 => GPIO0, 0x02 => GPIO1
OGMACAM_IOCONTROLTYPE_SET_PWMSOURCE             = 0x1e
OGMACAM_IOCONTROLTYPE_GET_OUTPUTMODE            = 0x1f  # 0x00 => Frame Trigger Wait
                                                        # 0x01 => Exposure Active
                                                        # 0x02 => Strobe
                                                        # 0x03 => User output
OGMACAM_IOCONTROLTYPE_SET_OUTPUTMODE            = 0x20
OGMACAM_IOCONTROLTYPE_GET_STROBEDELAYMODE       = 0x21  # boolean, 1 => delay, 0 => pre-delay; compared to exposure active signal
OGMACAM_IOCONTROLTYPE_SET_STROBEDELAYMODE       = 0x22
OGMACAM_IOCONTROLTYPE_GET_STROBEDELAYTIME       = 0x23  # Strobe delay or pre-delay time in microseconds, [0, 5000000]
OGMACAM_IOCONTROLTYPE_SET_STROBEDELAYTIME       = 0x24
OGMACAM_IOCONTROLTYPE_GET_STROBEDURATION        = 0x25  # Strobe duration time in microseconds, [0, 5000000]
OGMACAM_IOCONTROLTYPE_SET_STROBEDURATION        = 0x26
OGMACAM_IOCONTROLTYPE_GET_USERVALUE             = 0x27  # bit0 => Opto-isolated output
                                                        # bit1 => GPIO0 output
                                                        # bit2 => GPIO1 output
OGMACAM_IOCONTROLTYPE_SET_USERVALUE             = 0x28
OGMACAM_IOCONTROLTYPE_GET_UART_ENABLE           = 0x29  # enable: 1 => on; 0 => off
OGMACAM_IOCONTROLTYPE_SET_UART_ENABLE           = 0x2a
OGMACAM_IOCONTROLTYPE_GET_UART_BAUDRATE         = 0x2b  # baud rate: 0 => 9600; 1 => 19200; 2 => 38400; 3 => 57600; 4 => 115200
OGMACAM_IOCONTROLTYPE_SET_UART_BAUDRATE         = 0x2c
OGMACAM_IOCONTROLTYPE_GET_UART_LINEMODE         = 0x2d  # line mode: 0 => TX(GPIO_0)/RX(GPIO_1); 1 => TX(GPIO_1)/RX(GPIO_0)
OGMACAM_IOCONTROLTYPE_SET_UART_LINEMODE         = 0x2e
OGMACAM_IOCONTROLTYPE_GET_EXPO_ACTIVE_MODE      = 0x2f  # exposure time signal: 0 => specified line, 1 => common exposure time
OGMACAM_IOCONTROLTYPE_SET_EXPO_ACTIVE_MODE      = 0x30
OGMACAM_IOCONTROLTYPE_GET_EXPO_START_LINE       = 0x31  # exposure start line, default: 0
OGMACAM_IOCONTROLTYPE_SET_EXPO_START_LINE       = 0x32
OGMACAM_IOCONTROLTYPE_GET_EXPO_END_LINE         = 0x33  # exposure end line, default: 0
                                                        # end line must be no less than start line
OGMACAM_IOCONTROLTYPE_SET_EXPO_END_LINE         = 0x34
OGMACAM_IOCONTROLTYPE_GET_EXEVT_ACTIVE_MODE     = 0x35  # exposure event: 0 => specified line, 1 => common exposure time
OGMACAM_IOCONTROLTYPE_SET_EXEVT_ACTIVE_MODE     = 0x36

# hardware level range mode
OGMACAM_LEVELRANGE_MANUAL                       = 0x0000 # manual
OGMACAM_LEVELRANGE_ONCE                         = 0x0001 # once
OGMACAM_LEVELRANGE_CONTINUE                     = 0x0002 # continue
OGMACAM_LEVELRANGE_ROI                          = 0xffff # update roi rect only

# see rwc_Flash
OGMACAM_FLASH_SIZE      = 0x00    # query total size
OGMACAM_FLASH_EBLOCK    = 0x01    # query erase block size
OGMACAM_FLASH_RWBLOCK   = 0x02    # query read/write block size
OGMACAM_FLASH_STATUS    = 0x03    # query status
OGMACAM_FLASH_READ      = 0x04    # read
OGMACAM_FLASH_WRITE     = 0x05    # write
OGMACAM_FLASH_ERASE     = 0x06    # erase

# HRESULT: error code
S_OK            = 0x00000000 # Success
S_FALSE         = 0x00000001 # Yet another success
E_UNEXPECTED    = 0x8000ffff # Catastrophic failure
E_NOTIMPL       = 0x80004001 # Not supported or not implemented
E_ACCESSDENIED  = 0x80070005 # Permission denied
E_OUTOFMEMORY   = 0x8007000e # Out of memory
E_INVALIDARG    = 0x80070057 # One or more arguments are not valid
E_POINTER       = 0x80004003 # Pointer that is not valid
E_FAIL          = 0x80004005 # Generic failure
E_WRONG_THREAD  = 0x8001010e # Call function in the wrong thread
E_GEN_FAILURE   = 0x8007001f # Device not functioning
E_PENDING       = 0x8000000a # The data necessary to complete this operation is not yet available
E_TIMEOUT       = 0x8001011f # This operation returned because the timeout period expired

OGMACAM_EXPOGAIN_DEF             = 100      # exposure gain, default value
OGMACAM_EXPOGAIN_MIN             = 100      # exposure gain, minimum value
OGMACAM_TEMP_DEF                 = 6503     # color temperature, default value
OGMACAM_TEMP_MIN                 = 2000     # color temperature, minimum value
OGMACAM_TEMP_MAX                 = 15000    # color temperature, maximum value
OGMACAM_TINT_DEF                 = 1000     # tint
OGMACAM_TINT_MIN                 = 200      # tint
OGMACAM_TINT_MAX                 = 2500     # tint
OGMACAM_HUE_DEF                  = 0        # hue
OGMACAM_HUE_MIN                  = -180     # hue
OGMACAM_HUE_MAX                  = 180      # hue
OGMACAM_SATURATION_DEF           = 128      # saturation
OGMACAM_SATURATION_MIN           = 0        # saturation
OGMACAM_SATURATION_MAX           = 255      # saturation
OGMACAM_BRIGHTNESS_DEF           = 0        # brightness
OGMACAM_BRIGHTNESS_MIN           = -64      # brightness
OGMACAM_BRIGHTNESS_MAX           = 64       # brightness
OGMACAM_CONTRAST_DEF             = 0        # contrast
OGMACAM_CONTRAST_MIN             = -100     # contrast
OGMACAM_CONTRAST_MAX             = 100      # contrast
OGMACAM_GAMMA_DEF                = 100      # gamma
OGMACAM_GAMMA_MIN                = 20       # gamma
OGMACAM_GAMMA_MAX                = 180      # gamma
OGMACAM_AETARGET_DEF             = 120      # target of auto exposure
OGMACAM_AETARGET_MIN             = 16       # target of auto exposure
OGMACAM_AETARGET_MAX             = 220      # target of auto exposure
OGMACAM_WBGAIN_DEF               = 0        # white balance gain
OGMACAM_WBGAIN_MIN               = -127     # white balance gain
OGMACAM_WBGAIN_MAX               = 127      # white balance gain
OGMACAM_BLACKLEVEL_MIN           = 0        # minimum black level
OGMACAM_BLACKLEVEL8_MAX          = 31       # maximum black level for bit depth = 8
OGMACAM_BLACKLEVEL10_MAX         = 31 * 4   # maximum black level for bit depth = 10
OGMACAM_BLACKLEVEL12_MAX         = 31 * 16  # maximum black level for bit depth = 12
OGMACAM_BLACKLEVEL14_MAX         = 31 * 64  # maximum black level for bit depth = 14
OGMACAM_BLACKLEVEL16_MAX         = 31 * 256 # maximum black level for bit depth = 16
OGMACAM_SHARPENING_STRENGTH_DEF  = 0        # sharpening strength
OGMACAM_SHARPENING_STRENGTH_MIN  = 0        # sharpening strength
OGMACAM_SHARPENING_STRENGTH_MAX  = 500      # sharpening strength
OGMACAM_SHARPENING_RADIUS_DEF    = 2        # sharpening radius
OGMACAM_SHARPENING_RADIUS_MIN    = 1        # sharpening radius
OGMACAM_SHARPENING_RADIUS_MAX    = 10       # sharpening radius
OGMACAM_SHARPENING_THRESHOLD_DEF = 0        # sharpening threshold
OGMACAM_SHARPENING_THRESHOLD_MIN = 0        # sharpening threshold
OGMACAM_SHARPENING_THRESHOLD_MAX = 255      # sharpening threshold
OGMACAM_AUTOEXPO_THRESHOLD_DEF   = 5        # auto exposure threshold
OGMACAM_AUTOEXPO_THRESHOLD_MIN   = 2        # auto exposure threshold
OGMACAM_AUTOEXPO_THRESHOLD_MAX   = 15       # auto exposure threshold
OGMACAM_BANDWIDTH_DEF            = 100      # bandwidth
OGMACAM_BANDWIDTH_MIN            = 1        # bandwidth
OGMACAM_BANDWIDTH_MAX            = 100      # bandwidth
OGMACAM_DENOISE_DEF              = 0        # denoise
OGMACAM_DENOISE_MIN              = 0        # denoise
OGMACAM_DENOISE_MAX              = 100      # denoise
OGMACAM_TEC_TARGET_MIN           = -300     # TEC target: -30.0 degrees Celsius
OGMACAM_TEC_TARGET_DEF           = 0        # TEC target: 0.0 degrees Celsius
OGMACAM_TEC_TARGET_MAX           = 400      # TEC target: 40.0 degrees Celsius
OGMACAM_HEARTBEAT_MIN            = 100      # millisecond
OGMACAM_HEARTBEAT_MAX            = 10000    # millisecond
OGMACAM_AE_PERCENT_MIN           = 0        # auto exposure percent, 0 => full roi average
OGMACAM_AE_PERCENT_MAX           = 100
OGMACAM_AE_PERCENT_DEF           = 10
OGMACAM_NOPACKET_TIMEOUT_MIN     = 500      # no packet timeout minimum: 500ms
OGMACAM_NOFRAME_TIMEOUT_MIN      = 500      # no frame timeout minimum: 500ms

def TDIBWIDTHBYTES(bits):
    return ((bits + 31) // 32 * 4)

"""
------------------------------------------------------------------|
| Parameter               |   Range       |   Default             |
|-----------------------------------------------------------------|
| Auto Exposure Target    |   16~235      |   120                 |
| Exposure Gain           |   100~        |   100                 |
| Temp                    |   2000~15000  |   6503                |
| Tint                    |   200~2500    |   1000                |
| LevelRange              |   0~255       |   Low = 0, High = 255 |
| Contrast                |   -100~100    |   0                   |
| Hue                     |   -180~180    |   0                   |
| Saturation              |   0~255       |   128                 |
| Brightness              |   -64~64      |   0                   |
| Gamma                   |   20~180      |   100                 |
| WBGain                  |   -127~127    |   0                   |
------------------------------------------------------------------|
"""

class OgmacamResolution:
    def __init__(self, w, h):
        self.width = w
        self.height = h

class OgmacamAfParam:
    def __init__(self, imax, imin, idef, imaxabs, iminabs, zoneh, zonev):
        self.imax = imax                 # maximum auto focus sensor board positon
        self.imin = imin                 # minimum auto focus sensor board positon
        self.idef = idef                 # conjugate calibration positon
        self.imaxabs = imaxabs           # maximum absolute auto focus sensor board positon, micrometer
        self.iminabs = iminabs           # maximum absolute auto focus sensor board positon, micrometer
        self.zoneh = zoneh               # zone horizontal
        self.zonev = zonev               # zone vertical

class OgmacamFrameInfoV3:
    def __init__(self):
        self.width = 0
        self.height = 0
        self.flag = 0                    # OGMACAM_FRAMEINFO_FLAG_xxxx
        self.seq = 0                     # frame sequence number
        self.timestamp = 0               # microsecond
        self.shutterseq = 0              # sequence shutter counter
        self.expotime = 0                # expotime
        self.expogain = 0                # expogain
        self.blacklevel = 0              # black level

class OgmacamFrameInfoV2:
    def __init__(self):
        self.width = 0
        self.height = 0
        self.flag = 0                    # OGMACAM_FRAMEINFO_FLAG_xxxx
        self.seq = 0                     # frame sequence number
        self.timestamp = 0               # microsecond

class OgmacamModelV2:                    # camera model v2
    def __init__(self, name, flag, maxspeed, preview, still, maxfanspeed, ioctrol, xpixsz, ypixsz, res):
        self.name = name                 # model name, in Windows, we use unicode
        self.flag = flag                 # OGMACAM_FLAG_xxx, 64 bits
        self.maxspeed = maxspeed         # number of speed level, same as Ogmacam_get_MaxSpeed(), the speed range = [0, maxspeed], closed interval
        self.preview = preview           # number of preview resolution, same as Ogmacam_get_ResolutionNumber()
        self.still = still               # number of still resolution, same as Ogmacam_get_StillResolutionNumber()
        self.maxfanspeed = maxfanspeed   # maximum fan speed, fan speed range = [0, max], closed interval
        self.ioctrol = ioctrol           # number of input/output control
        self.xpixsz = xpixsz             # physical pixel size
        self.ypixsz = ypixsz             # physical pixel size
        self.res = res                   # OgmacamResolution

class OgmacamDeviceV2:
    def __init__(self, displayname, id, model):
        self.displayname = displayname   # display name
        self.id = id                     # unique and opaque id of a connected camera, for Ogmacam_Open
        self.model = model               # OgmacamModelV2

if sys.platform == 'win32':
    class HRESULTException(OSError):
        def __init__(self, hr):
            OSError.__init__(self, None, ctypes.FormatError(hr).strip(), None, hr)
else:
    class HRESULTException(Exception):
        def __init__(self, hr):
            self.hr = hr

class _Resolution(ctypes.Structure):
    _fields_ = [('width', ctypes.c_uint),
                ('height', ctypes.c_uint)]

if sys.platform == 'win32':
    class _ModelV2(ctypes.Structure):                      # camera model v2 win32
        _fields_ = [('name', ctypes.c_wchar_p),            # model name, in Windows, we use unicode
                    ('flag', ctypes.c_ulonglong),          # OGMACAM_FLAG_xxx, 64 bits
                    ('maxspeed', ctypes.c_uint),           # number of speed level, same as Ogmacam_get_MaxSpeed(), the speed range = [0, maxspeed], closed interval
                    ('preview', ctypes.c_uint),            # number of preview resolution, same as Ogmacam_get_ResolutionNumber()
                    ('still', ctypes.c_uint),              # number of still resolution, same as Ogmacam_get_StillResolutionNumber()
                    ('maxfanspeed', ctypes.c_uint),        # maximum fan speed, fan speed range = [0, max], closed interval
                    ('ioctrol', ctypes.c_uint),            # number of input/output control
                    ('xpixsz', ctypes.c_float),            # physical pixel size
                    ('ypixsz', ctypes.c_float),            # physical pixel size
                    ('res', _Resolution * 16)]
    class _DeviceV2(ctypes.Structure):                     # win32
        _fields_ = [('displayname', ctypes.c_wchar * 64),  # display name
                    ('id', ctypes.c_wchar * 64),           # unique and opaque id of a connected camera, for Ogmacam_Open
                    ('model', ctypes.POINTER(_ModelV2))]
else:
    class _ModelV2(ctypes.Structure):                      # camera model v2 linux/mac
        _fields_ = [('name', ctypes.c_char_p),             # model name
                    ('flag', ctypes.c_ulonglong),          # OGMACAM_FLAG_xxx, 64 bits
                    ('maxspeed', ctypes.c_uint),           # number of speed level, same as Ogmacam_get_MaxSpeed(), the speed range = [0, maxspeed], closed interval
                    ('preview', ctypes.c_uint),            # number of preview resolution, same as Ogmacam_get_ResolutionNumber()
                    ('still', ctypes.c_uint),              # number of still resolution, same as Ogmacam_get_StillResolutionNumber()
                    ('maxfanspeed', ctypes.c_uint),        # maximum fan speed
                    ('ioctrol', ctypes.c_uint),            # number of input/output control
                    ('xpixsz', ctypes.c_float),            # physical pixel size
                    ('ypixsz', ctypes.c_float),            # physical pixel size
                    ('res', _Resolution * 16)]
    class _DeviceV2(ctypes.Structure):                     # linux/mac
        _fields_ = [('displayname', ctypes.c_char * 64),   # display name
                    ('id', ctypes.c_char * 64),            # unique and opaque id of a connected camera, for Ogmacam_Open
                    ('model', ctypes.POINTER(_ModelV2))]

class Ogmacam:
    class __RECT(ctypes.Structure):
        _fields_ = [('left', ctypes.c_int),
                    ('top', ctypes.c_int),
                    ('right', ctypes.c_int),
                    ('bottom', ctypes.c_int)]

    class __AfParam(ctypes.Structure):
        _fields_ = [('imax', ctypes.c_int),                # maximum auto focus sensor board positon
                    ('imin', ctypes.c_int),                # minimum auto focus sensor board positon
                    ('idef', ctypes.c_int),                # conjugate calibration positon
                    ('imaxabs', ctypes.c_int),             # maximum absolute auto focus sensor board positon, micrometer
                    ('iminabs', ctypes.c_int),             # maximum absolute auto focus sensor board positon, micrometer
                    ('zoneh', ctypes.c_int),               # zone horizontal
                    ('zonev', ctypes.c_int)]               # zone vertical

    class __FrameInfoV3(ctypes.Structure):
        _fields_ = [('width', ctypes.c_uint),
                    ('height', ctypes.c_uint),
                    ('flag', ctypes.c_uint),               # OGMACAM_FRAMEINFO_FLAG_xxxx
                    ('seq', ctypes.c_uint),                # frame sequence number
                    ('timestamp', ctypes.c_longlong),      # microsecond
                    ('shutterseq', ctypes.c_uint),         # sequence shutter counter
                    ('expotime', ctypes.c_uint),           # expotime
                    ('expogain', ctypes.c_ushort),         # expogain
                    ('blacklevel', ctypes.c_ushort)]       # black level

    class __FrameInfoV2(ctypes.Structure):
        _fields_ = [('width', ctypes.c_uint),
                    ('height', ctypes.c_uint),
                    ('flag', ctypes.c_uint),               # OGMACAM_FRAMEINFO_FLAG_xxxx
                    ('seq', ctypes.c_uint),                # frame sequence number
                    ('timestamp', ctypes.c_longlong)]      # microsecond

    if sys.platform == 'win32':
        __EVENT_CALLBACK = ctypes.WINFUNCTYPE(None, ctypes.c_uint, ctypes.py_object)
        __PROGRESS_CALLBACK = ctypes.WINFUNCTYPE(None, ctypes.c_int, ctypes.py_object)
    else:
        __EVENT_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_uint, ctypes.py_object)
        __PROGRESS_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.py_object)
        __HOTPLUG_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
        __hotplug = None

    __lib = None
    __progress = None

    @staticmethod
    def __errcheck(result, fun, args):
        if result < 0:
            raise HRESULTException(result)
        return args

    @staticmethod
    def __convertStr(x):
        if isinstance(x, str):
            return x
        else:
            return x.decode('ascii')

    @classmethod
    def Version(cls):
        """get the version of this dll, which is: 53.22081.20230207"""
        cls.__initlib()
        return cls.__lib.Ogmacam_Version()

    @staticmethod
    def __convertResolution(a):
        t = []
        for i in range(0, a.preview):
            t.append(OgmacamResolution(a.res[i].width, a.res[i].height))
        return t

    @staticmethod
    def __convertModel(a):
        t = OgmacamModelV2(__class__.__convertStr(a.name), a.flag, a.maxspeed, a.preview, a.still, a.maxfanspeed, a.ioctrol, a.xpixsz, a.ypixsz, __class__.__convertResolution(a))
        return t

    @staticmethod
    def __convertDevice(a):
        return OgmacamDeviceV2(__class__.__convertStr(a.displayname), __class__.__convertStr(a.id), __class__.__convertModel(a.model.contents))

    @staticmethod
    def __hotplugCallbackFun(ctx):
        if __class__.__hotplug:
            __class__.__hotplug()

    @classmethod
    def HotPlug(cls, fun):
        """
        Only available on macOS and Linux, it's unnecessary on Windows & Android. To process the device plug in / pull out:
            (1) On Windows, please refer to the MSDN
                (a) Device Management, https://docs.microsoft.com/en-us/windows/win32/devio/device-management
                (b) Detecting Media Insertion or Removal, https://docs.microsoft.com/en-us/windows/win32/devio/detecting-media-insertion-or-removal
            (2) On Android, please refer to https://developer.android.com/guide/topics/connectivity/usb/host
            (3) On Linux / macOS, please call this function to register the callback function.
                When the device is inserted or pulled out, you will be notified by the callback funcion, and then call Ogmacam_EnumV2(...) again to enum the cameras.
            (4) On macOS, IONotificationPortCreate series APIs can also be used as an alternative.
        """
        if sys.platform == 'win32' or sys.platform == 'android':
            raise HRESULTException(0x80004001)
        else:
            cls.__initlib()
            cls.__hotplug = fun
            if cls.__hotplug is None:
                cls.__lib.Ogmacam_HotPlug(None, None)
            else:
                cls.__lib.Ogmacam_HotPlug(cls.__HOTPLUG_CALLBACK(cls.__hotplugCallbackFun), None)

    @classmethod
    def EnumV2(cls):
        cls.__initlib()
        a = (_DeviceV2 * OGMACAM_MAX)()
        n = cls.__lib.Ogmacam_EnumV2(a)
        arr = []
        for i in range(0, n):
            arr.append(cls.__convertDevice(a[i]))
        return arr

    def __init__(self, h):
        """the object of Ogmacam must be obtained by classmethod Open or OpenByIndex, it cannot be obtained by obj = ogmacam.Ogmacam()"""
        self.__h = h
        self.__fun = None
        self.__ctx = None
        self.__cb = None

    def __del__(self):
        self.Close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.Close()

    def __nonzero__(self):
        return self.__h is not None

    def __bool__(self):
        return self.__h is not None

    @classmethod
    def Open(cls, id):
        """
        the object of Ogmacam must be obtained by classmethod Open or OpenByIndex, it cannot be obtained by obj = ogmacam.Ogmacam()
        Open(None) means try to Open the first enumerated camera
        """
        cls.__initlib()
        if id is None:
            h = cls.__lib.Ogmacam_Open(None)
        elif sys.platform == 'win32':
            h = cls.__lib.Ogmacam_Open(id)
        else:
            h = cls.__lib.Ogmacam_Open(id.encode('ascii'))
        if h is None:
            return None
        return __class__(h)

    @classmethod
    def OpenByIndex(cls, index):
        """
        the object of Ogmacam must be obtained by classmethod Open or OpenByIndex, it cannot be obtained by obj = ogmacam.Ogmacam()

        the same with Ogmacam_Open, but use the index as the parameter. such as:
        index == 0, open the first camera,
        index == 1, open the second camera,
        etc
        """
        cls.__initlib()
        h = cls.__lib.Ogmacam_OpenByIndex(index)
        if h is None:
            return None
        return __class__(h)

    def Close(self):
        if self.__h:
            self.__lib.Ogmacam_Close(self.__h)
            self.__h = None

    @staticmethod
    def __eventCallbackFun(nEvent, ctx):
        if ctx:
            ctx.__callbackFun(nEvent)

    def __callbackFun(self, nEvent):
        if self.__fun:
            self.__fun(nEvent, self.__ctx)

    def StartPullModeWithCallback(self, fun, ctx):
        self.__fun = fun
        self.__ctx = ctx
        self.__cb = __class__.__EVENT_CALLBACK(__class__.__eventCallbackFun)
        self.__lib.Ogmacam_StartPullModeWithCallback(self.__h, self.__cb, ctypes.py_object(self))

    @staticmethod
    def __convertFrameInfoV3(pInfo, x):
        pInfo.width = x.width
        pInfo.height = x.height
        pInfo.flag = x.flag
        pInfo.seq = x.seq
        pInfo.shutterseq = x.shutterseq
        pInfo.timestamp = x.timestamp
        pInfo.expotime = x.expotime
        pInfo.expogain = x.expogain
        pInfo.blacklevel = x.blacklevel

    @staticmethod
    def __convertFrameInfoV2(pInfo, x):
        pInfo.width = x.width
        pInfo.height = x.height
        pInfo.flag = x.flag
        pInfo.seq = x.seq
        pInfo.timestamp = x.timestamp

    def PullImageV3(self, pImageData, bStill, bits, rowPitch, pInfo):
        """
        bStill: to pull still image, set to 1, otherwise 0
        bits: 24 (RGB24), 32 (RGB32), 48 (RGB48), 8 (Grey), 16 (Grey), 64 (RGB64).
              In RAW mode, this parameter is ignored.
              bits = 0 means using default bits (see OGMACAM_OPTION_RGB).
              When bits and OGMACAM_OPTION_RGB are inconsistent, format conversion will have to be performed, resulting in loss of efficiency.
              See the following bits and OGMACAM_OPTION_RGB correspondence table:
                ----------------------------------------------------------------------------------------------------------------------
                | OGMACAM_OPTION_RGB |   0 (RGB24)   |   1 (RGB48)   |   2 (RGB32)   |   3 (Grey8)   |  4 (Grey16)   |   5 (RGB64)   |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 0           |      24       |       48      |      32       |       8       |       16      |       64      |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 24          |      24       |       NA      | Convert to 24 | Convert to 24 |       NA      |       NA      |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 32          | Convert to 32 |       NA      |       32      | Convert to 32 |       NA      |       NA      |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 48          |      NA       |       48      |       NA      |       NA      | Convert to 48 | Convert to 48 |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 8           | Convert to 8  |       NA      | Convert to 8  |       8       |       NA      |       NA      |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
                | bits = 16          |      NA       | Convert to 16 |       NA      |       NA      |       16      | Convert to 16 |
                |--------------------|---------------|-----------|-------------------|---------------|---------------|---------------|
                | bits = 64          |      NA       | Convert to 64 |       NA      |       NA      | Convert to 64 |       64      |
                |--------------------|---------------|---------------|---------------|---------------|---------------|---------------|
        
        rowPitch: The distance from one row to the next row. rowPitch = 0 means using the default row pitch. rowPitch = -1 means zero padding, see below:
                ----------------------------------------------------------------------------------------------
                | format                             | 0 means default row pitch     | -1 means zero padding |
                |------------------------------------|-------------------------------|-----------------------|
                | RGB       | RGB24                  | TDIBWIDTHBYTES(24 * Width)    | Width * 3             |
                |           | RGB32                  | Width * 4                     | Width * 4             |
                |           | RGB48                  | TDIBWIDTHBYTES(48 * Width)    | Width * 6             |
                |           | GREY8                  | TDIBWIDTHBYTES(8 * Width)     | Width                 |
                |           | GREY16                 | TDIBWIDTHBYTES(16 * Width)    | Width * 2             |
                |           | RGB64                  | Width * 8                     | Width * 8             |
                |-----------|------------------------|-------------------------------|-----------------------|
                | RAW       | 8bits Mode             | Width                         | Width                 |
                |           | 10/12/14/16bits Mode   | Width * 2                     | Width * 2             |
                |-----------|------------------------|-------------------------------|-----------------------|
        """
        if pInfo is None:
            self.__lib.Ogmacam_PullImageV3(self.__h, pImageData, bStill, bits, rowPitch, None)
        else:
            x = self.__FrameInfoV3()
            self.__lib.Ogmacam_PullImageV3(self.__h, pImageData, bStill, bits, rowPitch, ctypes.byref(x))
            self.__convertFrameInfoV3(pInfo, x)

    def PullImageV2(self, pImageData, bits, pInfo):
        if pInfo is None:
            self.__lib.Ogmacam_PullImageV2(self.__h, pImageData, bits, None)
        else:
            x = self.__FrameInfoV2()
            self.__lib.Ogmacam_PullImageV2(self.__h, pImageData, bits, ctypes.byref(x))
            self.__convertFrameInfoV2(pInfo, x)

    def PullStillImageV2(self, pImageData, bits, pInfo):
        if pInfo is None:
            self.__lib.Ogmacam_PullStillImageV2(self.__h, pImageData, bits, None)
        else:
            x = self.__FrameInfoV2()
            self.__lib.Ogmacam_PullStillImageV2(self.__h, pImageData, bits, ctypes.byref(x))
            self.__convertFrameInfoV2(pInfo, x)

    def PullImageWithRowPitchV2(self, pImageData, bits, rowPitch, pInfo):
        if pInfo is None:
            self.__lib.Ogmacam_PullImageWithRowPitchV2(self.__h, pImageData, bits, rowPitch, None)
        else:
            x = self.__FrameInfoV2()
            self.__lib.Ogmacam_PullImageWithRowPitchV2(self.__h, pImageData, bits, rowPitch, ctypes.byref(x))
            self.__convertFrameInfoV2(pInfo, x)

    def PullStillImageWithRowPitchV2(self, pImageData, bits, rowPitch, pInfo):
        if pInfo is None:
            self.__lib.Ogmacam_PullStillImageWithRowPitchV2(self.__h, pImageData, bits, rowPitch, None)
        else:
            x = self.__FrameInfoV2()
            self.__lib.Ogmacam_PullStillImageWithRowPitchV2(self.__h, pImageData, bits, rowPitch, ctypes.byref(x))
            self.__convertFrameInfoV2(pInfo, x)

    def ResolutionNumber(self):
        return self.__lib.Ogmacam_get_ResolutionNumber(self.__h)

    def StillResolutionNumber(self):
        """return (width, height)"""
        return self.__lib.Ogmacam_get_StillResolutionNumber(self.__h)

    def MonoMode(self):
        return (self.__lib.Ogmacam_get_MonoMode(self.__h) == 0)

    def MaxSpeed(self):
        """get the maximum speed, 'Frame Speed Level'"""
        return self.__lib.Ogmacam_get_MaxSpeed(self.__h)

    def MaxBitDepth(self):
        """get the max bit depth of this camera, such as 8, 10, 12, 14, 16"""
        return self.__lib.Ogmacam_get_MaxBitDepth(self.__h)

    def FanMaxSpeed(self):
        """get the maximum fan speed, the fan speed range = [0, max], closed interval"""
        return self.__lib.Ogmacam_get_FanMaxSpeed(self.__h)

    def Revision(self):
        """get the revision"""
        x = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_Revision(self.__h, ctypes.byref(x))
        return x.value

    def SerialNumber(self):
        """get the serial number which is always 32 chars which is zero-terminated such as: TP110826145730ABCD1234FEDC56787"""
        str = (ctypes.c_char * 32)()
        self.__lib.Ogmacam_get_SerialNumber(self.__h, str)
        return str.value.decode('ascii')

    def FwVersion(self):
        """get the camera firmware version, such as: 3.2.1.20140922"""
        str = (ctypes.c_char * 16)()
        self.__lib.Ogmacam_get_FwVersion(self.__h, str)
        return str.value.decode('ascii')

    def HwVersion(self):
        """get the camera hardware version, such as: 3.2.1.20140922"""
        str = (ctypes.c_char * 16)()
        self.__lib.Ogmacam_get_HwVersion(self.__h, str)
        return str.value.decode('ascii')

    def ProductionDate(self):
        """such as: 20150327"""
        str = (ctypes.c_char * 16)()
        self.__lib.Ogmacam_get_ProductionDate(self.__h, str)
        return str.value.decode('ascii')

    def FpgaVersion(self):
        str = (ctypes.c_char * 16)()
        self.__lib.Ogmacam_get_FpgaVersion(self.__h, str)
        return str.value.decode('ascii')

    def Field(self):
        return self.__lib.Ogmacam_get_Field(self.__h)

    def Stop(self):
        self.__lib.Ogmacam_Stop(self.__h)

    def Pause(self, bPause):
        '''1 => pause, 0 => continue'''
        self.__lib.Ogmacam_Pause(self.__h, ctypes.c_int(1 if bPause else 0))

    def Snap(self, nResolutionIndex):
        """still image snap, nResolutionIndex = 0xffffffff means use the cureent preview resolution"""
        self.__lib.Ogmacam_Snap(self.__h, ctypes.c_uint(nResolutionIndex))

    def SnapN(self, nResolutionIndex, nNumber):
        """multiple still image snap, nResolutionIndex = 0xffffffff means use the cureent preview resolution"""
        self.__lib.Ogmacam_SnapN(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.c_uint(nNumber))

    def SnapR(self, nResolutionIndex, nNumber):
        """multiple RAW still image snap, nResolutionIndex = 0xffffffff means use the cureent preview resolution"""
        self.__lib.Ogmacam_SnapR(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.c_uint(nNumber))

    def Trigger(self, nNumber):
        """
        soft trigger:
        nNumber:    0xffff:     trigger continuously
                    0:          cancel trigger
                    others:     number of images to be triggered
        """
        self.__lib.Ogmacam_Trigger(self.__h, ctypes.c_ushort(nNumber))

    def put_Size(self, nWidth, nHeight):
        self.__lib.Ogmacam_put_Size(self.__h, ctypes.c_int(nWidth), ctypes.c_int(nHeight))

    def get_Size(self):
        """return (width, height)"""
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Size(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def put_eSize(self, nResolutionIndex):
        """
        put_Size, put_eSize, can be used to set the video output resolution BEFORE Start.
        put_Size use width and height parameters, put_eSize use the index parameter.
        for example, UCMOS03100KPA support the following resolutions:
            index 0:    2048,   1536
            index 1:    1024,   768
            index 2:    680,    510
        so, we can use put_Size(h, 1024, 768) or put_eSize(h, 1). Both have the same effect.
        """
        self.__lib.Ogmacam_put_eSize(self.__h, ctypes.c_uint(nResolutionIndex))

    def get_eSize(self):
        x = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_eSize(self.__h, ctypes.byref(x))
        return x.value

    def get_FinalSize(self):
        """final size after ROI, rotate, binning"""
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_FinalSize(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_Resolution(self, nResolutionIndex):
        """return (width, height)"""
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Resolution(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_PixelSize(self, nResolutionIndex):
        """get the sensor pixel size, such as: 2.4um x 2.4um"""
        x = ctypes.c_float(0)
        y = ctypes.c_float(0)
        self.__lib.Ogmacam_get_PixelSize(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_ResolutionRatio(self, nResolutionIndex):
        """numerator/denominator, such as: 1/1, 1/2, 1/3"""
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_ResolutionRatio(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_RawFormat(self):
        """
        see: http://www.fourcc.org
        FourCC:
            MAKEFOURCC('G', 'B', 'R', 'G'), see http://www.siliconimaging.com/RGB%20Bayer.htm
            MAKEFOURCC('R', 'G', 'G', 'B')
            MAKEFOURCC('B', 'G', 'G', 'R')
            MAKEFOURCC('G', 'R', 'B', 'G')
            MAKEFOURCC('Y', 'Y', 'Y', 'Y'), monochromatic sensor
            MAKEFOURCC('Y', '4', '1', '1'), yuv411
            MAKEFOURCC('V', 'U', 'Y', 'Y'), yuv422
            MAKEFOURCC('U', 'Y', 'V', 'Y'), yuv422
            MAKEFOURCC('Y', '4', '4', '4'), yuv444
            MAKEFOURCC('R', 'G', 'B', '8'), RGB888
        """
        x = ctypes.c_uint(0)
        y = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_RawFormat(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def put_RealTime(self, val):
        """
        0: stop grab frame when frame buffer deque is full, until the frames in the queue are pulled away and the queue is not full
        1: realtime
            use minimum frame buffer. When new frame arrive, drop all the pending frame regardless of whether the frame buffer is full.
            If DDR present, also limit the DDR frame buffer to only one frame.
        2: soft realtime
            Drop the oldest frame when the queue is full and then enqueue the new frame
        default: 0
        """
        self.__lib.Ogmacam_put_RealTime(self.__h, val)

    def get_RealTime(self):
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_RealTime(self.__h, b)
        return b.value

    def Flush():
        """Flush is obsolete, recommend using put_Option(h, OGMACAM_OPTION_FLUSH, 3)"""
        self.__lib.Ogmacam_Flush(self.__h)

    def get_AutoExpoEnable(self):
        """
        bAutoExposure:
           0: disable auto exposure
           1: auto exposure continue mode
           2: auto exposure once mode
        """
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_AutoExpoEnable(self.__h, b)
        return b.value

    def put_AutoExpoEnable(self, bAutoExposure):
        """
        bAutoExposure:
           0: disable auto exposure
           1: auto exposure continue mode
           2: auto exposure once mode
        """
        self.__lib.Ogmacam_put_AutoExpoEnable(self.__h, ctypes.c_int(bAutoExposure))

    def get_AutoExpoTarget(self):
        x = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_AutoExpoTarget(self.__h, ctypes.byref(x))
        return x.value

    def put_AutoExpoTarget(self, Target):
        self.__lib.Ogmacam_put_AutoExpoTarget(self.__h, ctypes.c_int(Target))

    def put_MaxAutoExpoTimeAGain(self, maxTime, maxGain):
        return self.__lib.Ogmacam_put_MaxAutoExpoTimeAGain(self.__h, ctypes.c_uint(maxTime), ctypes.c_ushort(maxGain))

    def get_MaxAutoExpoTimeAGain(self):
        x = ctypes.c_uint(0)
        y = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_MaxAutoExpoTimeAGain(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def put_MinAutoExpoTimeAGain(self, minTime, minGain):
        self.__lib.Ogmacam_put_MinAutoExpoTimeAGain(self.__h, ctypes.c_uint(minTime), ctypes.c_ushort(minGain))

    def get_MinAutoExpoTimeAGain(self):
        x = ctypes.c_uint(0)
        y = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_MinAutoExpoTimeAGain(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def get_ExpoTime(self):
        """in microseconds"""
        x = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_ExpoTime(self.__h, ctypes.byref(x))
        return x.value

    def put_ExpoTime(self, Time):
        self.__lib.Ogmacam_put_ExpoTime(self.__h, ctypes.c_uint(Time))

    def get_ExpTimeRange(self):
        x = ctypes.c_uint(0)
        y = ctypes.c_uint(0)
        z = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_ExpTimeRange(self.__h, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)

    def get_ExpoAGain(self):
        """percent, such as 300"""
        x = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_ExpoAGain(self.__h, ctypes.byref(x))
        return x.value

    def put_ExpoAGain(self, Gain):
        self.__lib.Ogmacam_put_ExpoAGain(self.__h, ctypes.c_ushort(Gain))

    def get_ExpoAGainRange(self):
        """ return (min, max, default)"""
        x = ctypes.c_ushort(0)
        y = ctypes.c_ushort(0)
        z = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_ExpoAGainRange(self.__h, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)

    def put_LevelRange(self, aLow, aHigh):
        if len(aLow) == 4 and len(aHigh) == 4:
            x = (ctypes.c_ushort * 4)(aLow[0], aLow[1], aLow[2], aLow[3])
            y = (ctypes.c_ushort * 4)(aHigh[0], aHigh[1], aHigh[2], aHigh[3])
            self.__lib.Ogmacam_put_LevelRange(self.__h, x, y)
        else:
            raise HRESULTException(0x80070057)

    def get_LevelRange(self):
        x = (ctypes.c_ushort * 4)()
        y = (ctypes.c_ushort * 4)()
        self.__lib.Ogmacam_get_LevelRange(self.__h, x, y)
        aLow = (x[0], x[1], x[2], x[3])
        aHigh = (y[0], y[1], y[2], y[3])
        return (aLow, aHigh)

    def put_LevelRangeV2(self, mode, roiX, roiY, roiWidth, roiHeight, aLow, aHigh):
        if len(aLow) == 4 and len(aHigh) == 4:
            x = (ctypes.c_ushort * 4)(aLow[0], aLow[1], aLow[2], aLow[3])
            y = (ctypes.c_ushort * 4)(aHigh[0], aHigh[1], aHigh[2], aHigh[3])
            rc = self.__RECT()
            rc.left = roiX
            rc.right = roiX + roiWidth
            rc.top = roiY
            rc.bottom = roiY + roiHeight
            self.__lib.Ogmacam_put_LevelRangeV2(self.__h, mode, ctypes.byref(rc), x, y)
        else:
            raise HRESULTException(0x80070057)

    def get_LevelRangeV2(self):
        mode = ctypes.c_ushort(0)
        x = (ctypes.c_ushort * 4)()
        y = (ctypes.c_ushort * 4)()
        rc = self.__RECT()
        self.__lib.Ogmacam_get_LevelRange(self.__h, mode, ctypes.byref(rc), x, y)
        aLow = (x[0], x[1], x[2], x[3])
        aHigh = (y[0], y[1], y[2], y[3])
        return (mode, (rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top), aLow, aHigh)

    def put_Hue(self, Hue):
        self.__lib.Ogmacam_put_Hue(self.__h, ctypes.c_int(Hue))

    def get_Hue(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Hue(self.__h, ctypes.byref(x))
        return x.value

    def put_Saturation(self, Saturation):
        self.__lib.Ogmacam_put_Saturation(self.__h, ctypes.c_int(Saturation))

    def get_Saturation(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Saturation(self.__h, ctypes.byref(x))
        return x.value

    def put_Brightness(self, Brightness):
        self.__lib.Ogmacam_put_Brightness(self.__h, ctypes.c_int(Brightness))

    def get_Brightness(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Brightness(self.__h, ctypes.byref(x))
        return x.value

    def get_Contrast(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Contrast(self.__h, ctypes.byref(x))
        return x.value

    def put_Contrast(self, Contrast):
        self.__lib.Ogmacam_put_Contrast(self.__h, ctypes.c_int(Contrast))

    def get_Gamma(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Gamma(self.__h, ctypes.byref(x))
        return x.value

    def put_Gamma(self, Gamma):
        self.__lib.Ogmacam_put_Gamma(self.__h, ctypes.c_int(Gamma))

    def get_Chrome(self):
        """monochromatic mode"""
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Chrome(self.__h, ctypes.byref(b)) < 0
        return (b.value != 0)

    def put_Chrome(self, bChrome):
        self.__lib.Ogmacam_put_Chrome(self.__h, ctypes.c_int(1 if bChrome else 0))

    def get_VFlip(self):
        """vertical flip"""
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_VFlip(self.__h, ctypes.byref(b))
        return (b.value != 0)

    def put_VFlip(self, bVFlip):
        """vertical flip"""
        self.__lib.Ogmacam_put_VFlip(self.__h, ctypes.c_int(1 if bVFlip else 0))

    def get_HFlip(self):
        """horizontal flip"""
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_HFlip(self.__h, ctypes.byref(b))
        return (b.value != 0)

    def put_HFlip(self, bHFlip):
        """horizontal flip"""
        self.__lib.Ogmacam_put_HFlip(self.__h, ctypes.c_int(1 if bHFlip else 0))

    def get_Negative(self):
        """negative film"""
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Negative(self.__h, ctypes.byref(b))
        return (b.value != 0)

    def put_Negative(self, bNegative):
        """negative film"""
        self.__lib.Ogmacam_put_Negative(self.__h, ctypes.c_int(1 if bNegative else 0))

    def put_Speed(self, nSpeed):
        self.__lib.Ogmacam_put_Speed(self.__h, ctypes.c_ushort(nSpeed))

    def get_Speed(self):
        x = ctypes.c_ushort(0)
        self.__lib.Ogmacam_get_Speed(self.__h, ctypes.byref(x))
        return x.value

    def put_HZ(self, nHZ):
        """
        power supply:
            0 => 60HZ AC
            1 => 50Hz AC
            2 => DC
        """
        self.__lib.Ogmacam_put_HZ(self.__h, ctypes.c_int(nHZ))

    def get_HZ(self):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_HZ(self.__h, ctypes.byref(x))
        return x.value

    def put_Mode(self, bSkip):
        """skip or bin"""
        self.__lib.Ogmacam_put_Mode(self.__h, ctypes.c_int(1 if bSkip else 0))

    def get_Mode(self):
        b = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Mode(self.__h, ctypes.byref(b))
        return (b.value != 0)

    def put_TempTint(self, nTemp, nTint):
        """White Balance, Temp/Tint mode"""
        self.__lib.Ogmacam_put_TempTint(self.__h, ctypes.c_int(nTemp), ctypes.c_int(nTint))

    def get_TempTint(self):
        """White Balance, Temp/Tint mode"""
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_TempTint(self.__h, ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def put_WhiteBalanceGain(self, aGain):
        """White Balance, RGB Gain Mode"""
        if len(aGain) == 3:
            x = (ctypes.c_int * 3)(aGain[0], aGain[1], aGain[2])
            self.__lib.Ogmacam_put_WhiteBalanceGain(self.__h, x)
        else:
            raise HRESULTException(0x80070057)

    def get_WhiteBalanceGain(self):
        """White Balance, RGB Gain Mode"""
        x = (ctypes.c_int * 3)()
        self.__lib.Ogmacam_get_WhiteBalanceGain(self.__h, x)
        return (x[0], x[1], x[2])

    def put_AWBAuxRect(self, X, Y, Width, Height):
        rc = self.__RECT()
        rc.left = X
        rc.right = X + Width
        rc.top = Y
        rc.bottom = Y + Height
        self.__lib.Ogmacam_put_AWBAuxRect(self.__h, ctypes.byref(rc))

    def get_AWBAuxRect(self):
        """return (left, top, width, height)"""
        rc = self.__RECT()
        self.__lib.Ogmacam_get_AWBAuxRect(self.__h, ctypes.byref(rc))
        return (rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top)

    def put_AEAuxRect(self, X, Y, Width, Height):
        rc = self.__RECT()
        rc.left = X
        rc.right = X + Width
        rc.top = Y
        rc.bottom = Y + Height
        self.__lib.Ogmacam_put_AEAuxRect(self.__h, ctypes.byref(rc))

    def get_AEAuxRect(self):
        """return (left, top, width, height)"""
        rc = self.__RECT()
        self.__lib.Ogmacam_get_AEAuxRect(self.__h, ctypes.byref(rc))
        return (rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top)

    def put_BlackBalance(self, aSub):
        if len(aSub) == 3:
            x = (ctypes.c_int * 3)(aSub[0], aSub[1], aSub[2])
            self.__lib.Ogmacam_put_BlackBalance(self.__h, x)
        else:
            raise HRESULTException(0x80070057)

    def get_BlackBalance(self):
        x = (ctypes.c_int * 3)()
        self.__lib.Ogmacam_get_BlackBalance(self.__h, x)
        return (x[0], x[1], x[2])

    def put_ABBAuxRect(self, X, Y, Width, Height):
        rc = self.__RECT()
        rc.left = X
        rc.right = X + Width
        rc.top = Y
        rc.bottom = Y + Height
        self.__lib.Ogmacam_put_ABBAuxRect(self.__h, ctypes.byref(rc))

    def get_ABBAuxRect(self):
        """return (left, top, width, height)"""
        rc = self.__RECT()
        self.__lib.Ogmacam_get_ABBAuxRect(self.__h, ctypes.byref(rc))
        return (rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top)

    def get_StillResolution(self, nResolutionIndex):
        x = ctypes.c_int(0)
        y = ctypes.c_int(0)
        self.__lib.Ogmacam_get_StillResolution(self.__h, ctypes.c_uint(nResolutionIndex), ctypes.byref(x), ctypes.byref(y))
        return (x.value, y.value)

    def put_LEDState(self, iLed, iState, iPeriod):
        """
        led state:
            iLed: Led index, (0, 1, 2, ...)
            iState: 1 => Ever bright; 2 => Flashing; other => Off
            iPeriod: Flashing Period (>= 500ms)
        """
        self.__lib.Ogmacam_put_LEDState(self.__h, ctypes.c_ushort(iLed), ctypes.c_ushort(iState), ctypes.c_ushort(iPeriod))

    def write_EEPROM(self, addr, pBuffer):
        self.__lib.Ogmacam_write_EEPROM(self.__h, addr, pBuffer, ctypes.c_uint(len(pBuffer)))

    def read_EEPROM(self, addr, pBuffer):
        self.__lib.Ogmacam_read_EEPROM(self.__h, addr, pBuffer, ctypes.c_uint(len(pBuffer)))

    def rwc_Flash(self, action, addr, pData):
        self.__lib.Ogmacam_rwc_Flash(self.__h, action, addr, ctypes.c_uint(len(pData)), pData)

    def write_Pipe(self, pipeId, pBuffer):
        self.__lib.Ogmacam_write_Pipe(self.__h, pipeId, pBuffer, ctypes.c_uint(len(pBuffer)))

    def read_Pipe(self, pipeId, pBuffer):
        self.__lib.Ogmacam_read_Pipe(self.__h, pipeId, pBuffer, ctypes.c_uint(len(pBuffer)))

    def feed_Pipe(self, pipeId):
        self.__lib.Ogmacam_feed_Pipe(self.__h, ctypes.c_uint(pipeId))

    def write_UART(self, pBuffer):
        self.__lib.Ogmacam_write_UART(self.__h, pBuffer, ctypes.c_uint(len(pBuffer)))

    def read_UART(self, pBuffer):
        self.__lib.Ogmacam_read_UART(self.__h, pBuffer, ctypes.c_uint(len(pBuffer)))

    def put_Option(self, iOption, iValue):
        self.__lib.Ogmacam_put_Option(self.__h, ctypes.c_uint(iOption), ctypes.c_int(iValue))

    def get_Option(self, iOption):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_get_Option(self.__h, ctypes.c_uint(iOption), ctypes.byref(x))
        return x.value

    def put_Linear(self, v8, v16):
        self.__lib.Ogmacam_put_Linear(self.__h, v8, v16)

    def put_Curve(self, v8, v16):
        self.__lib.Ogmacam_put_Curve(self.__h, v8, v16)

    def put_ColorMatrix(self, v):
        if len(v) == 9:
            a = (ctypes.c_double * 9)(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8])
            return self.__lib.Ogmacam_put_ColorMatrix(self.__h, v)
        else:
            raise HRESULTException(0x80070057)

    def put_InitWBGain(self, v):
        if len(v) == 3:
            a = (ctypes.c_short * 3)(v[0], v[1], v[2])
            self.__lib.Ogmacam_put_InitWBGain(self.__h, a)
        else:
            raise HRESULTException(0x80070057)

    def get_Temperature(self, nTemperature):
        """get the temperature of the sensor, in 0.1 degrees Celsius (32 means 3.2 degrees Celsius, -35 means -3.5 degree Celsius)"""
        x = ctypes.c_short(0)
        self.__lib.Ogmacam_get_Temperature(self.__h, ctypes.byref(x))
        return x.value

    def put_Temperature(self, nTemperature):
        """set the target temperature of the sensor or TEC, in 0.1 degrees Celsius (32 means 3.2 degrees Celsius, -35 means -3.5 degree Celsius)"""
        self.__lib.Ogmacam_put_Temperature(self.__h, ctypes.c_short(nTemperature))

    def put_Roi(self, xOffset, yOffset, xWidth, yHeight):
        """xOffset, yOffset, xWidth, yHeight: must be even numbers"""
        self.__lib.Ogmacam_put_Roi(self.__h, ctypes.c_uint(xOffset), ctypes.c_uint(yOffset), ctypes.c_uint(xWidth), ctypes.c_uint(yHeight))

    def get_Roi(self):
        """return (xOffset, yOffset, xWidth, yHeight)"""
        x = ctypes.c_uint(0)
        y = ctypes.c_uint(0)
        w = ctypes.c_uint(0)
        h = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_Roi(self.__h, ctypes.byref(x), ctypes.byref(y), ctypes.byref(w), ctypes.byref(h))
        return (x.value, y.value, w.value, h.value)

    def get_FrameRate(self):
        """
        get the frame rate: framerate (fps) = Frame * 1000.0 / nTime
        return (Frame, Time, TotalFrame)
        """
        x = ctypes.c_uint(0)
        y = ctypes.c_uint(0)
        z = ctypes.c_uint(0)
        self.__lib.Ogmacam_get_FrameRate(self.__h, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)

    def LevelRangeAuto(self):
        self.__lib.Ogmacam_LevelRangeAuto(self.__h)

    def AwbOnce(self):
        """Auto White Balance "Once", Temp/Tint Mode"""
        self.__lib.Ogmacam_AwbOnce(self.__h, None, None)

    def AwbOnePush(self):
        AwbOnce(self)

    def AwbInit(self):
        """Auto White Balance "Once", Temp/Tint Mode"""
        self.__lib.Ogmacam_AwbInit(self.__h, None, None)

    def AbbOnce(self):
        self.__lib.Ogmacam_AbbOnce(self.__h, None, None)

    def AbbOnePush(self):
        AbbOnce(self)

    def FfcOnce(self):
        self.__lib.Ogmacam_FfcOnce(self.__h)

    def FfcOnePush(self):
        FfcOnce(self)

    def DfcOnce(self):
        self.__lib.Ogmacam_DfcOnce(self.__h)

    def DfcOnePush(self):
        DfcOnce(self)

    def DfcExport(filepath):
        if sys.platform == 'win32':
            self.__lib.Ogmacam_DfcExport(self.__h, filepath)
        else:
            self.__lib.Ogmacam_DfcExport(self.__h, filepath.encode())

    def FfcExport(filepath):
        if sys.platform == 'win32':
            self.__lib.Ogmacam_FfcExport(self.__h, filepath)
        else:
            self.__lib.Ogmacam_FfcExport(self.__h, filepath.encode())

    def DfcImport(filepath):
        if sys.platform == 'win32':
            self.__lib.Ogmacam_DfcImport(self.__h, filepath)
        else:
            self.__lib.Ogmacam_DfcImport(self.__h, filepath.encode())

    def FfcImport(filepath):
        if sys.platform == 'win32':
            self.__lib.Ogmacam_FfcImport(self.__h, filepath)
        else:
            self.__lib.Ogmacam_FfcImport(self.__h, filepath.encode())

    def IoControl(self, ioLineNumber, eType, outVal):
        x = ctypes.c_int(0)
        self.__lib.Ogmacam_IoControl(self.__h, ctypes.c_uint(ioLineNumber), ctypes.c_uint(eType), ctypes.c_int(outVal), ctypes.byref(x))
        return x.value

    def get_AfParam(self):
        x = self.__AfParam()
        self.__lib.Ogmacam_get_AfParam(self.__h, ctypes.byref(x))
        return OgmacamAfParam(x.imax.value, x.imin.value, x.idef.value, x.imaxabs.value, x.iminabs.value, x.zoneh.value, x.zonev.value)

    @classmethod
    def Replug(cls, id):
        """
        simulate replug:
        return > 0, the number of device has been replug
        return = 0, no device found
        return E_ACCESSDENIED if without UAC Administrator privileges
        for each device found, it will take about 3 seconds
        """
        if sys.platform == 'win32':
            return cls.__lib.Ogmacam_Replug(id)
        else:
            return cls.__lib.Ogmacam_Replug(id.encode('ascii'))

    @staticmethod
    def __progressCallbackFun(percent, ctx):
        if __class__.__progress:
            __class__.__progress(percent)

    @classmethod
    def Update(cls, camId, filePath, pFun):
        """
        firmware update:
           camId: camera ID
           filePath: ufw file full path
           pFun: progress percent callback
        Please do not unplug the camera or lost power during the upgrade process, this is very very important.
        Once an unplugging or power outage occurs during the upgrade process, the camera will no longer be available and can only be returned to the factory for repair.
        """
        cls.__progress = pFun
        if sys.platform == 'win32':
            return cls.__lib.Ogmacam_Update(camId, filePath, cls.__PROGRESS_CALLBACK(cls.__progressCallbackFun), None)
        else:
            return cls.__lib.Ogmacam_Update(camId.encode('ascii'), filePath.encode('ascii'), cls.__PROGRESS_CALLBACK(cls.__progressCallbackFun), None)

    @classmethod
    def __initlib(cls):
        if cls.__lib is None:
            try: # Firstly try to load the library in the directory where this file is located
                dir = os.path.dirname(os.path.realpath(__file__))
                if sys.platform == 'win32':
                    cls.__lib = ctypes.windll.LoadLibrary(os.path.join(dir, 'ogmacam.dll'))
                elif sys.platform.startswith('linux'):
                    cls.__lib = ctypes.cdll.LoadLibrary(os.path.join(dir, 'libogmacam.so'))
                else:
                    cls.__lib = ctypes.cdll.LoadLibrary(os.path.join(dir, 'libogmacam.dylib'))
            except OSError:
                pass

            if cls.__lib is None:
                if sys.platform == 'win32':
                    cls.__lib = ctypes.windll.LoadLibrary('ogmacam.dll')
                elif sys.platform.startswith('linux'):
                    cls.__lib = ctypes.cdll.LoadLibrary('libogmacam.so')
                else:
                    cls.__lib = ctypes.cdll.LoadLibrary('libogmacam.dylib')

            cls.__lib.Ogmacam_Version.argtypes = None
            cls.__lib.Ogmacam_EnumV2.restype = ctypes.c_uint
            cls.__lib.Ogmacam_EnumV2.argtypes = [_DeviceV2 * OGMACAM_MAX]
            cls.__lib.Ogmacam_Open.restype = ctypes.c_void_p
            cls.__lib.Ogmacam_Replug.restype = ctypes.c_int
            cls.__lib.Ogmacam_Update.restype = ctypes.c_int
            if sys.platform == 'win32':
                cls.__lib.Ogmacam_Version.restype = ctypes.c_wchar_p
                cls.__lib.Ogmacam_Open.argtypes = [ctypes.c_wchar_p]
                cls.__lib.Ogmacam_Replug.argtypes = [ctypes.c_wchar_p]
                cls.__lib.Ogmacam_Update.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p, cls.__PROGRESS_CALLBACK, ctypes.py_object]
            else:
                cls.__lib.Ogmacam_Version.restype = ctypes.c_char_p
                cls.__lib.Ogmacam_Open.argtypes = [ctypes.c_char_p]
                cls.__lib.Ogmacam_Replug.argtypes = [ctypes.c_char_p]
                cls.__lib.Ogmacam_Update.argtypes = [ctypes.c_char_p, ctypes.c_char_p, cls.__PROGRESS_CALLBACK, ctypes.py_object]
            cls.__lib.Ogmacam_Replug.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Update.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_OpenByIndex.restype = ctypes.c_void_p
            cls.__lib.Ogmacam_OpenByIndex.argtypes = [ctypes.c_uint]
            cls.__lib.Ogmacam_Close.restype = None
            cls.__lib.Ogmacam_Close.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_StartPullModeWithCallback.restype = ctypes.c_int
            cls.__lib.Ogmacam_StartPullModeWithCallback.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_StartPullModeWithCallback.argtypes = [ctypes.c_void_p, cls.__EVENT_CALLBACK, ctypes.py_object]
            cls.__lib.Ogmacam_PullImageV3.restype = ctypes.c_int
            cls.__lib.Ogmacam_PullImageV3.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_PullImageV3.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(cls.__FrameInfoV3)]
            cls.__lib.Ogmacam_PullImageV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_PullImageV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_PullImageV2.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.POINTER(cls.__FrameInfoV2)]
            cls.__lib.Ogmacam_PullStillImageV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_PullStillImageV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_PullStillImageV2.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.POINTER(cls.__FrameInfoV2)]
            cls.__lib.Ogmacam_PullImageWithRowPitchV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_PullImageWithRowPitchV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_PullImageWithRowPitchV2.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.POINTER(cls.__FrameInfoV2)]
            cls.__lib.Ogmacam_PullStillImageWithRowPitchV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_PullStillImageWithRowPitchV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_PullStillImageWithRowPitchV2.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.POINTER(cls.__FrameInfoV2)]
            cls.__lib.Ogmacam_Stop.restype = ctypes.c_int
            cls.__lib.Ogmacam_Stop.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Stop.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_Pause.restype = ctypes.c_int
            cls.__lib.Ogmacam_Pause.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Pause.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_Snap.restype = ctypes.c_int
            cls.__lib.Ogmacam_Snap.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Snap.argtypes = [ctypes.c_void_p, ctypes.c_uint]
            cls.__lib.Ogmacam_SnapN.restype = ctypes.c_int
            cls.__lib.Ogmacam_SnapN.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_SnapN.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint]
            cls.__lib.Ogmacam_SnapR.restype = ctypes.c_int
            cls.__lib.Ogmacam_SnapR.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_SnapR.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint]
            cls.__lib.Ogmacam_Trigger.restype = ctypes.c_int
            cls.__lib.Ogmacam_Trigger.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Trigger.argtypes = [ctypes.c_void_p, ctypes.c_ushort]
            cls.__lib.Ogmacam_put_Size.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Size.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Size.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
            cls.__lib.Ogmacam_get_Size.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Size.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Size.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_eSize.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_eSize.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_eSize.argtypes = [ctypes.c_void_p, ctypes.c_uint]
            cls.__lib.Ogmacam_get_eSize.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_eSize.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_eSize.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_get_FinalSize.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_FinalSize.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_FinalSize.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_get_ResolutionNumber.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ResolutionNumber.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ResolutionNumber.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_Resolution.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Resolution.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Resolution.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)];
            cls.__lib.Ogmacam_get_ResolutionRatio.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ResolutionRatio.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ResolutionRatio.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)];
            cls.__lib.Ogmacam_get_Field.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Field.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Field.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_RawFormat.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_RawFormat.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_RawFormat.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_get_AutoExpoEnable.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_AutoExpoEnable.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_AutoExpoEnable.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_AutoExpoEnable.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_AutoExpoEnable.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_AutoExpoEnable.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_AutoExpoTarget.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_AutoExpoTarget.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_AutoExpoTarget.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_put_AutoExpoTarget.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_AutoExpoTarget.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_AutoExpoTarget.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_put_MaxAutoExpoTimeAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_MaxAutoExpoTimeAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_MaxAutoExpoTimeAGain.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_ushort]
            cls.__lib.Ogmacam_get_MaxAutoExpoTimeAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_MaxAutoExpoTimeAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_MaxAutoExpoTimeAGain.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_put_MinAutoExpoTimeAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_MinAutoExpoTimeAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_MinAutoExpoTimeAGain.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_ushort]
            cls.__lib.Ogmacam_get_MinAutoExpoTimeAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_MinAutoExpoTimeAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_MinAutoExpoTimeAGain.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_put_ExpoTime.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_ExpoTime.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_ExpoTime.argtypes = [ctypes.c_void_p, ctypes.c_uint]
            cls.__lib.Ogmacam_get_ExpoTime.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ExpoTime.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ExpoTime.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_get_RealExpoTime.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_RealExpoTime.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_RealExpoTime.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_get_ExpTimeRange.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ExpTimeRange.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ExpTimeRange.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_put_ExpoAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_ExpoAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_ExpoAGain.argtypes = [ctypes.c_void_p, ctypes.c_ushort]
            cls.__lib.Ogmacam_get_ExpoAGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ExpoAGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ExpoAGain.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_get_ExpoAGainRange.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ExpoAGainRange.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ExpoAGainRange.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_AwbOnce.restype = ctypes.c_int
            cls.__lib.Ogmacam_AwbOnce.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_AwbOnce.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
            cls.__lib.Ogmacam_AwbInit.restype = ctypes.c_int
            cls.__lib.Ogmacam_AwbInit.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_AwbInit.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
            cls.__lib.Ogmacam_put_TempTint.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_TempTint.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_TempTint.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
            cls.__lib.Ogmacam_get_TempTint.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_TempTint.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_TempTint.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_WhiteBalanceGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_WhiteBalanceGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_WhiteBalanceGain.argtypes = [ctypes.c_void_p, (ctypes.c_int * 3)]
            cls.__lib.Ogmacam_get_WhiteBalanceGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_WhiteBalanceGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_WhiteBalanceGain.argtypes = [ctypes.c_void_p, (ctypes.c_int * 3)]
            cls.__lib.Ogmacam_put_BlackBalance.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_BlackBalance.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_BlackBalance.argtypes = [ctypes.c_void_p, (ctypes.c_int * 3)]
            cls.__lib.Ogmacam_get_BlackBalance.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_BlackBalance.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_BlackBalance.argtypes = [ctypes.c_void_p, (ctypes.c_int * 3)]
            cls.__lib.Ogmacam_AbbOnce.restype = ctypes.c_int
            cls.__lib.Ogmacam_AbbOnce.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_AbbOnce.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
            cls.__lib.Ogmacam_FfcOnce.restype = ctypes.c_int
            cls.__lib.Ogmacam_FfcOnce.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_FfcOnce.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_DfcOnce.restype = ctypes.c_int
            cls.__lib.Ogmacam_DfcOnce.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_DfcOnce.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_FfcExport.restype = ctypes.c_int
            cls.__lib.Ogmacam_FfcExport.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_FfcImport.restype = ctypes.c_int
            cls.__lib.Ogmacam_FfcImport.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_DfcExport.restype = ctypes.c_int
            cls.__lib.Ogmacam_DfcExport.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_DfcImport.restype = ctypes.c_int
            cls.__lib.Ogmacam_DfcImport.errcheck = cls.__errcheck
            if sys.platform == 'win32':
                cls.__lib.Ogmacam_FfcExport.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
                cls.__lib.Ogmacam_FfcImport.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
                cls.__lib.Ogmacam_DfcExport.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
                cls.__lib.Ogmacam_DfcImport.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
            else:
                cls.__lib.Ogmacam_FfcExport.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
                cls.__lib.Ogmacam_FfcImport.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
                cls.__lib.Ogmacam_DfcExport.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
                cls.__lib.Ogmacam_DfcImport.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            cls.__lib.Ogmacam_put_Hue.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Hue.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Hue.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Hue.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Hue.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Hue.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Saturation.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Saturation.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Saturation.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Saturation.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Saturation.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Saturation.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Brightness.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Brightness.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Brightness.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Brightness.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Brightness.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Brightness.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Contrast.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Contrast.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Contrast.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Contrast.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Contrast.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Contrast.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Gamma.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Gamma.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Gamma.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Gamma.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Gamma.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Gamma.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Chrome.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Chrome.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Chrome.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Chrome.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Chrome.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Chrome.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_VFlip.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_VFlip.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_VFlip.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_VFlip.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_VFlip.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_VFlip.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_HFlip.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_HFlip.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_HFlip.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_HFlip.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_HFlip.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_HFlip.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Negative.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Negative.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Negative.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Negative.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Negative.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Negative.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Speed.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Speed.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Speed.argtypes = [ctypes.c_void_p, ctypes.c_ushort]
            cls.__lib.Ogmacam_get_Speed.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Speed.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Speed.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_get_MaxSpeed.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_MaxSpeed.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_MaxSpeed.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_FanMaxSpeed.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_FanMaxSpeed.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_FanMaxSpeed.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_MaxBitDepth.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_MaxBitDepth.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_MaxBitDepth.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_put_HZ.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_HZ.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_HZ.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_HZ.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_HZ.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_HZ.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Mode.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Mode.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Mode.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_Mode.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Mode.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Mode.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_AWBAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_AWBAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_AWBAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_get_AWBAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_AWBAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_AWBAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_put_AEAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_AEAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_AEAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_get_AEAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_AEAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_AEAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_put_ABBAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_ABBAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_ABBAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_get_ABBAuxRect.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ABBAuxRect.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ABBAuxRect.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__RECT)]
            cls.__lib.Ogmacam_get_MonoMode.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_MonoMode.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_MonoMode.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_StillResolutionNumber.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_StillResolutionNumber.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_StillResolutionNumber.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_get_StillResolution.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_StillResolution.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_StillResolution.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_RealTime.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_RealTime.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_RealTime.argtypes = [ctypes.c_void_p, ctypes.c_int]
            cls.__lib.Ogmacam_get_RealTime.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_RealTime.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_RealTime.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_Flush.restype = ctypes.c_int
            cls.__lib.Ogmacam_Flush.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_Flush.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_put_Temperature.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Temperature.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Temperature.argtypes = [ctypes.c_void_p, ctypes.c_ushort]
            cls.__lib.Ogmacam_get_Temperature.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Temperature.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Temperature.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_get_Revision.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Revision.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Revision.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_get_SerialNumber.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_SerialNumber.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_SerialNumber.argtypes = [ctypes.c_void_p, ctypes.c_char * 32]
            cls.__lib.Ogmacam_get_FwVersion.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_FwVersion.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_FwVersion.argtypes = [ctypes.c_void_p, ctypes.c_char * 16]
            cls.__lib.Ogmacam_get_HwVersion.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_HwVersion.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_HwVersion.argtypes = [ctypes.c_void_p, ctypes.c_char * 16]
            cls.__lib.Ogmacam_get_ProductionDate.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_ProductionDate.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_ProductionDate.argtypes = [ctypes.c_void_p, ctypes.c_char * 16]
            cls.__lib.Ogmacam_get_FpgaVersion.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_FpgaVersion.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_FpgaVersion.argtypes = [ctypes.c_void_p, ctypes.c_char * 16]
            cls.__lib.Ogmacam_get_PixelSize.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_PixelSize.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_PixelSize.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
            cls.__lib.Ogmacam_put_LevelRange.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_LevelRange.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_LevelRange.argtypes = [ctypes.c_void_p, (ctypes.c_ushort * 4), (ctypes.c_ushort * 4)]
            cls.__lib.Ogmacam_get_LevelRange.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_LevelRange.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_LevelRange.argtypes = [ctypes.c_void_p, (ctypes.c_ushort * 4), (ctypes.c_ushort * 4)]
            cls.__lib.Ogmacam_put_LevelRangeV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_LevelRangeV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_LevelRangeV2.argtypes = [ctypes.c_void_p, ctypes.c_ushort, ctypes.POINTER(cls.__RECT), (ctypes.c_ushort * 4), (ctypes.c_ushort * 4)]
            cls.__lib.Ogmacam_get_LevelRangeV2.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_LevelRangeV2.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_LevelRangeV2.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(cls.__RECT), (ctypes.c_ushort * 4), (ctypes.c_ushort * 4)]
            cls.__lib.Ogmacam_LevelRangeAuto.restype = ctypes.c_int
            cls.__lib.Ogmacam_LevelRangeAuto.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_LevelRangeAuto.argtypes = [ctypes.c_void_p]
            cls.__lib.Ogmacam_put_LEDState.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_LEDState.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_LEDState.argtypes = [ctypes.c_void_p, ctypes.c_ushort, ctypes.c_ushort, ctypes.c_ushort, ctypes.c_ushort]
            cls.__lib.Ogmacam_write_EEPROM.restype = ctypes.c_int
            cls.__lib.Ogmacam_write_EEPROM.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_write_EEPROM.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_read_EEPROM.restype = ctypes.c_int
            cls.__lib.Ogmacam_read_EEPROM.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_read_EEPROM.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_rwc_Flash.restype = ctypes.c_int
            cls.__lib.Ogmacam_rwc_Flash.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_rwc_Flash.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_char_p]
            cls.__lib.Ogmacam_read_Pipe.restype = ctypes.c_int
            cls.__lib.Ogmacam_read_Pipe.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_read_Pipe.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_write_Pipe.restype = ctypes.c_int
            cls.__lib.Ogmacam_write_Pipe.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_write_Pipe.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_feed_Pipe.restype = ctypes.c_int
            cls.__lib.Ogmacam_feed_Pipe.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_feed_Pipe.argtypes = [ctypes.c_void_p, ctypes.c_uint]
            cls.__lib.Ogmacam_put_Option.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Option.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Option.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int]
            cls.__lib.Ogmacam_get_Option.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Option.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Option.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_put_Roi.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Roi.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Roi.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint, ctypes.c_uint]
            cls.__lib.Ogmacam_get_Roi.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_Roi.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_Roi.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint)]
            cls.__lib.Ogmacam_get_AfParam.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_AfParam.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_AfParam.argtypes = [ctypes.c_void_p, ctypes.POINTER(cls.__AfParam)]
            cls.__lib.Ogmacam_IoControl.restype = ctypes.c_int
            cls.__lib.Ogmacam_IoControl.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_IoControl.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]
            cls.__lib.Ogmacam_read_UART.restype = ctypes.c_int
            cls.__lib.Ogmacam_read_UART.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_read_UART.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_write_UART.restype = ctypes.c_int
            cls.__lib.Ogmacam_write_UART.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_write_UART.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint]
            cls.__lib.Ogmacam_put_Linear.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Linear.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Linear.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_put_Curve.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_Curve.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_Curve.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ushort)]
            cls.__lib.Ogmacam_put_ColorMatrix.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_ColorMatrix.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_ColorMatrix.argtypes = [ctypes.c_void_p, ctypes.c_double * 9]
            cls.__lib.Ogmacam_put_InitWBGain.restype = ctypes.c_int
            cls.__lib.Ogmacam_put_InitWBGain.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_put_InitWBGain.argtypes = [ctypes.c_void_p, ctypes.c_ushort * 3]
            cls.__lib.Ogmacam_get_FrameRate.restype = ctypes.c_int
            cls.__lib.Ogmacam_get_FrameRate.errcheck = cls.__errcheck
            cls.__lib.Ogmacam_get_FrameRate.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint)]
            if sys.platform != 'win32' and sys.platform != 'android':
                cls.__lib.Ogmacam_HotPlug.restype = None
                cls.__lib.Ogmacam_HotPlug.argtypes = [cls.__HOTPLUG_CALLBACK, ctypes.c_void_p]