
// pico_test_mfcDlg.h: 헤더 파일
//

#pragma once
#include <opencv2/core.hpp>


// CpicotestmfcDlg 대화 상자
class CpicotestmfcDlg : public CDialogEx
{
// 생성입니다.
public:
	CpicotestmfcDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PICO_TEST_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	HANDLE m_hDevice;  // Pico driver device handle
	cv::Mat m_preprocessedImage;  // Preprocessed image for display

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnDmaLedOn();
	afx_msg void OnBnClickedBtnDmaLedOff();
	afx_msg void OnBnClickedBtnModelLoad();
	afx_msg void OnBnClickedBtnImageLoad();

private:
	void DisplayImageOnControl(const cv::Mat& img);
};
