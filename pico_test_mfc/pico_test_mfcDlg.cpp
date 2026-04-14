
// pico_test_mfcDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "pico_test_mfc.h"
#include "pico_test_mfcDlg.h"
#include "afxdialogex.h"
#include "../include/util.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <atlstr.h>
#include <vector>

#ifdef _DEBUG
#pragma comment(lib, "../lib/opencv/opencv_world4120d.lib")
#else
#pragma comment(lib, "../lib/opencv/opencv_world4120.lib")
#endif

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
	ON_BN_CLICKED(IDC_BTN_IMAGE_LOAD, &CpicotestmfcDlg::OnBnClickedBtnImageLoad)
	ON_MESSAGE(WM_INTERRUPT_DATA, &CpicotestmfcDlg::OnInterruptData)
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
		InitInterruptRead();
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

	CleanupInterruptRead();

	if (m_hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
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
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
		_T("All Files (*.*)|*.*||"), this);

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

void CpicotestmfcDlg::OnBnClickedBtnImageLoad()
{
	// Check device connection
	if (m_hDevice == INVALID_HANDLE_VALUE) {
		MessageBox(_T("Device not connected."), _T("Error"));
		return;
	}

	// Step 1: File dialog to select image file
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
		_T("Image Files (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp|All Files (*.*)|*.*||"), this);

	if (fileDlg.DoModal() != IDOK) {
		return;  // User cancelled
	}

	// Convert CString to const char* for LoadImageFromFile
	CString cstrPath = fileDlg.GetPathName();
	const char* pszPath = CW2A(cstrPath);

	// Step 1: Load image using LoadImageFromFile
	cv::Mat originalImg = LoadImageFromFile(pszPath);

	if (originalImg.empty()) {
		MessageBox(_T("Failed to load image."), _T("Error"));
		return;
	}

	// 원본 이미지 show! 
	DisplayImageOnControl(originalImg);

	// Display original image info
	CString origMsg;
	origMsg.Format(_T("Original image loaded!\n%d x %d x %d"),
		originalImg.cols, originalImg.rows, originalImg.channels());
	MessageBox(origMsg, _T("Original Image"));

	// Step 2: Preprocess image to INT8 (64x64)
	std::vector<int8_t> int8Data = PreprocessImageToInt8(originalImg);

	if (int8Data.empty()) {
		MessageBox(_T("Failed to preprocess image."), _T("Error"));
		return;
	}

	// Build input protocol packet [0x21][ImageData...]
	DWORD imageDataSize = 64 * 64;  // Preprocessed image is 64x64
	DWORD totalSize = 1 + imageDataSize;  // 1 byte for cmd + image data
	PUCHAR pInBuffer = new UCHAR[totalSize];

	if (!pInBuffer) {
		MessageBox(_T("Memory allocation failed."), _T("Error"));
		return;
	}

	// Build input protocol packet
	pInBuffer[0] = 0x21;  // Command byte for inference
	memcpy(&pInBuffer[1], int8Data.data(), imageDataSize);

	// Step 4: Allocate output buffer for inference results (64 bytes for 8x8 heatmap)
	DWORD resultBufferSize = 64;
	PUCHAR pOutBuffer = new UCHAR[resultBufferSize];

	if (!pOutBuffer) {
		MessageBox(_T("Memory allocation failed."), _T("Error"));
		delete[] pInBuffer;
		return;
	}

	// Create OVERLAPPED structure for asynchronous I/O
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!overlapped.hEvent) {
		MessageBox(_T("Failed to create event."), _T("Error"));
		delete[] pInBuffer;
		delete[] pOutBuffer;
		return;
	}

	DWORD bytesReturned = 0;

	// Step 3: Send image data via IOCTL (asynchronous)
	if (DeviceIoControl(m_hDevice, IOCTL_PICO_RUN_INFERENCE,
		pInBuffer, totalSize,           // Input: image + cmd
		pOutBuffer, resultBufferSize,   // Output: inference results (8x8 heatmap)
		&bytesReturned, &overlapped)) {
		// Request completed synchronously
		CString msg;
		msg.Format(_T("Inference - Success (sync)!\nResult: %lu bytes"), bytesReturned);
		MessageBox(msg, _T("Success"));

		// Step 5: Convert output buffer to float heatmap and apply blur
		std::vector<float> heatmap(64);
		for (int i = 0; i < 64; i++) {
			heatmap[i] = (float)pOutBuffer[i] / 255.0f;  // Normalize to 0.0-1.0
		}

		// Apply blur based on heatmap
		cv::Mat blurredImg = BlurImageByHeatmap(originalImg, heatmap, 0.5f);
		DisplayImageOnControl(blurredImg);

		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else if (GetLastError() == ERROR_IO_PENDING) {
		// Request is pending - wait for completion
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 30000);  // 30 second timeout

		if (waitResult == WAIT_OBJECT_0) {
			// Operation completed - get actual bytes returned
			GetOverlappedResult(m_hDevice, &overlapped, &bytesReturned, FALSE);

			CString msg;
			msg.Format(_T("Inference - Success (async)!\nResult: %lu bytes"), bytesReturned);
			MessageBox(msg, _T("Success"));

			// Step 5: Convert output buffer to float heatmap and apply blur
			std::vector<float> heatmap(64);
			for (int i = 0; i < 64; i++) {
				heatmap[i] = (float)pOutBuffer[i] / 255.0f;  // Normalize to 0.0-1.0
			}

			// Apply blur based on heatmap
			cv::Mat blurredImg = BlurImageByHeatmap(originalImg, heatmap, 0.5f);
			DisplayImageOnControl(blurredImg);
		}
		else if (waitResult == WAIT_TIMEOUT) {
			MessageBox(_T("Inference - Timeout!"), _T("Error"));
		}
		else {
			MessageBox(_T("Inference - Wait failed!"), _T("Error"));
		}

		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("Inference - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
}



void CpicotestmfcDlg::DisplayImageOnControl(const cv::Mat& img)
{
	cv::Mat displayImg;

	// If grayscale, convert to BGR for display
	if (img.channels() == 1) {
		cv::cvtColor(img, displayImg, cv::COLOR_GRAY2BGR);
	} else {
		displayImg = img.clone();
	}

	// Resize for better visibility (scale to 256x256)
	cv::Mat displayImgLarge;
	cv::resize(displayImg, displayImgLarge, cv::Size(256, 256), 0, 0, cv::INTER_LINEAR);

	// Create HBITMAP
	int rows = displayImgLarge.rows;
	int cols = displayImgLarge.cols;

	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = cols;
	bmpInfo.bmiHeader.biHeight = -rows;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	void* pBits = nullptr;
	HDC hdc = GetDC()->GetSafeHdc();
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, &pBits, nullptr, 0);

	if (!hBitmap) {
		return;
	}

	if (displayImgLarge.isContinuous()) {
		memcpy(pBits, displayImgLarge.data, rows * cols * 3);
	} else {
		DeleteObject(hBitmap);
		return;
	}

	// Display on Picture Control and clean up old bitmap
	CStatic* pPicCtrl = (CStatic*)GetDlgItem(IDC_PIC_IMG);
	if (pPicCtrl) {
		HBITMAP hOldBitmap = pPicCtrl->SetBitmap(hBitmap);
		if (hOldBitmap) {
			DeleteObject(hOldBitmap);  // Fix: cleanup old HBITMAP
		}
	} else {
		DeleteObject(hBitmap);
	}
}

void CpicotestmfcDlg::ParseAndApplyBlur(const cv::Mat& originalImg, PUCHAR pOutBuffer,
	DWORD bytesReturned, DWORD resultBufferSize)
{
	// Parse bounding boxes and apply blur
	if (pOutBuffer[0] == 0x22 && bytesReturned > 2) {  // Response code verification
		uint8_t boxCount = pOutBuffer[1];
		uint16_t idx = 2;

		// Clone original image for blur processing
		cv::Mat displayImg = originalImg.clone();

		// Process each bounding box
		for (uint8_t i = 0; i < boxCount && idx + 5 <= bytesReturned; i++) {
			uint8_t x = pOutBuffer[idx++];           // 0-255 normalized
			uint8_t y = pOutBuffer[idx++];
			uint8_t w = pOutBuffer[idx++];
			uint8_t h = pOutBuffer[idx++];
			uint8_t confidence = pOutBuffer[idx++];

			// Convert normalized coordinates (0-255) to original image size
			float norm_x = (float)x / 255.0f;
			float norm_y = (float)y / 255.0f;
			float norm_w = (float)w / 255.0f;
			float norm_h = (float)h / 255.0f;

			int orig_x = (int)(norm_x * originalImg.cols);
			int orig_y = (int)(norm_y * originalImg.rows);
			int orig_w = (int)(norm_w * originalImg.cols);
			int orig_h = (int)(norm_h * originalImg.rows);

			// Boundary check
			int x1 = std::max(0, orig_x);
			int y1 = std::max(0, orig_y);
			int x2 = std::min(originalImg.cols - 1, orig_x + orig_w);
			int y2 = std::min(originalImg.rows - 1, orig_y + orig_h);

			// Apply blur to detected region
			if (x2 > x1 && y2 > y1) {
				cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
				cv::Mat roiImg = displayImg(roi);
				cv::blur(roiImg, roiImg, cv::Size(25, 25));  // Blur kernel size: 25x25

				// Draw bounding box for visualization
				cv::rectangle(displayImg, roi, cv::Scalar(0, 255, 0), 2);
			}
		}

		// Display blurred image
		DisplayImageOnControl(displayImg);

		CString resultMsg;
		resultMsg.Format(_T("Successfully blurred %d face(s)"), boxCount);
		MessageBox(resultMsg, _T("Blur Complete"));
	} else {
		MessageBox(_T("Invalid response format"), _T("Error"));
	}
}

void CpicotestmfcDlg::HandleImageInference()
{
	// Image file dialog to select image file
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST,
		_T("All Files (*.*)|*.*||"), this);

	if (fileDlg.DoModal() != IDOK) {
		return;  // User cancelled
	}

	// Convert CString to std::string for OpenCV
	CString cstrPath = fileDlg.GetPathName();
	const char* pszPath = CW2A(cstrPath);
	std::string imagePath(pszPath);

	// Load image using OpenCV
	cv::Mat originalImg = cv::imread(imagePath);

	if (originalImg.empty()) {
		MessageBox(_T("Failed to load image."), _T("Error"));
		return;
	}

	// Step 1: Display original image
	DisplayImageOnControl(originalImg);
	CString origMsg;
	origMsg.Format(_T("Original image loaded!\n%d x %d x %d"),
		originalImg.cols, originalImg.rows, originalImg.channels());
	MessageBox(origMsg, _T("Original Image"));

	// Step 2: Convert to grayscale
	cv::Mat grayImg;
	cv::cvtColor(originalImg, grayImg, cv::COLOR_BGR2GRAY);

	// Step 3: Resize to 64x64
	cv::Mat resizedImg;
	cv::resize(grayImg, resizedImg, cv::Size(64, 64));

	// Store preprocessed image
	m_preprocessedImage = resizedImg.clone();

	// Step 4: Display preprocessed image
	//DisplayImageOnControl(resizedImg);

	// 비동기로 DeviceIOControl 보내기
	// IOCTL_PICO_RUN_INFERENCE IOCTL CODE
	// Protocol (Write to device): [0x21][ImageData...]
	// Protocol (Read from device): [결과 데이터...]
	// cmd = 0x21

	DWORD imageDataSize = resizedImg.rows * resizedImg.cols * resizedImg.channels();
	DWORD totalSize = 1 + imageDataSize;  // 1 byte for cmd + image data
	PUCHAR pInBuffer = new UCHAR[totalSize];

	if (!pInBuffer) {
		MessageBox(_T("Memory allocation failed."), _T("Error"));
		return;
	}

	// Allocate output buffer for inference results
	DWORD resultBufferSize = 64;
	PUCHAR pOutBuffer = new UCHAR[resultBufferSize];

	if (!pOutBuffer) {
		MessageBox(_T("Memory allocation failed."), _T("Error"));
		delete[] pInBuffer;
		return;
	}

	// Build input protocol packet
	pInBuffer[0] = 0x21;  // Command byte for inference

	// INT8 변환: uint8[0,255] → int8[-128,127]
	// MODEL_SPECIFICATION.md 기준: int8 = (int16_t)uint8 - 128
	std::vector<int8_t> int8Data(imageDataSize);
	for (DWORD i = 0; i < imageDataSize; i++) {
		int8Data[i] = (int8_t)((int16_t)resizedImg.data[i] - 128);
	}
	memcpy(&pInBuffer[1], int8Data.data(), imageDataSize);

	// Create OVERLAPPED structure for asynchronous I/O
	OVERLAPPED overlapped = {};
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!overlapped.hEvent) {
		MessageBox(_T("Failed to create event."), _T("Error"));
		delete[] pInBuffer;
		delete[] pOutBuffer;
		return;
	}

	DWORD bytesReturned = 0;

	// Send image data and receive inference result via IOCTL (two-phase: write + read)
	if (DeviceIoControl(m_hDevice, IOCTL_PICO_RUN_INFERENCE,
		pInBuffer, totalSize,           // Input: image + cmd
		pOutBuffer, resultBufferSize,   // Output: inference results
		&bytesReturned, &overlapped)) {
		// Request completed synchronously
		CString msg;
		msg.Format(_T("Inference - Success (sync)!\nInput: %lu bytes\nResult: %lu bytes"), imageDataSize, bytesReturned);
		MessageBox(msg, _T("Success"));

		ParseAndApplyBlur(originalImg, pOutBuffer, bytesReturned, resultBufferSize);

		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else if (GetLastError() == ERROR_IO_PENDING) {
		// Request is pending - wait for completion
		DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 30000);  // 30 second timeout

		if (waitResult == WAIT_OBJECT_0) {
			// Operation completed - get actual bytes returned
			GetOverlappedResult(m_hDevice, &overlapped, &bytesReturned, FALSE);

			CString msg;
			msg.Format(_T("Inference - Success (async)!\nInput: %lu bytes\nResult: %lu bytes"), imageDataSize, bytesReturned);
			MessageBox(msg, _T("Success"));

			ParseAndApplyBlur(originalImg, pOutBuffer, bytesReturned, resultBufferSize);
		}
		else if (waitResult == WAIT_TIMEOUT) {
			MessageBox(_T("Inference - Timeout!"), _T("Error"));
		}
		else {
			MessageBox(_T("Inference - Wait failed!"), _T("Error"));
		}

		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
	else {
		DWORD error = GetLastError();
		CString msg;
		msg.Format(_T("Inference - Failed: 0x%08X"), error);
		MessageBox(msg, _T("Error"));
		delete[] pInBuffer;
		delete[] pOutBuffer;
		CloseHandle(overlapped.hEvent);
	}
}

// OpenCV로 이미지를 읽어오는 함수
cv::Mat CpicotestmfcDlg::LoadImageFromFile(const char* imagePath)
{
	cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

	if (image.empty()) {
		fprintf(stderr, "Error: Cannot open image file: %s\n", imagePath);
		return cv::Mat();
	}

	printf("Image loaded: %d x %d\n", image.cols, image.rows);
	return image;
}

// 이미지 전처리 함수
// 입력: RGB 컬러 이미지 (cv::Mat)
// 출력: [1, 64, 64, 1] INT8 배열
std::vector<int8_t> CpicotestmfcDlg::PreprocessImageToInt8(const cv::Mat& rgbImage)
{
	if (rgbImage.empty()) {
		fprintf(stderr, "Error: Empty image\n");
		return std::vector<int8_t>();
	}

	// 1단계: 64×64로 리사이징
	cv::Mat resized;
	cv::resize(rgbImage, resized, cv::Size(64, 64), 0, 0, cv::INTER_LINEAR);

	// 2단계: Grayscale 변환
	cv::Mat gray;
	cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);

	// 3단계: uint8 [0,255] → int8 양자화
	// Edge Impulse 표준: 0-255를 -128~127로 매핑
	std::vector<int8_t> input_data(64 * 64);

	for (int i = 0; i < 64 * 64; i++) {
		uint8_t pixel = gray.data[i];
		// 방식 1: 단순 매핑 (pixel - 128)
		input_data[i] = static_cast<int8_t>(static_cast<int16_t>(pixel) - 128);
	}

	printf("Preprocessing complete: 64x64 grayscale converted to INT8\n");
	printf("Sample input values (첫 10개): ");
	for (int i = 0; i < 10; i++) {
		printf("%d ", input_data[i]);
	}
	printf("\n");

	return input_data;
}

// 추론 결과 기반 선택적 Blur 처리 함수
// 입력: 원본 이미지, runInference의 output (8x8 확률값), threshold
// 출력: blur 처리된 이미지
cv::Mat CpicotestmfcDlg::BlurImageByHeatmap(const cv::Mat& original_image, const std::vector<float>& output_probs, float threshold)
{
	if (original_image.empty()) {
		fprintf(stderr, "Error: Empty original image\n");
		return cv::Mat();
	}

	if (output_probs.size() != 64) {
		fprintf(stderr, "Error: Output size mismatch (expected 64, got %zu)\n", output_probs.size());
		return cv::Mat();
	}

	cv::Mat result = original_image.clone();

	// 8x8 heatmap을 원본 이미지 크기에 매핑
	int heatmap_size = 8;
	int cell_width = original_image.cols / heatmap_size;   // 원본 이미지 가로 크기 / 8
	int cell_height = original_image.rows / heatmap_size;  // 원본 이미지 세로 크기 / 8

	printf("Original image size: %d x %d\n", original_image.cols, original_image.rows);
	printf("Cell size: %d x %d\n", cell_width, cell_height);

	for (int h_row = 0; h_row < heatmap_size; h_row++) {
		for (int h_col = 0; h_col < heatmap_size; h_col++) {
			float prob = output_probs[h_row * heatmap_size + h_col];

			// 확률이 threshold 이상인 영역만 blur 처리
			if (prob >= threshold) {
				// 원본 이미지 좌표로 변환
				int x_start = h_col * cell_width;
				int y_start = h_row * cell_height;
				int x_end = x_start + cell_width;
				int y_end = y_start + cell_height;

				// 경계 체크
				x_start = std::max(0, x_start);
				y_start = std::max(0, y_start);
				x_end = std::min(result.cols, x_end);
				y_end = std::min(result.rows, y_end);

				// ROI(Region of Interest) 추출 및 blur 처리
				if (x_end > x_start && y_end > y_start) {
					cv::Rect roi(x_start, y_start, x_end - x_start, y_end - y_start);
					cv::Mat roi_image = result(roi);

					// 최강 Gaussian blur 적용 (매우 큰 커널 + 높은 sigma)
					cv::GaussianBlur(roi_image, roi_image, cv::Size(51, 51), 10.0);
					cv::GaussianBlur(roi_image, roi_image, cv::Size(51, 51), 10.0);  // 2회 적용
				}
			}
		}
	}

	printf("Image blur processing complete (threshold: %.2f)\n", threshold);
	return result;
}

// interrupt 초기화 함수 
void CpicotestmfcDlg::InitInterruptRead()
{
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWaitThread = NULL;

	SubmitInterruptRead();
}

void CpicotestmfcDlg::SubmitInterruptRead()
{
	DWORD bytesReturned = 0;
	ResetEvent(m_overlapped.hEvent);

	BOOL result = DeviceIoControl(
		m_hDevice,
		IOCTL_PICO_READ_INTERRUPT,
		NULL, 0,
		m_interruptBuf, sizeof(m_interruptBuf),
		&bytesReturned,
		&m_overlapped
	);

	if (!result && GetLastError() != ERROR_IO_PENDING) {
		return;
	}

	// hEvent가 시그널되면 InterruptCallback 호출 (Windows ThreadPool)
	RegisterWaitForSingleObject(
		&m_hWaitThread,
		m_overlapped.hEvent,
		InterruptCallback,
		this,
		INFINITE,
		WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE
	);
}

VOID CALLBACK CpicotestmfcDlg::InterruptCallback(PVOID lpParam, BOOLEAN)
{
	CpicotestmfcDlg* pDlg = (CpicotestmfcDlg*)lpParam;

	DWORD bytesTransferred = 0;
	if (!GetOverlappedResult(pDlg->m_hDevice, &pDlg->m_overlapped, &bytesTransferred, FALSE)) {
		return;
	}

	// 0xBB: Pico interrupt marker, [1]: 정수부, [2]: 소수부
	if (bytesTransferred >= 3 && pDlg->m_interruptBuf[0] == 0xBB) {
		WPARAM wParam = MAKEWPARAM(pDlg->m_interruptBuf[2], pDlg->m_interruptBuf[1]);
		pDlg->PostMessage(WM_INTERRUPT_DATA, wParam, 0);
	}

	// 다음 read 등록 (루프)
	UnregisterWait(pDlg->m_hWaitThread);
	pDlg->m_hWaitThread = NULL;
	pDlg->SubmitInterruptRead();
}

LRESULT CpicotestmfcDlg::OnInterruptData(WPARAM wParam, LPARAM)
{
	BYTE intPart  = (BYTE)HIWORD(wParam);  // buf[1] = 정수부
	BYTE fracPart = (BYTE)LOWORD(wParam);  // buf[2] = 소수부

	CString tempStr;
	tempStr.Format(_T("%d.%02d °C"), intPart, fracPart);
	GetDlgItem(IDC_EDIT2)->SetWindowText(tempStr);

	return 0;
}

void CpicotestmfcDlg::CleanupInterruptRead()
{
	if (m_hWaitThread) {
		UnregisterWaitEx(m_hWaitThread, INVALID_HANDLE_VALUE);
		m_hWaitThread = NULL;
	}
	if (m_overlapped.hEvent) {
		CloseHandle(m_overlapped.hEvent);
		m_overlapped.hEvent = NULL;
	}
}
