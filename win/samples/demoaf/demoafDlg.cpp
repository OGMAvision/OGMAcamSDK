#include "stdafx.h"
#include "demoaf.h"
#include "demoafDlg.h"
#include <InitGuid.h>
#include <wincodec.h>

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
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)&spIWICImagingFactory);
	if (FAILED(hr))
		return FALSE;

	CComPtr<IWICBitmapEncoder> spIWICBitmapEncoder;
	hr = spIWICImagingFactory->CreateEncoder(guidContainerFormat, NULL, &spIWICBitmapEncoder);
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

CdemoafDlg::CdemoafDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CdemoafDlg::IDD, pParent), m_hcam(NULL), m_pImageData(NULL), m_dFV(0), m_dLum(0)
	, m_bLensCal_Update_Done(false), m_bLensStatus_Update_Done(false), m_ucAM_Max_Previous(0)
{
	memset(&m_header, 0, sizeof(m_header));
	m_header.biSize = sizeof(m_header);
	m_header.biPlanes = 1;
	m_header.biBitCount = 24;

	m_rectTracker = new CRectTrackerEx();
	m_rectTracker->m_nStyle = CRectTracker::resizeInside | CRectTracker::dottedLine;

	m_revision = 0;
}

BEGIN_MESSAGE_MAP(CdemoafDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CdemoafDlg::OnBnClickedButton1)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CdemoafDlg::OnCbnSelchangeCombo1)
	ON_MESSAGE(MSG_CAMEVENT, &CdemoafDlg::OnMsgCamevent)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, &CdemoafDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK1, &CdemoafDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON3, &CdemoafDlg::OnBnClickedButton3)
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_EXITSIZEMOVE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_LENSCAL, &CdemoafDlg::OnBnClickedLenscal)
	ON_BN_CLICKED(IDC_FOCUSMOTORUP, &CdemoafDlg::OnBnClickedFocusmotorup)
	ON_BN_CLICKED(IDC_FOCUSMOTORDOWN, &CdemoafDlg::OnBnClickedFocusmotordown)
	ON_CBN_SELCHANGE(IDC_COMBO_F, &CdemoafDlg::OnCbnSelchangeComboF)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, &CdemoafDlg::OnBnClickedRadioManual)
	ON_BN_CLICKED(IDC_STATUS_DEFAULT, &CdemoafDlg::OnBnClickedStatusDefault)
	ON_BN_CLICKED(IDC_RADIO_AUTO, &CdemoafDlg::OnBnClickedRadioAuto)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_ONEPUSH, &CdemoafDlg::OnBnClickedButtonOnepush)
	ON_EN_SETFOCUS(IDC_FOCUSMOTORSTEP, &CdemoafDlg::OnEnSetfocusFocusmotorstep)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_STATUS, &CdemoafDlg::OnBnClickedButtonSaveStatus)
END_MESSAGE_MAP()

BOOL CdemoafDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLIDER_EXP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLIDER2)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLIDER3)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO1)->EnableWindow(FALSE);

	SetFocusFNControll(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(FALSE);
	GetDlgItem(IDC_LENSCAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONEPUSH)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);

	GetDlgItem(IDC_STATUS_DEFAULT)->EnableWindow(FALSE);

	return TRUE;
}

void CdemoafDlg::OnBnClickedButton1()
{
	if (m_hcam)
		return;

	m_hcam = Ogmacam_Open(NULL);
	if (NULL == m_hcam)
	{
		AfxMessageBox(_T("No Device"));
		return;
	}
	Ogmacam_get_Revision(m_hcam, &m_revision);

	CComboBox* pCombox = (CComboBox*)GetDlgItem(IDC_COMBO1);//Resolution combobox
	pCombox->ResetContent();
	const int n = (int)Ogmacam_get_ResolutionNumber(m_hcam);
	if (n > 0)
	{
		TCHAR txt[128];
		int nWidth, nHeight;
		for (int i = 0; i < n; ++i)
		{
			Ogmacam_get_Resolution(m_hcam, i, &nWidth, &nHeight);
			_stprintf(txt, _T("%d * %d"), nWidth, nHeight);
			pCombox->AddString(txt);
		}

		unsigned nCur = 0;
		Ogmacam_get_eSize(m_hcam, &nCur);
		pCombox->SetCurSel(nCur);
	}
	StartAFLensControll();
	StartDevice();
	SetTimer(1, 100, nullptr);
}

void CdemoafDlg::StartAFLensControll()
{
	bool bLoadStatus;
	Ogmacam_read_EEPROM(m_hcam, 0, (unsigned char*)&bLoadStatus, sizeof(bool));
	if (bLoadStatus)
	{
		((CButton*)GetDlgItem(IDC_STATUS_DEFAULT))->SetCheck(TRUE);
		m_bLensStatus_Update_Done = false;
	}
	else
	{
		((CButton*)GetDlgItem(IDC_STATUS_DEFAULT))->SetCheck(FALSE);
		m_bLensStatus_Update_Done = true;
	}
	m_bLensCal_Update_Done = false;
	Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_CALIBRATE);
}

void CdemoafDlg::StartDevice()
{
	int nWidth = 0, nHeight = 0;
	HRESULT hr = Ogmacam_get_Size(m_hcam, &nWidth, &nHeight);
	if (FAILED(hr))
		return;

	m_header.biWidth = nWidth;
	m_header.biHeight = nHeight;
	m_header.biSizeImage = TDIBWIDTHBYTES(nWidth * 24) * nHeight;
	if (m_pImageData)
	{
		free(m_pImageData);
		m_pImageData = NULL;
	}
	m_pImageData = malloc(m_header.biSizeImage);

	Ogmacam_StartPullModeWithWndMsg(m_hcam, m_hWnd, MSG_CAMEVENT);

	GetAEAuxRect();
	const CRect rc = GetDrawRect();
	m_RClimit = SetDisplayLimit(rc);
	m_rectTracker->SetRectLimit(m_RClimit);

	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(FALSE);
	Ogmacam_put_AutoExpoEnable(m_hcam, 0);
	UpdateExpoSlidersEnable();

	OnEventTempTint();
	SetClarityRect();
	SetFrameRateLimit();

	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(TRUE);
	GetDlgItem(IDC_SLIDER2)->EnableWindow(TRUE);
	GetDlgItem(IDC_SLIDER3)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO1)->EnableWindow(TRUE);
}

void CdemoafDlg::OnCbnSelchangeCombo1()
{
	if (NULL == m_hcam)
		return;

	const int nSel = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCurSel();
	if (nSel < 0)
		return;

	unsigned nResolutionIndex = 0;
	HRESULT hr = Ogmacam_get_eSize(m_hcam, &nResolutionIndex);
	if (FAILED(hr))
		return;

	if (nResolutionIndex != nSel)
	{
		Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_NONE);
		hr = Ogmacam_Stop(m_hcam);
		if (FAILED(hr))
			return;

		Ogmacam_put_eSize(m_hcam, nSel);

		StartDevice();
		Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_MANUAL);
	}
}

LRESULT CdemoafDlg::OnMsgCamevent(WPARAM wp, LPARAM /*lp*/)
{
	switch (wp)
	{
	case OGMACAM_EVENT_ERROR:
	case OGMACAM_EVENT_NOFRAMETIMEOUT:
	case OGMACAM_EVENT_NOPACKETTIMEOUT:
		OnEventError();
		break;
	case OGMACAM_EVENT_DISCONNECTED:
		OnEventDisconnected();
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
	default:
		break;
	}
	return 0;
}

void CdemoafDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_rectTracker->SetCursor(this, m_rectTracker->HitTest(point));
	if (m_rectTracker->HitTest(point) < 0)
	{
		CRectTracker tempRectTracker;
		tempRectTracker.TrackRubberBand(this, point);
		tempRectTracker.m_rect.NormalizeRect();
		Invalidate();
	}
	else
	{
		OgmacamAFState afStatus;
		Ogmacam_get_AFState(m_hcam, &afStatus);
		if (afStatus.AF_Mode == OgmacamAFMode_AUTO || afStatus.AF_Mode == OgmacamAFMode_ONCE)
			Ogmacam_put_AFRoi(m_hcam, 0, 0, 0, 0);
		if (m_rectTracker->Track(this, point))
		{
			Invalidate();
			SetAEAuxRect();
			SetClarityRect();
		}
	}
	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CdemoafDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ((pWnd == this) && (m_rectTracker->SetCursor(this, nHitTest)))
		return TRUE;
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CdemoafDlg::OnEventDisconnected()
{
	if (m_hcam)
	{
		Ogmacam_Close(m_hcam);
		m_hcam = NULL;
	}
	AfxMessageBox(_T("Camera disconnect."));
}

void CdemoafDlg::OnEventError()
{
	if (m_hcam)
	{
		Ogmacam_Close(m_hcam);
		m_hcam = NULL;
	}
	AfxMessageBox(_T("Generic error."));
}

void CdemoafDlg::OnEventExpo()
{
	if (GetDlgItem(IDC_SLIDER_EXP))
		UpdateExpoValue();

	if (GetDlgItem(IDC_SLIDER_GAIN))
		UpdateGainValue();
}

void CdemoafDlg::UpdateExpoSlidersEnable()
{
	BOOL bEnableAutoExpo = FALSE;
	Ogmacam_get_AutoExpoEnable(m_hcam, &bEnableAutoExpo);
	GetDlgItem(IDC_SLIDER_TARGET)->EnableWindow(bEnableAutoExpo);
	GetDlgItem(IDC_SLIDER_EXP)->EnableWindow(!bEnableAutoExpo);
	GetDlgItem(IDC_SLIDER_GAIN)->EnableWindow(!bEnableAutoExpo);

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_TARGET))->SetRange(OGMACAM_AETARGET_MIN, OGMACAM_AETARGET_MAX);
	unsigned short target = 0;
	Ogmacam_get_AutoExpoTarget(m_hcam, &target);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_TARGET))->SetPos(target);
	SetDlgItemInt(IDC_STATIC_TARGET, target);

	unsigned nMinExpoTime, nMaxExpoTime, nDefExpoTime;
	Ogmacam_get_ExpTimeRange(m_hcam, &nMinExpoTime, &nMaxExpoTime, &nDefExpoTime);
	if (nMaxExpoTime > MY_EXPOTUER_TIME_MAX)
		nMaxExpoTime = MY_EXPOTUER_TIME_MAX;
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_EXP))->SetRange(nMinExpoTime / 1000, nMaxExpoTime / 1000);
	UpdateExpoValue();

	unsigned short gainMin = 0, gainMax = 0, gainDef = 0, gainVal = 0;
	Ogmacam_get_ExpoAGainRange(m_hcam, &gainMin, &gainMax, &gainDef);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN))->SetRange(gainMin, gainMax);
	UpdateGainValue();

	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetRange(OGMACAM_TEMP_MIN, OGMACAM_TEMP_MAX);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetRange(OGMACAM_TINT_MIN, OGMACAM_TINT_MAX);
}

void CdemoafDlg::UpdateExpoValue()
{
	unsigned m_nTime = 0;
	Ogmacam_get_ExpoTime(m_hcam, &m_nTime);
	if (m_nTime > MY_EXPOTUER_TIME_MAX)
		m_nTime = MY_EXPOTUER_TIME_MAX;
	SetDlgItemInt(IDC_STATIC_EXP, m_nTime / 1000, FALSE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_EXP))->SetPos(m_nTime / 1000);
}

void CdemoafDlg::UpdateGainValue()
{
	USHORT nGain = 0;
	Ogmacam_get_ExpoAGain(m_hcam, &nGain);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN))->SetPos(nGain);
	SetDlgItemInt(IDC_STATIC_GAIN, nGain);
}

void CdemoafDlg::OnEventTempTint()
{
	int nTemp = OGMACAM_TEMP_DEF, nTint = OGMACAM_TINT_DEF;
	Ogmacam_get_TempTint(m_hcam, &nTemp, &nTint);
	SetDlgItemInt(IDC_STATIC2, nTemp, TRUE);
	SetDlgItemInt(IDC_STATIC3, nTint, TRUE);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->SetPos(nTemp);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->SetPos(nTint);
}

CRect CdemoafDlg::SetDisplayLimit(CRect rect)
{
	CRect display_limit;
	int iWidth, iHeight;
	HREFTYPE hr = Ogmacam_get_Size(m_hcam, &iWidth, &iHeight);

	double scale = 1.5;
	display_limit.left = rect.left;
	display_limit.top = rect.top;

	if (iHeight != 0)
		scale = (double)iWidth / (double)iHeight;

	if ((double)rect.Width() / (double)rect.Height() >= scale)
	{
		display_limit.bottom = rect.bottom;
		display_limit.right = rect.left + rect.Height() * scale;
	}
	else
	{
		display_limit.right = rect.right;
		display_limit.bottom = rect.top + rect.Width() / scale;
	}
	return display_limit;
}

void CdemoafDlg::SetFrameRateLimit()
{
	int iWidth, iHeight;
	int nFrameRateLimit = 0;
	HREFTYPE hr = Ogmacam_get_Size(m_hcam, &iWidth, &iHeight);
	if (SUCCEEDED(hr))
	{
		nFrameRateLimit = 20;
		if (iWidth * iHeight > 6000 * 8000)
			nFrameRateLimit = 5;
		else if (iWidth * iHeight > 4000 * 6000)
			nFrameRateLimit = 10;
		else if (iWidth * iHeight > 2000 * 3000)
			nFrameRateLimit = 25;
		else if (iWidth * iHeight > 1000 * 2000)
			nFrameRateLimit = 50;
	}
	Ogmacam_put_Option(m_hcam, OGMACAM_OPTION_FRAMERATE, nFrameRateLimit);
}

CRect CdemoafDlg::GetDrawRect()
{
	CRect rc, rcStartButton;
	GetClientRect(&rc);
	GetDlgItem(IDC_BUTTON1)->GetWindowRect(&rcStartButton);
	ScreenToClient(&rcStartButton);
	rc.left = rcStartButton.right + 4;
	rc.top += 4;
	rc.bottom -= 4;
	rc.right -= 4;
	return rc;
}

void CdemoafDlg::OnEventImage()
{
	OgmacamFrameInfoV3 info = { 0 };
	HRESULT hr = Ogmacam_PullImageV3(m_hcam, m_pImageData, 0, 24, 0, &info);
	Ogmacam_get_FrameRate(m_hcam, &m_nFrame, &m_nTime, &m_nTotalFrame);
	if (SUCCEEDED(hr))
	{
		const CRect rc = GetDrawRect();
		CClientDC dc(this);
		int m = dc.SetStretchBltMode(COLORONCOLOR);

		StretchDIBits(dc, rc.left, rc.top, m_RClimit.Width(), m_RClimit.Height(), 0, 0, m_header.biWidth, m_header.biHeight, m_pImageData, (BITMAPINFO*)&m_header, DIB_RGB_COLORS, SRCCOPY);
		dc.SetStretchBltMode(m);

		CPen Pen(PS_SOLID, 1, RGB(255, 0, 0));
		m_rectTracker->Draw(&dc, &Pen);
	}
}

void CdemoafDlg::OnEventStillImage()
{
	OgmacamFrameInfoV3 info = { 0 };
	HRESULT hr = Ogmacam_PullImageV3(m_hcam, NULL, 1, 24, 0, &info);
	if (SUCCEEDED(hr))
	{
		void* pData = malloc(TDIBWIDTHBYTES(info.width * 24) * info.height);
		hr = Ogmacam_PullImageV3(m_hcam, pData, 1, 24, 0, NULL);
		if (SUCCEEDED(hr))
		{
			BITMAPINFOHEADER header = { 0 };
			header.biSize = sizeof(header);
			header.biPlanes = 1;
			header.biBitCount = 24;
			header.biWidth = info.width;
			header.biHeight = info.height;
			header.biSizeImage = TDIBWIDTHBYTES(header.biWidth * header.biBitCount) * header.biHeight;
			SaveImageByWIC(L"demoaf.jpg", pData, &header);
		}
		free(pData);
	}
}

void CdemoafDlg::GetAEAuxRect()
{
	const CRect rc = GetDrawRect();
	RECT rect;
	Ogmacam_get_AEAuxRect(m_hcam, &rect);
	rect.left = rect.left * rc.Width() / m_header.biWidth + rc.left;
	rect.right = rect.right * rc.Width() / m_header.biWidth + rc.left;
	rect.top = rect.top * rc.Height() / m_header.biHeight + rc.top;
	rect.bottom = rect.bottom * rc.Height() / m_header.biHeight + rc.top;
	m_rectTracker->m_rect.SetRect(CPoint(rect.left, rect.top), CPoint(rect.right, rect.bottom));
	m_rectTracker->m_nStyle |= CRectTracker::solidLine;
}

void CdemoafDlg::SetAEAuxRect()
{
	const CRect rc = GetDrawRect();
	RECT rect;
	rect.left = (m_rectTracker->m_rect.left - rc.left) * m_header.biWidth / rc.Width();
	rect.right = (m_rectTracker->m_rect.right - rc.left) * m_header.biWidth / rc.Width();
	rect.bottom = (m_rectTracker->m_rect.bottom - rc.top) * m_header.biHeight / rc.Height();
	rect.top = (m_rectTracker->m_rect.top - rc.top) * m_header.biHeight / rc.Height();
	Ogmacam_put_AEAuxRect(m_hcam, &rect);
}

void CdemoafDlg::SetClarityRect()
{
	const CRect rc = GetDrawRect();
	RECT rect;
	rect.left = (m_rectTracker->m_rect.left - rc.left) * m_header.biWidth / rc.Width();
	rect.right = (m_rectTracker->m_rect.right - rc.left) * m_header.biWidth / rc.Width();
	rect.bottom = (m_rectTracker->m_rect.bottom - rc.top) * m_header.biHeight / rc.Height();
	rect.top = (m_rectTracker->m_rect.top - rc.top) * m_header.biHeight / rc.Height();

	m_ClarityROI.usSize_X = rect.right - rect.left;
	m_ClarityROI.usSize_Y = rect.bottom - rect.top;
	m_ClarityROI.usOffset_X = rect.left;
	m_ClarityROI.usOffset_Y = m_header.biHeight - rect.bottom;
	Ogmacam_put_AFRoi(m_hcam, m_ClarityROI.usOffset_X, m_ClarityROI.usOffset_Y, m_ClarityROI.usSize_X, m_ClarityROI.usSize_Y);
}

void CdemoafDlg::OnDestroy()
{
	if (m_hcam)
	{
		Ogmacam_Close(m_hcam);
		m_hcam = NULL;
	}
	if (m_pImageData)
	{
		free(m_pImageData);
		m_pImageData = NULL;
	}
	if (m_rectTracker)
	{
		delete m_rectTracker;
		m_rectTracker = NULL;
	}
	CDialog::OnDestroy();
}

void CdemoafDlg::OnBnClickedButton2()
{
	Ogmacam_Snap(m_hcam, 0xffffffff);
}

void CdemoafDlg::OnBnClickedCheck1()//Auto exposure
{
	if (m_hcam)
		Ogmacam_put_AutoExpoEnable(m_hcam, IsDlgButtonChecked(IDC_CHECK1) ? 1 : 0);
	UpdateExpoSlidersEnable();
}

void CdemoafDlg::OnBnClickedButton3() //White Balance
{
	GetAEAuxRect();
	if (m_hcam)
		Ogmacam_AwbOnce(m_hcam, NULL, NULL);
}

void CdemoafDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData(TRUE);
	Ogmacam_get_LensInfo(m_hcam, &m_afLensInfo);
	switch (pScrollBar->GetDlgCtrlID())
	{
	case IDC_SLIDER_TARGET:
	{
		unsigned short curTarget = 0;
		unsigned short target = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_TARGET))->GetPos();
		Ogmacam_get_AutoExpoTarget(m_hcam, &curTarget);
		if (target != curTarget)
		{
			Ogmacam_put_AutoExpoTarget(m_hcam, target);
			SetDlgItemInt(IDC_STATIC_TARGET, target);
		}
	}
	case IDC_SLIDER_EXP://Auto Exposure
	{
		unsigned curTime = 0;
		unsigned time = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_EXP))->GetPos();
		Ogmacam_get_ExpoTime(m_hcam, &curTime);
		if(time != curTime)
		{
			Ogmacam_put_ExpoTime(m_hcam, time * 1000);
			SetDlgItemInt(IDC_STATIC_EXP, time, TRUE);
		}
	}
	case IDC_SLIDER_GAIN://Auto Exposure
	{
		unsigned short curGain = 0;
		Ogmacam_get_ExpoAGain(m_hcam, &curGain);
		unsigned short gain = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN))->GetPos();
		if (gain != curGain)
		{
			Ogmacam_put_ExpoAGain(m_hcam, gain);
			SetDlgItemInt(IDC_STATIC_GAIN, gain);
		}
	}
	case IDC_SLIDER2://Temp
	case IDC_SLIDER3://Tint
	{
		const int nTemp = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER2))->GetPos();
		const int nTint = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER3))->GetPos();
		SetDlgItemInt(IDC_STATIC2, nTemp, TRUE);
		SetDlgItemInt(IDC_STATIC3, nTint, TRUE);
		Ogmacam_put_TempTint(m_hcam, nTemp, nTint);
		break;
	}
	case IDC_SLIDER_AP://aperture
	{
		int SliderApSet = m_slider_ap.GetPos();
		Ogmacam_put_AFAperture(m_hcam, SliderApSet);
		SetDlgItemText(IDC_STATIC_FNUMBER, CA2W(m_afLensInfo.arrayFN[SliderApSet]));
		m_combo_aperture.SetCurSel(SliderApSet);
		break;
	}
	case IDC_SLIDER_FOCUS://Manual slider for autofocus
	{
		int SliderFocusSet = m_slider_foc.GetPos();
		SetDlgItemInt(IDC_STATIC_FOCUSMOTOR, SliderFocusSet);
		Ogmacam_put_AFFMPos(m_hcam, SliderFocusSet);
		break;
	}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CdemoafDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_AP, m_slider_ap);
	DDX_Control(pDX, IDC_SLIDER_FOCUS, m_slider_foc);
	DDX_Control(pDX, IDC_COMBO_F, m_combo_aperture);
}

void CdemoafDlg::OnBnClickedFocusmotorup()
{
	int pos = m_slider_foc.GetPos();
	int step = GetDlgItemInt(IDC_FOCUSMOTORSTEP);

	m_slider_foc.SetPos(pos + step);
	Ogmacam_put_AFFMPos(m_hcam, pos + step);

	SetDlgItemInt(IDC_STATIC_FOCUSMOTOR, pos + step);
}

void CdemoafDlg::OnBnClickedFocusmotordown()
{
	int pos = m_slider_foc.GetPos();
	int step = GetDlgItemInt(IDC_FOCUSMOTORSTEP);
	m_slider_foc.SetPos(pos - step);
	Ogmacam_put_AFFMPos(m_hcam, pos - step);
	SetDlgItemInt(IDC_STATIC_FOCUSMOTOR, pos - step);
}

void CdemoafDlg::AF_FocusDlg_Init()
{
	SetDlgItemInt(IDC_FOCUSMOTORMIN, 0);
	SetDlgItemInt(IDC_FOCUSMOTORMAX, m_afLensInfo.farFM);
	m_slider_foc.SetRange(0, m_afLensInfo.farFM);
	m_slider_foc.SetTicFreq(200);
	m_slider_foc.SetPos(0);
	SetDlgItemInt(IDC_STATIC_FOCUSMOTOR, m_afLensInfo.curFM);
}

void CdemoafDlg::AF_APDlg_Init()
{
	m_slider_ap.SetRange(0, m_afLensInfo.sizeFN - 1);
	m_slider_ap.SetPos(0);

	SetDlgItemText(IDC_FNUMBERMIN, CA2W(m_afLensInfo.arrayFN[0]));
	SetDlgItemText(IDC_FNUMBERMAX, CA2W(m_afLensInfo.arrayFN[m_afLensInfo.sizeFN - 1]));

	m_combo_aperture.ResetContent();
	for (int i = 0; i < m_afLensInfo.sizeFN; i++)
		m_combo_aperture.AddString(CA2W(m_afLensInfo.arrayFN[i]));
	SetDlgItemText(IDC_STATIC_FNUMBER, CA2W(m_afLensInfo.arrayFN[0]));
	m_combo_aperture.SetCurSel(0);
}

void CdemoafDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (1 == nIDEvent)
	{
		TCHAR str_frame[64] = { 0 }, str_fv_lum[128] = { 0 };
		_stprintf(str_frame, _T("%u, fps = %.1f"), m_nTotalFrame, m_nFrame * 1000.0 / m_nTime);
		SetDlgItemText(IDC_FRAMENUM, str_frame);
		_stprintf(str_fv_lum, _T("FV = %.1f, LUM = %.1f"), m_dFV, m_dLum);
		SetDlgItemText(IDC_STATIC_FV_LUM, str_fv_lum);
		UpdateData(TRUE);
		OgmacamAFState afStatus;
		if (SUCCEEDED(Ogmacam_get_AFState(m_hcam, &afStatus)) && SUCCEEDED(Ogmacam_get_LensInfo(m_hcam, &m_afLensInfo)))
		{
			if (afStatus.AF_LensManual_Flag)
			{
				if (!m_bLensCal_Update_Done)//data needs to be updated after calibration is completed
				{
					AF_FocusDlg_Init();
					AF_APDlg_Init();
					GetLensName();
					SetDlgItemInt(IDC_EDIT_LENSID, m_afLensInfo.lensID);
					SetDlgItemInt(IDC_EDIT_LENSFMIN, m_afLensInfo.minFocalLength);
					SetDlgItemInt(IDC_EDIT_LENSFMAX, m_afLensInfo.maxFocalLength);
					SetDlgItemInt(IDC_EDIT_LENSFCUR, m_afLensInfo.curFocalLength);
					SetDlgItemInt(IDC_EDIT_LENSFOCUSMOTOR, m_afLensInfo.curFM);
					SetDlgItemText(IDC_FOCUSMOTORSTEP, L"Step");
					GetDlgItem(IDC_EDIT_LENSMFAF)->SetWindowText((0x80 == m_afLensInfo.statusAfmf) ? L"MF" : L"AF");
					m_ucAM_Max_Previous = m_afLensInfo.maxAM;
					m_bLensCal_Update_Done = true;
				}
				else if (!m_bLensStatus_Update_Done)
				{
					if (((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->GetCheck())
						SetLensStatus();
				}
				if (m_ucAM_Max_Previous != m_afLensInfo.maxAM && afStatus.AF_LensAP_Update_Flag)
				{
					AF_APDlg_Init();
					m_ucAM_Max_Previous = m_afLensInfo.maxAM;
				}
			}
			if (((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->GetCheck())
			{
				SetDlgItemText(IDC_STATIC_FNUMBER, CA2W(m_afLensInfo.arrayFN[m_afLensInfo.posAM]));
			}
			else
			{
				if (afStatus.AF_Mode == OgmacamAFMode_MANUAL)
				{
					((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(TRUE);
					((CButton*)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(FALSE);
					SetFocusFNControll(TRUE);
					GetDlgItem(IDC_LENSCAL)->EnableWindow(TRUE);
					GetDlgItem(IDC_BUTTON_ONEPUSH)->EnableWindow(TRUE);
					GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(TRUE);
					GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(TRUE);
					GetDlgItem(IDC_STATUS_DEFAULT)->EnableWindow(TRUE);
					GetDlgItem(IDC_BUTTON_ONEPUSH)->SetWindowText(L"Once");
				}
				else if (m_bLensCal_Update_Done && m_bLensStatus_Update_Done)
				{
					m_slider_foc.SetPos(m_afLensInfo.curFM);
				}
			}
			SetDlgItemInt(IDC_STATIC_FOCUSMOTOR, m_afLensInfo.curFM);
			SetDlgItemInt(IDC_EDIT_LENSFOCUSMOTOR, m_afLensInfo.curFM);
			SetDlgItemInt(IDC_EDIT_LENSFCUR, m_afLensInfo.curFocalLength);

			GetDlgItem(IDC_EDIT_LENSMFAF)->SetWindowText((0x80 == m_afLensInfo.statusAfmf) ? L"MF" : L"AF");
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CdemoafDlg::OnExitSizeMove()
{
	const CRect rc = GetDrawRect();
	m_RClimit = SetDisplayLimit(rc);
	m_rectTracker->SetRectLimit(m_RClimit);
	if (m_hcam)
	{
		SetAEAuxRect();
		SetClarityRect();
		Invalidate();
	}
	CDialog::OnExitSizeMove();
}

void CdemoafDlg::OnCbnSelchangeComboF()
{
	CString strTmp;
	int cindex = m_combo_aperture.GetCurSel();
	m_combo_aperture.GetLBText(cindex, strTmp);
	Ogmacam_put_AFAperture(m_hcam, cindex);
	m_slider_ap.SetPos(cindex);
	SetDlgItemText(IDC_STATIC_FNUMBER, strTmp);
}

void CdemoafDlg::SetLensStatus()
{
	LensStatusInEEPROM SetStatus;
	Ogmacam_read_EEPROM(m_hcam, 0, (unsigned char*)&SetStatus, sizeof(LensStatusInEEPROM));
	if (SetStatus.usID == m_afLensInfo.lensID)
	{
		m_slider_ap.SetPos(SetStatus.cFNumber_Index);
		m_slider_foc.SetPos(SetStatus.sFM_Cur);
		m_combo_aperture.SetCurSel(SetStatus.cFNumber_Index);
		Ogmacam_put_AFAperture(m_hcam, SetStatus.cFNumber_Index);
		Ogmacam_put_AFFMPos(m_hcam, SetStatus.sFM_Cur);

		if (0 < SetStatus.uiExpTime && SetStatus.uiExpTime < 1000)
		{
			Ogmacam_put_ExpoTime(m_hcam, SetStatus.uiExpTime * 1000);
			((CSliderCtrl*)GetDlgItem(IDC_SLIDER_EXP))->SetPos(SetStatus.uiExpTime);
			SetDlgItemInt(IDC_STATIC_EXP, SetStatus.uiExpTime, TRUE);
		}
		if (0 < SetStatus.usGain && SetStatus.usGain < 1000)
		{
			Ogmacam_put_ExpoAGain(m_hcam, SetStatus.usGain);
			((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN))->SetPos(SetStatus.usGain);
			SetDlgItemInt(IDC_STATIC_GAIN, SetStatus.usGain, TRUE);
		}
	}
	m_bLensStatus_Update_Done = true;
}

void CdemoafDlg::OnBnClickedStatusDefault()
{
	UpdateData(TRUE);
	LensStatusInEEPROM SetStatus;
	Ogmacam_read_EEPROM(m_hcam, 0, ( unsigned char*)&SetStatus, sizeof(LensStatusInEEPROM));
	SetStatus.bSet_Default = ((CButton*)GetDlgItem(IDC_STATUS_DEFAULT))->GetCheck();
	Ogmacam_write_EEPROM(m_hcam, 0, (const unsigned char*)&SetStatus, sizeof(LensStatusInEEPROM));
}

void CdemoafDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (m_hcam)
	{
		if (nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED)
			OnExitSizeMove();
	}
}

void CdemoafDlg::OnBnClickedLenscal()//Manual focus
{
	m_bLensCal_Update_Done = false;
	if (((CButton*)GetDlgItem(IDC_STATUS_DEFAULT))->GetCheck())
		m_bLensStatus_Update_Done = false;
	((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(FALSE);
	((CButton*)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(FALSE);
	SetFocusFNControll(FALSE);
	GetDlgItem(IDC_LENSCAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ONEPUSH)->EnableWindow(FALSE);
	GetDlgItem(IDC_STATUS_DEFAULT)->EnableWindow(FALSE);

	Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_CALIBRATE);
}

void CdemoafDlg::OnBnClickedRadioManual()
{
	Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_MANUAL);
	SetFocusFNControll(TRUE);
	GetDlgItem(IDC_LENSCAL)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ONEPUSH)->EnableWindow(TRUE);
}

void CdemoafDlg::OnBnClickedRadioAuto()
{
	Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_AUTO);
	SetFocusFNControll(FALSE);
	GetDlgItem(IDC_LENSCAL)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ONEPUSH)->EnableWindow(FALSE);
}

void CdemoafDlg::OnBnClickedButtonOnepush()
{
	OgmacamAFState afStatus;
	Ogmacam_get_AFState(m_hcam, &afStatus);
	if (afStatus.AF_Mode == OgmacamAFMode_ONCE)
	{
		Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_MANUAL);
		GetDlgItem(IDC_BUTTON_ONEPUSH)->SetWindowText(L"Once");
	}
	else
	{
		Ogmacam_put_AFMode(m_hcam, OgmacamAFMode_ONCE);
		((CButton*)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(FALSE);
		SetFocusFNControll(FALSE);
		GetDlgItem(IDC_LENSCAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_MANUAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ONEPUSH)->SetWindowText(L"Stop");
	}
}

void CdemoafDlg::SetFocusFNControll(BOOL bControll)
{
	GetDlgItem(IDC_COMBO_F)->EnableWindow(bControll);
	GetDlgItem(IDC_SLIDER_AP)->EnableWindow(bControll);
	GetDlgItem(IDC_SLIDER_FOCUS)->EnableWindow(bControll);
	GetDlgItem(IDC_FOCUSMOTORSTEP)->EnableWindow(bControll);
	GetDlgItem(IDC_FOCUSMOTORUP)->EnableWindow(bControll);
	GetDlgItem(IDC_FOCUSMOTORDOWN)->EnableWindow(bControll);
}

void CdemoafDlg::GetLensName()
{
	char cID_Model[256];
	FILE* FLens_ID_Model = _wfopen(L"Canon_Lens_ID_Model.txt", L"r");
	if (FLens_ID_Model)
	{
		while (fgets(cID_Model, 256, FLens_ID_Model))
		{
			USHORT usLensID;
			char cLensModel[256];
			if (sscanf_s(cID_Model, "%hu = %[^\n]", &usLensID, cLensModel, 256) == 2)
			{
				if (usLensID == m_afLensInfo.lensID)
				{
					cLensModel[255] = '\0';
					SetDlgItemText(IDC_EDIT_LENSNAME, (CString)cLensModel);
					break;
				}
			}
		}
		fclose(FLens_ID_Model);
	}
}

void CdemoafDlg::OnBnClickedButtonSaveStatus()
{
	LensStatusInEEPROM SaveStatus;
	SaveStatus.bSet_Default = ((CButton*)GetDlgItem(IDC_STATUS_DEFAULT))->GetCheck();
	SaveStatus.usID = m_afLensInfo.lensID;
	SaveStatus.sFM_Cur = m_afLensInfo.posFM;
	SaveStatus.cFNumber_Index = m_afLensInfo.posAM;
	SaveStatus.uiExpTime = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_EXP))->GetPos();
	SaveStatus.usGain = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN))->GetPos();
	Ogmacam_write_EEPROM(m_hcam, 0, (const unsigned char*)&SaveStatus, sizeof(LensStatusInEEPROM));
}

void CdemoafDlg::OnEnSetfocusFocusmotorstep()
{
	CString str;
	UpdateData();
	GetDlgItemText(IDC_FOCUSMOTORSTEP, str);
	if (str != L"Step")
		return;
	SetDlgItemText(IDC_FOCUSMOTORSTEP, L"");
	UpdateData(FALSE);
}