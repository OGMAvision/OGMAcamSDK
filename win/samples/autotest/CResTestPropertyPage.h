#pragma once

class CResTestPropertyPage : public CPropertyPage
{
	bool m_bStart;
	int m_totalCount;
	int m_count, m_resCount;
public:
	CResTestPropertyPage();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPERTY_RESOLUTION_TEST };
#endif

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditResolutionTestCount();
	afx_msg void OnBnClickedButtonResolutionTestStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
private:
	void Stop();
	void UpdateHint();
};