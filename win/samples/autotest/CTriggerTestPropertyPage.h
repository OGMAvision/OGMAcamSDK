#pragma once

class CTriggerTestPropertyPage : public CPropertyPage
{
	bool m_bStart;
	int m_totalCount;
	int m_count;
	int m_interval;
public:
	CTriggerTestPropertyPage();
	
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPERTY_TRIGGER_TEST };
#endif

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditTriggerTestTimes();
	afx_msg void OnEnChangeEditTriggerTestInterval();
	afx_msg void OnBnClickedButtonTriggerTestStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
private:
	void Stop();
	void UpdateHint();
};
