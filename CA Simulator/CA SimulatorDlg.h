#pragma once

#include "OpenGLControl.h"

// CASimulatorDlg dialog
class CASimulatorDlg : public CDialogEx
{
private:
	OpenGLControl openGLWindow;

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CASimulatorDlg(CWnd* pParent = NULL);

	// Dialog Data
	enum { IDD = IDD_CASIMULATOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBnClickedSetRuleButton();
	afx_msg void OnFileNewPattern();
	afx_msg void OnRuleMenuItems(UINT nID);
	afx_msg void OnPlaybackPlay();
	afx_msg void OnPlaybackPause();
	afx_msg void OnPlaybackFaster();
	afx_msg void OnPlaybackSlower();
	afx_msg void OnPlaybackStep();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedEvolveButton();
	afx_msg void OnGuide();
};
