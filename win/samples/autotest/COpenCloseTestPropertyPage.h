#pragma once

class COpenCloseTestPropertyPage : public CPropertyPage
{
	bool m_bStart;
	int m_totalCount;
	int m_count;
	bool m_conModel;
	bool m_initFlag;
public:
	COpenCloseTestPropertyPage();


#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPERTY_OPEN_CLOSE_TEST };
#endif

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditOpenCloseCnt();
	afx_msg void OnBnClickedButtonOpenCloseTestStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
private:
	void Stop();
	void UpdateHint();
};