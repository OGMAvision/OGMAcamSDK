#pragma once

class CPacketTimeoutPropertyPage : public CPropertyPage
{
public:
	CPacketTimeoutPropertyPage();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPERTY_PACKET_TIMEOUT };
#endif

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSet();
	DECLARE_MESSAGE_MAP()
};
