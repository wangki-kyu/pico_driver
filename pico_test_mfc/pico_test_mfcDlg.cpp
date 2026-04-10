
// pico_test_mfcDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "pico_test_mfc.h"
#include "pico_test_mfcDlg.h"
#include "afxdialogex.h"
#include "../include/util.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CpicotestmfcDlg 대화 상자



CpicotestmfcDlg::CpicotestmfcDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PICO_TEST_MFC_DIALOG, pParent), m_hDevice(INVALID_HANDLE_VALUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpicotestmfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CpicotestmfcDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CpicotestmfcDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CpicotestmfcDlg::OnBnClickedButton2)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON3, &CpicotestmfcDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_DMA_LED_ON, &CpicotestmfcDlg::OnBnClickedBtnDmaLedOn)
	ON_BN_CLICKED(IDC_BTN_DMA_LED_OFF, &CpicotestmfcDlg::OnBnClickedBtnDmaLedOff)
	ON_BN_CLICKED(IDC_BTN_MODEL_LOAD, &CpicotestmfcDlg::OnBnClickedBtnModelLoad)
END_MESSAGE_MAP()


// CpicotestmfcDlg 메시지 처리기

BOOL CpicotestmfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// Find and open Pico driver device
	m_hDevice = FindPicoDriverDevice();

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		MessageBox(_T("Pico driver device found and opened successfully!"), _T("Success"));
	} else {
		MessageBox(_T("Failed to find or open the pico driver device."), _T("Error"));
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CpicotestmfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CpicotestmfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CpicotestmfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CpicotestmfcDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hDevice);
	}
}


// Event Handler
void CpicotestmfcDlg::OnBnClickedButton1()
{
	// LED ON
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	DWORD bytesReturned = 0;
	UCHAR command = LED_ON;

	if (DeviceIoControl(m_hDevice, IOCTL_PICO_TEST_WRITE, &command, sizeof(command), NULL, 0, &bytesReturned, NULL)) {
		MessageBox(_T("LED ON - Success!"), _T("Success"));
	} else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("LED ON - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
	}
}

void CpicotestmfcDlg::OnBnClickedButton2()
{
	// LED OFF
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	DWORD bytesReturned = 0;
	UCHAR command = LED_OFF;

	if (DeviceIoControl(m_hDevice, IOCTL_PICO_TEST_WRITE, &command, sizeof(command), NULL, 0, &bytesReturned, NULL)) {
		MessageBox(_T("LED OFF - Success!"), _T("Success"));
	} else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("LED OFF - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
	}
}

void CpicotestmfcDlg::OnBnClickedButton3()
{
	// TEMP READ
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	DWORD bytesReturned = 0;
	UCHAR command = SENSOR_READ_TEMP;
	UCHAR outputBuffer[64] = { 0, };

	if (DeviceIoControl(m_hDevice, IOCTL_PICO_READ_TEMP, &command, sizeof(command), &outputBuffer, sizeof(outputBuffer), &bytesReturned, NULL)) {
		// outputBuffer[0] -> 정수부 (예: 25도)
		// outputBuffer[1] -> 소수부 (예: 67)
		// 조합: "25.67"
		CString tempStr;
		tempStr.Format(_T("%.2d.%.2d"), outputBuffer[0], outputBuffer[1]);

		GetDlgItem(IDC_EDIT1)->SetWindowText(tempStr);
		MessageBox(_T("Temperature read successfully!"), _T("Success"));
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("READ TEMP - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
	}
}


void CpicotestmfcDlg::OnBnClickedBtnDmaLedOn()
{
	// DMA LED ON (Asynchronous with OverlappedIO)
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	// Create OVERLAPPED structure for asynchronous I/O
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!overlapped.hEvent) {
		MessageBox(_T("Failed to create event."), _T("Error"));
		return;
	}

	UCHAR command = LED_ON;
	DWORD bytesReturned = 0;

	// Send DMA write request asynchronously
	if (DeviceIoControl(m_hDevice, IOCTL_PICO_DMA_WRITE, &command, sizeof(command),
		NULL, 0, &bytesReturned, &overlapped)) {
		// Request completed synchronously
		MessageBox(_T("DMA LED ON - Success (sync)!"), _T("Success"));
		CloseHandle(overlapped.hEvent);
	}
	else if (GetLastError() == ERROR_IO_PENDING) {
		// Request is pending - wait for completion
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 5000);  // 5 second timeout

		if (waitResult == WAIT_OBJECT_0) {
			// Operation completed
			MessageBox(_T("DMA LED ON - Success (async)!"), _T("Success"));
		}
		else if (waitResult == WAIT_TIMEOUT) {
			MessageBox(_T("DMA LED ON - Timeout!"), _T("Error"));
		}
		else {
			MessageBox(_T("DMA LED ON - Wait failed!"), _T("Error"));
		}

		CloseHandle(overlapped.hEvent);
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("DMA LED ON - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
		CloseHandle(overlapped.hEvent);
	}
}

void CpicotestmfcDlg::OnBnClickedBtnDmaLedOff()
{
	// DMA LED OFF (Asynchronous with OverlappedIO)
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	// Create OVERLAPPED structure for asynchronous I/O
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!overlapped.hEvent) {
		MessageBox(_T("Failed to create event."), _T("Error"));
		return;
	}

	UCHAR command = LED_OFF;
	DWORD bytesReturned = 0;

	// Send DMA write request asynchronously
	if (DeviceIoControl(m_hDevice, IOCTL_PICO_DMA_WRITE, &command, sizeof(command),
		NULL, 0, &bytesReturned, &overlapped)) {
		// Request completed synchronously
		MessageBox(_T("DMA LED OFF - Success (sync)!"), _T("Success"));
		CloseHandle(overlapped.hEvent);
	}
	else if (GetLastError() == ERROR_IO_PENDING) {
		// Request is pending - wait for completion
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 5000);  // 5 second timeout

		if (waitResult == WAIT_OBJECT_0) {
			// Operation completed
			MessageBox(_T("DMA LED OFF - Success (async)!"), _T("Success"));
		}
		else if (waitResult == WAIT_TIMEOUT) {
			MessageBox(_T("DMA LED OFF - Timeout!"), _T("Error"));
		}
		else {
			MessageBox(_T("DMA LED OFF - Wait failed!"), _T("Error"));
		}

		CloseHandle(overlapped.hEvent);
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("DMA LED OFF - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
		CloseHandle(overlapped.hEvent);
	}
}

void CpicotestmfcDlg::OnBnClickedBtnModelLoad()
{
	// Model Load via DMA (Asynchronous with OverlappedIO)
	// Protocol: [0x20][dataLen_HIGH][dataLen_LOW][ModelData...]
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	// File dialog to select model file
	CFileDialog fileDlg(TRUE, _T("bin"), _T("*.bin"), OFN_FILEMUSTEXIST,
		_T("Binary Files (*.bin)|*.bin|All Files (*.*)|*.*||"), this);

	if (fileDlg.DoModal() != IDOK) {
		return;  // User cancelled
	}

	// Open file
	CFile modelFile;
	if (!modelFile.Open(fileDlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite)) {
		MessageBox(_T("Failed to open model file."), _T("Error"));
		return;
	}

	ULONGLONG fileSize = modelFile.GetLength();

	// Check size (50KB max)
	const ULONGLONG MAX_MODEL_SIZE = 60 * 1024;  // 60KB
	if (fileSize > MAX_MODEL_SIZE) {
		CString msg;
		msg.Format(_T("Model file too large: %llu bytes (max %llu bytes)"), fileSize, MAX_MODEL_SIZE);
		MessageBox(msg, _T("Error"));
		modelFile.Close();
		return;
	}

	// Allocate buffer for protocol: [0x20][HIGH][LOW][ModelData]
	// Total size = 1 (cmd) + 2 (length) + fileSize
	DWORD totalSize = 1 + 2 + (DWORD)fileSize;
	PUCHAR pBuffer = new UCHAR[totalSize];

	if (!pBuffer) {
		MessageBox(_T("Memory allocation failed."), _T("Error"));
		modelFile.Close();
		return;
	}

	// Build protocol packet
	pBuffer[0] = 0x20;  // Command byte
	pBuffer[1] = (UCHAR)((fileSize >> 8) & 0xFF);  // Data length HIGH
	pBuffer[2] = (UCHAR)(fileSize & 0xFF);         // Data length LOW

	// Read model file data
	UINT bytesRead = modelFile.Read(&pBuffer[3], (UINT)fileSize);
	modelFile.Close();

	if ((ULONGLONG)bytesRead != fileSize) {
		MessageBox(_T("Failed to read complete model file."), _T("Error"));
		delete[] pBuffer;
		return;
	}

	// Create OVERLAPPED structure for asynchronous I/O
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!overlapped.hEvent) {
		MessageBox(_T("Failed to create event."), _T("Error"));
		delete[] pBuffer;
		return;
	}

	DWORD bytesReturned = 0;

	// Send model data via DMA write asynchronously
	if (DeviceIoControl(m_hDevice, IOCTL_PICO_MODEL_LOAD, pBuffer, totalSize,
		NULL, 0, &bytesReturned, &overlapped)) {
		// Request completed synchronously
		MessageBox(_T("Model Load - Success (sync)!"), _T("Success"));
		delete[] pBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else if (GetLastError() == ERROR_IO_PENDING) {
		// Request is pending - wait for completion
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 30000);  // 30 second timeout (large file)

		if (waitResult == WAIT_OBJECT_0) {
			// Operation completed
			CString msg;
			msg.Format(_T("Model Load - Success (async)! Size: %llu bytes"), fileSize);
			MessageBox(msg, _T("Success"));
		}
		else if (waitResult == WAIT_TIMEOUT) {
			MessageBox(_T("Model Load - Timeout!"), _T("Error"));
		}
		else {
			MessageBox(_T("Model Load - Wait failed!"), _T("Error"));
		}

		delete[] pBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("Model Load - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
		delete[] pBuffer;
		CloseHandle(overlapped.hEvent);
	}
}
