
// Load SRTDlg.h : header file
//

#pragma once

#include "StaticDrawSRT.h"


// CLoadSRTDlg dialog
class CLoadSRTDlg : public CDialogEx
{
// Construction
public:
	CLoadSRTDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOADSRT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
   afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnBnClickedButtonLoadSrtFile();

   afx_msg void OnBnClickedButtonExportTXT();
   afx_msg void OnBnClickedButtonExportSRT();

private:
   CStaticDrawSRT                  m_staticDrawSRT;
   CButton                         m_btnLoad;
   CSliderCtrl                     m_sliderProgress;
   CButton                         m_btnExportTXT;
   CButton                         m_btnExportSRT;
};
