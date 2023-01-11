#include "stdafx.h"
#include "global.h"
#include "AutoTest.h"
#include "CPacketTimeoutPropertyPage.h"

CPacketTimeoutPropertyPage::CPacketTimeoutPropertyPage()
	: CPropertyPage(IDD_PROPERTY_PACKET_TIMEOUT)
{
}

BEGIN_MESSAGE_MAP(CPacketTimeoutPropertyPage, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CPacketTimeoutPropertyPage::OnBnClickedSet)
END_MESSAGE_MAP()

BOOL CPacketTimeoutPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	int n = 2000;
	if (g_hcam)
		Ogmacam_get_Option(g_hcam, OGMACAM_OPTION_NOPACKET_TIMEOUT, &n);
	SetDlgItemInt(IDC_EDIT1, n);
	return TRUE;
}

void CPacketTimeoutPropertyPage::OnBnClickedSet()
{
	BOOL bTrans = FALSE;
	const UINT n = GetDlgItemInt(IDC_EDIT1, &bTrans, 0);
	if (!bTrans)
	{
		GotoDlgCtrl(GetDlgItem(IDC_EDIT1));
		AfxMessageBox(L"Bad input.", MB_OK | MB_ICONWARNING);
		return;
	}
	if (g_hcam)
		Ogmacam_put_Option(g_hcam, OGMACAM_OPTION_NOPACKET_TIMEOUT, n);
}