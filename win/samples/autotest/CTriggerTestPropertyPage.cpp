#include "stdafx.h"
#include "global.h"
#include "AutoTest.h"
#include "CTriggerTestPropertyPage.h"
#include "AutoTestDlg.h"

CTriggerTestPropertyPage::CTriggerTestPropertyPage()
	: CPropertyPage(IDD_PROPERTY_TRIGGER_TEST)
	, m_bStart(false), m_totalCount(0), m_count(0), m_interval(0)
{
}

void CTriggerTestPropertyPage::UpdateHint()
{
	CString str;
	str.Format(_T("%d/%d"), m_count, m_totalCount);
	SetDlgItemText(IDC_STATIC_TRIGGER_TEST_HINT, str);
}

BEGIN_MESSAGE_MAP(CTriggerTestPropertyPage, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_TRIGGER_TEST_TIMES, &CTriggerTestPropertyPage::OnEnChangeEditTriggerTestTimes)
	ON_EN_CHANGE(IDC_EDIT_TRIGGER_TEST_INTERVAL, &CTriggerTestPropertyPage::OnEnChangeEditTriggerTestInterval)
	ON_BN_CLICKED(IDC_BUTTON_TRIGGER_TEST_START, &CTriggerTestPropertyPage::OnBnClickedButtonTriggerTestStart)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CTriggerTestPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateHint();
	GetDlgItem(IDC_BUTTON_TRIGGER_TEST_START)->EnableWindow(FALSE);

	return TRUE;
}

void CTriggerTestPropertyPage::OnEnChangeEditTriggerTestTimes()
{
	m_totalCount = GetDlgItemInt(IDC_EDIT_TRIGGER_TEST_TIMES);
	UpdateHint();
	GetDlgItem(IDC_BUTTON_TRIGGER_TEST_START)->EnableWindow(m_totalCount > 0 && m_interval >= 100);
}

void CTriggerTestPropertyPage::OnEnChangeEditTriggerTestInterval()
{
	m_interval = GetDlgItemInt(IDC_EDIT_TRIGGER_TEST_INTERVAL);
	GetDlgItem(IDC_BUTTON_TRIGGER_TEST_START)->EnableWindow(m_totalCount > 0 && m_interval >= 100);
}

void CTriggerTestPropertyPage::OnTimer(UINT_PTR nIDEvent)
{
	Ogmacam_Trigger(g_hcam, 1);

	++m_count;
	UpdateHint();
	if (m_count >= m_totalCount)
	{
		Stop();
		AfxMessageBox(_T("Trigger test completed."));
	}
}

void CTriggerTestPropertyPage::Stop()
{
	KillTimer(1);
	m_bStart = g_bTriggerTest = false;
	SetDlgItemText(IDC_BUTTON_TRIGGER_TEST_START, _T("Start"));
	GetDlgItem(IDC_EDIT_TRIGGER_TEST_TIMES)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_TRIGGER_TEST_INTERVAL)->EnableWindow(TRUE);
	Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_TRIGGER, 0);
}

void CTriggerTestPropertyPage::OnBnClickedButtonTriggerTestStart()
{
	if (m_bStart)
		Stop();
	else
	{
		g_snapDir = GetAppTimeDir(_T("TriggerTest"));
		if (!PathIsDirectory((LPCTSTR)g_snapDir))
			SHCreateDirectory(m_hWnd, (LPCTSTR)g_snapDir);

		Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_TRIGGER, 1);
		m_bStart = g_bTriggerTest = true;
		m_count = 0;
		SetDlgItemText(IDC_BUTTON_TRIGGER_TEST_START, _T("Stop"));
		GetDlgItem(IDC_EDIT_TRIGGER_TEST_TIMES)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_TRIGGER_TEST_INTERVAL)->EnableWindow(FALSE);
		SetTimer(1, m_interval, nullptr);
	}
}