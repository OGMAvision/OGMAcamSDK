#include "stdafx.h"
#include "AutoTest.h"
#include "AutoTestDlg.h"
#include "afxdialogex.h"
#include "global.h"
#include "CSettingPropertySheet.h"
#include "CExposureGainPropertyPage.h"
#include "CWhiteBalancePropertyPage.h"
#include "CTestPropertySheet.h"
#include <Dbt.h>

CAutoTestDlg* g_pMainDlg = nullptr;

static BOOL SaveImageByWIC(const wchar_t* strFilename, const void* pData, const BITMAPINFOHEADER* pHeader)
{
	GUID guidContainerFormat;
	if (PathMatchSpec(strFilename, L"*.bmp"))
		guidContainerFormat = GUID_ContainerFormatBmp;
	else if (PathMatchSpec(strFilename, L"*.jpg"))
		guidContainerFormat = GUID_ContainerFormatJpeg;
	else if (PathMatchSpec(strFilename, L"*.png"))
		guidContainerFormat = GUID_ContainerFormatPng;
	else
		return FALSE;

	CComPtr<IWICImagingFactory> spIWICImagingFactory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)&spIWICImagingFactory);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IWICBitmapEncoder> spIWICBitmapEncoder;
	hr = spIWICImagingFactory->CreateEncoder(guidContainerFormat, nullptr, &spIWICBitmapEncoder);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IWICStream> spIWICStream;
	spIWICImagingFactory->CreateStream(&spIWICStream);
	if (FAILED(hr))
		return FALSE;

	hr = spIWICStream->InitializeFromFilename(strFilename, GENERIC_WRITE);
	if (FAILED(hr))
		return FALSE;

	hr = spIWICBitmapEncoder->Initialize(spIWICStream, WICBitmapEncoderNoCache);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IWICBitmapFrameEncode> spIWICBitmapFrameEncode;
	CComPtr<IPropertyBag2> spIPropertyBag2;
	hr = spIWICBitmapEncoder->CreateNewFrame(&spIWICBitmapFrameEncode, &spIPropertyBag2);
	if (FAILED(hr))
		return FALSE;

	if (GUID_ContainerFormatJpeg == guidContainerFormat)
	{
		PROPBAG2 option = { 0 };
		option.pstrName = L"ImageQuality"; /* jpg quality, you can change this setting */
		CComVariant varValue(0.75f);
		spIPropertyBag2->Write(1, &option, &varValue);
	}
	hr = spIWICBitmapFrameEncode->Initialize(spIPropertyBag2);
	if (FAILED(hr))
		return FALSE;

	hr = spIWICBitmapFrameEncode->SetSize(pHeader->biWidth, pHeader->biHeight);
	if (FAILED(hr))
		return FALSE;

	WICPixelFormatGUID formatGUID = GUID_WICPixelFormat24bppBGR;
	hr = spIWICBitmapFrameEncode->SetPixelFormat(&formatGUID);
	if (FAILED(hr))
		return FALSE;

	LONG nWidthBytes = TDIBWIDTHBYTES(pHeader->biWidth * pHeader->biBitCount);
	for (LONG i = 0; i < pHeader->biHeight; ++i)
	{
		hr = spIWICBitmapFrameEncode->WritePixels(1, nWidthBytes, nWidthBytes, ((BYTE*)pData) + nWidthBytes * (pHeader->biHeight - i - 1));
		if (FAILED(hr))
			return FALSE;
	}

	hr = spIWICBitmapFrameEncode->Commit();
	if (FAILED(hr))
		return FALSE;
	hr = spIWICBitmapEncoder->Commit();
	if (FAILED(hr))
		return FALSE;

	return TRUE;
}

CAutoTestDlg::CAutoTestDlg(CWnd* pParent /*=nullptr*/)
: CDialog(IDD_AUTOTEST_DIALOG, pParent), m_pImageData(nullptr), m_pSettingPropertySheet(nullptr), m_dwHeartbeat(0)
{
	g_pMainDlg = this;

	memset(&m_header, 0, sizeof(m_header));
	m_header.biSize = sizeof(m_header);
	m_header.biPlanes = 1;
	m_header.biBitCount = 24;

	g_NopacketTimeout = theApp.GetProfileInt(_T("Options"), _T("NopacketTimeout"), g_NopacketTimeout);
	g_NoframeTimeout = theApp.GetProfileInt(_T("Options"), _T("NoframeTimeout"), g_NoframeTimeout);
	g_HeartbeatTimeout = theApp.GetProfileInt(_T("Options"), _T("HeartbeatTimeout"), g_HeartbeatTimeout);
	g_bReplug = theApp.GetProfileInt(_T("Options"), _T("Replug"), g_bReplug ? 1 : 0);
	g_bEnableCheckBlack = theApp.GetProfileInt(_T("Options"), _T("CheckBlack"), g_bEnableCheckBlack ? 1 : 0);
}

void CAutoTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERA_LIST, m_camList);
}

BEGIN_MESSAGE_MAP(CAutoTestDlg, CDialog)
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BUTTON_START, &CAutoTestDlg::OnBnClickedButtonStart)
	ON_MESSAGE(MSG_CAMEVENT, &CAutoTestDlg::OnMsgCamevent)
	ON_MESSAGE(WM_USER_PREVIEW_CHANGE, &CAutoTestDlg::OnPreviewResChanged)
	ON_MESSAGE(WM_USER_OPEN_CLOSE, &CAutoTestDlg::OnCloseOpen)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SETTING, &CAutoTestDlg::OnBnClickedButtonSetting)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CAutoTestDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON1, &CAutoTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_OPTIONS, &CAutoTestDlg::OnBnClickedOptions)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CAutoTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	EnumCamera();

	CheckDlgButton(IDC_CHECK2, g_bEnableCheckBlack ? 1 : 0);
	return TRUE;
}

BOOL CAutoTestDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	if (DBT_DEVNODES_CHANGED == nEventType)
	{
		EnumCamera();
		if (g_bReplug && (nullptr == g_hcam) && (g_cameraCnt > 0))
		{
			Sleep(500);
			OnBnClickedButtonStart();
		}
	}

	return FALSE;
}

LRESULT CAutoTestDlg::OnMsgCamevent(WPARAM wp, LPARAM lp)
{
	switch (wp)
	{
	case OGMACAM_EVENT_ERROR:
	case OGMACAM_EVENT_NOFRAMETIMEOUT:
	case OGMACAM_EVENT_NOPACKETTIMEOUT:
	case OGMACAM_EVENT_DISCONNECTED:
		OnEventError();
		break;
	case OGMACAM_EVENT_IMAGE:
		OnEventImage();
		break;
	case OGMACAM_EVENT_EXPOSURE:
		OnEventExpo();
		break;
	case OGMACAM_EVENT_TEMPTINT:
		OnEventTempTint();
		break;
	case OGMACAM_EVENT_STILLIMAGE:
		OnEventStillImage();
		break;
	case OGMACAM_EVENT_HEARTBEAT:
		m_dwHeartbeat = GetTickCount();
		break;
	default:
		break;
	}
	return 0;
}

void CAutoTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (1 == nIDEvent)
	{
		if (g_HeartbeatTimeout && (GetTickCount() - m_dwHeartbeat > g_HeartbeatTimeout))
			OnEventError();
	}
}

LRESULT CAutoTestDlg::OnPreviewResChanged(WPARAM wp, LPARAM lp)
{
	if (nullptr == g_hcam)
		return -1;

	unsigned nSel = wp;
	unsigned nResolutionIndex = 0;
	HRESULT hr = Ogmacam_get_eSize(g_hcam, &nResolutionIndex);
	if (FAILED(hr))
		return -1;

	if (nResolutionIndex != nSel)
	{
		hr = Ogmacam_Stop(g_hcam);
		if (FAILED(hr))
			return -1;
		Ogmacam_put_eSize(g_hcam, nSel);

		StartCamera();
		UpdateInfo();
	}
	return 0;
}

LRESULT CAutoTestDlg::OnCloseOpen(WPARAM wp, LPARAM lp)
{
	OnBnClickedButtonStart();
	return 0;
}

void CAutoTestDlg::EnumCamera()
{
	g_cameraCnt = Ogmacam_EnumV2(g_cam);
	m_camList.ResetContent();
	int index = 0;
	for (int i = 0; i < g_cameraCnt; ++i)
	{
		m_camList.AddString(g_cam[i].displayname);
		if (0 == m_camID.Compare(g_cam[i].id))
			index = i;
	}

	if (g_cameraCnt > 0)
		m_camList.SetCurSel(index);

	UpdateButtonsState();
}

void CAutoTestDlg::UpdateButtonsState()
{
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(g_cameraCnt > 0);
	CString startBtnText;
	GetDlgItemText(IDC_BUTTON_START, startBtnText);
	BOOL bStart = (0 == startBtnText.Compare(_T("Close")));
	GetDlgItem(IDC_BUTTON_SETTING)->EnableWindow(g_cameraCnt > 0 && bStart);
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(g_cameraCnt > 0 && bStart);
}

void CAutoTestDlg::OnBnClickedButtonStart()
{
	CString startBtnText;
	GetDlgItemText(IDC_BUTTON_START, startBtnText);
	if (0 == startBtnText.Compare(_T("Open")))
	{
		if (g_cameraCnt <= 0)
			return;

		m_camID = g_cam[m_camList.GetCurSel()].id;
		m_camName = g_cam[m_camList.GetCurSel()].displayname;
		g_hcam = Ogmacam_Open(m_camID);
		if (nullptr == g_hcam)
			return;

		m_dwHeartbeat = 0;
		if (g_HeartbeatTimeout && (g_cam[m_camList.GetCurSel()].model->flag & OGMACAM_FLAG_EVENT_HARDWARE))
		{
			Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_EVENT_HARDWARE, 1);
			Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_EVENT_HARDWARE | OGMACAM_EVENT_HEARTBEAT, 1);

			SetTimer(1, 1000, nullptr);
		}
		Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_NOPACKET_TIMEOUT, g_NopacketTimeout);
		Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_NOFRAME_TIMEOUT, g_NoframeTimeout);

		StartCamera();

		SetDlgItemText(IDC_BUTTON_START, _T("Close"));
		UpdateInfo();
	}
	else
	{
		CloseCamera();

		SetDlgItemText(IDC_BUTTON_START, _T("Open"));
		SetWindowText(_T(""));
	}
	
	UpdateButtonsState();
}

void CAutoTestDlg::StartCamera()
{
	int width = 0, height = 0;
	Ogmacam_get_Size(g_hcam, &width, &height);
	m_header.biWidth = width;
	m_header.biHeight = height;
	m_header.biSizeImage = TDIBWIDTHBYTES(width * 24) * height;
	if (m_pImageData)
	{
		free(m_pImageData);
		m_pImageData = nullptr;
	}
	m_pImageData = malloc(m_header.biSizeImage);
	Ogmacam_StartPullModeWithWndMsg(g_hcam, m_hWnd, MSG_CAMEVENT);
}

void CAutoTestDlg::CloseCamera()
{
	if (g_hcam)
	{
		Ogmacam_Close(g_hcam);
		g_hcam = nullptr;
	}

	if (m_pImageData)
	{
		free(m_pImageData);
		m_pImageData = nullptr;
	}
}

void CAutoTestDlg::OnEventError()
{
	CloseCamera();
	if (g_bReplug)
		OnBnClickedButton1();

	SetDlgItemText(IDC_BUTTON_START, _T("Open"));
	SetWindowText(_T(""));

	UpdateButtonsState();
}

void CAutoTestDlg::OnEventImage()
{
	OgmacamFrameInfoV3 info = { 0 };
	HRESULT hr = Ogmacam_PullImageV3(g_hcam, m_pImageData, 0, 24, 0, &info);
	if (SUCCEEDED(hr))
	{
		if (g_bROITest && g_bROITest_SnapStart)
		{
			unsigned offsetX = 0, offsetY = 0, width = 0, height = 0;
			Ogmacam_get_Roi(g_hcam, &offsetX, &offsetY, &width, &height);
			if (width == info.width && height == info.height)
			{
				BITMAPINFOHEADER header = { 0 };
				header.biSize = sizeof(header);
				header.biPlanes = 1;
				header.biBitCount = 24;
				header.biWidth = info.width;
				header.biHeight = info.height;
				header.biSizeImage = TDIBWIDTHBYTES(header.biWidth * header.biBitCount) * header.biHeight;
				CString str;
				SYSTEMTIME tm;
				GetLocalTime(&tm);
				str.Format(g_snapDir + _T("\\%d_%dx%d_%04hu%02hu%02hu_%02hu%02hu%02hu_%03hu.jpg"), g_ROITestCount++, info.width, info.height, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
				SaveImageByWIC(str, m_pImageData, &header);
				g_bROITest_SnapStart = false;
				g_bROITest_SnapFinish = true;
			}
		}
		else if (g_bTriggerTest)
		{
			SYSTEMTIME tm;
			GetLocalTime(&tm);
			CString str;
			str.Format(g_snapDir + _T("\\%04hu%02hu%02hu_%02hu%02hu%02hu_%03hu.jpg"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
			SaveImageByWIC(str, m_pImageData, &m_header);
		}
		else if (g_bImageSnap)
		{
			int resWidth = 0, resHeight = 0;
			Ogmacam_get_Size(g_hcam, &resWidth, &resHeight);
			CString str;
			SYSTEMTIME tm;
			GetLocalTime(&tm);
			str.Format(g_snapDir + _T("\\%d_%dx%d_%04hu%02hu%02hu_%02hu%02hu%02hu_%03hu.jpg"), g_snapCount, resWidth, resHeight, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
			SaveImageByWIC(str, m_pImageData, &m_header);
			if (g_bCheckBlack)
			{
				const int pitchWidth = TDIBWIDTHBYTES(m_header.biWidth * 24);
				char* pData = static_cast<char*>(m_pImageData);
				bool bBlack = false;
				int blackCnt = 0;
				for (int i = 0; i < m_header.biHeight; ++i)
				{
					for (int j = 0; j < m_header.biWidth; ++j)
					{
						const int value0 = *(pData + i * pitchWidth + j);
						const int value1 = *(pData + i * pitchWidth + j + 1);
						const int value2 = *(pData + i * pitchWidth + j + 2);
						if (0 == value0 && 0 == value1 && 0 == value2)
						{
							++blackCnt;
							if (blackCnt > m_header.biHeight * m_header.biWidth / 2)
							{
								bBlack = true;
								break;
							}
						}
					}
					if (bBlack)
						break;
				}
				g_bBlack = bBlack;
			}
			g_bImageSnap = false;
		}
		else
		{
			CClientDC dc(this);
			CRect rc;
			GetClientRect(&rc);
			GetDlgItem(IDC_STATIC_IMAGE)->GetWindowRect(&rc);
			ScreenToClient(&rc);

			int m = dc.SetStretchBltMode(COLORONCOLOR);
			StretchDIBits(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0, 0,
				m_header.biWidth, m_header.biHeight, m_pImageData, (BITMAPINFO*)&m_header, DIB_RGB_COLORS, SRCCOPY);
			dc.SetStretchBltMode(m);
		}
	}
}

void CAutoTestDlg::OnEventExpo()
{
	if (m_pSettingPropertySheet)
	{
		CExposureGainPropertyPage* pPage = m_pSettingPropertySheet->GetExposureGainPropertyPage();
		pPage->OnAutoExposure();
	}
}

void CAutoTestDlg::OnEventTempTint()
{
	if (m_pSettingPropertySheet)
	{
		CWhiteBalancePropertyPage* pPage = m_pSettingPropertySheet->GetWhiteBalancePropertyPage();
		pPage->OnWhiteBalance();
	}
}

void CAutoTestDlg::OnEventStillImage()
{
	OgmacamFrameInfoV3 info = { 0 };
	HRESULT hr = Ogmacam_PullImageV3(g_hcam, nullptr, 1, 24, 0, &info);
	if (SUCCEEDED(hr))
	{
		void* pData = malloc(TDIBWIDTHBYTES(info.width * 24) * info.height);
		hr = Ogmacam_PullImageV3(g_hcam, pData, 1, 24, 0, nullptr);
		if (SUCCEEDED(hr))
		{
			BITMAPINFOHEADER header = { 0 };
			header.biSize = sizeof(header);
			header.biPlanes = 1;
			header.biBitCount = 24;
			header.biWidth = info.width;
			header.biHeight = info.height;
			header.biSizeImage = TDIBWIDTHBYTES(header.biWidth * header.biBitCount) * header.biHeight;

			if (g_bSnapTest)
			{
				int resWidth = 0, resHeight = 0;
				Ogmacam_get_Size(g_hcam, &resWidth, &resHeight);
				CString str;
				str.Format(g_snapDir + _T("\\%d_%dx%d_%dx%d.jpg"), g_snapCount, resWidth, resHeight, info.width, info.height);
				SaveImageByWIC(str, pData, &header);
				g_bSnapFinish = true;
			}
			else
			{
				static int index = 0;
				CString str;
				str.Format(_T("autotest_%d.jpg"), ++index);
				SaveImageByWIC(str, pData, &header);
			}
		}
		free(pData);
	}
}

void CAutoTestDlg::UpdateInfo()
{
	if (g_hcam)
	{
		int width = 0, height = 0;
		Ogmacam_get_Size(g_hcam, &width, &height);
		CString str;
		str.Format(_T("autotest: [%d x %d]"), width, height);
		SetWindowText(str);
	}
}

void CAutoTestDlg::OnClose()
{
	CloseCamera();
	CDialog::OnClose();
}

void CAutoTestDlg::OnBnClickedButtonSetting()
{
	CSettingPropertySheet setting(_T("Settings"));
	m_pSettingPropertySheet = &setting;
	setting.DoModal();
	m_pSettingPropertySheet = nullptr;
}

void CAutoTestDlg::OnBnClickedButtonTest()
{
	CTestPropertySheet test(_T("Test"));
	test.DoModal();
}

void CAutoTestDlg::OnBnClickedButton1()
{
	if (g_cameraCnt <= 0)
		return;
	if (g_hcam)
	{
		AfxMessageBox(L"Camera cannot be replug when it is running.", MB_OK | MB_ICONWARNING);
		return;
	}
	Ogmacam_Replug(g_cam[m_camList.GetCurSel()].id);
}

class COptionsDlg : public CDialog
{
public:
	COptionsDlg(CWnd* pParent = nullptr);
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_OPTIONS };
	//}}AFX_DATA
	//{{AFX_VIRTUAL(COptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL
	virtual BOOL OnInitDialog();
	virtual void OnOK();
protected:
	DECLARE_MESSAGE_MAP()
};

COptionsDlg::COptionsDlg(CWnd* pParent)
: CDialog(IDD_OPTIONS, pParent)
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CheckDlgButton(IDC_CHECK1, g_bReplug ? 1 : 0);
	CheckDlgButton(IDC_CHECK2, g_bEnableCheckBlack ? 1 : 0);
	SetDlgItemInt(IDC_EDIT1, g_NopacketTimeout);
	SetDlgItemInt(IDC_EDIT3, g_NoframeTimeout);
	SetDlgItemInt(IDC_EDIT2, g_HeartbeatTimeout);

	return TRUE;
}

void COptionsDlg::OnOK()
{
	g_NopacketTimeout = GetDlgItemInt(IDC_EDIT1);
	if (g_NopacketTimeout && (g_NopacketTimeout < OGMACAM_NOPACKET_TIMEOUT_MIN))
	{
		GotoDlgCtrl(GetDlgItem(IDC_EDIT1));
		AfxMessageBox(_T("Value to small."));
		return;
	}
	g_NoframeTimeout = GetDlgItemInt(IDC_EDIT3);
	if (g_NoframeTimeout && (g_NoframeTimeout < OGMACAM_NOFRAME_TIMEOUT_MIN))
	{
		GotoDlgCtrl(GetDlgItem(IDC_EDIT3));
		AfxMessageBox(_T("Value to small."));
		return;
	}
	g_HeartbeatTimeout = GetDlgItemInt(IDC_EDIT2);

	g_bReplug = IsDlgButtonChecked(IDC_CHECK1) ? true : false;
	g_bEnableCheckBlack = IsDlgButtonChecked(IDC_CHECK2) ? true : false;

	CDialog::OnOK();
}

void CAutoTestDlg::OnBnClickedOptions()
{
	COptionsDlg dlg;
	if (IDOK == dlg.DoModal())
	{
		if (g_hcam)
		{
			Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_NOPACKET_TIMEOUT, g_NopacketTimeout);
			Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_NOFRAME_TIMEOUT, g_NoframeTimeout);
		}
	
		theApp.WriteProfileInt(_T("Options"), _T("NopacketTimeout"), g_NopacketTimeout);
		theApp.WriteProfileInt(_T("Options"), _T("NoframeTimeout"), g_NoframeTimeout);
		theApp.WriteProfileInt(_T("Options"), _T("HeartbeatTimeout"), g_HeartbeatTimeout);
		theApp.WriteProfileInt(_T("Options"), _T("Replug"), g_bReplug ? 1 : 0);
		theApp.WriteProfileInt(_T("Options"), _T("CheckBlack"), g_bEnableCheckBlack ? 1 : 0);
	}
}