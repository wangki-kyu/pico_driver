# Pico Driver

Raspberry Pi Pico와 PC를 USB로 연결하여 센서 데이터를 읽고 머신러닝 추론을 수행하는 Windows KMDF 기반 드라이버입니다.

작성할 내용 
- kernel debugging setting 하기 

## 주요 기능

- **실시간 온도 읽기**: Pico의 온도 센서 데이터를 주기적으로 읽음
- **이미지 추론**: FOMO 기반 얼굴 감지 모델을 활용한 실시간 객체 인식
- **인터럽트 처리**: Pico의 인터럽트 신호를 안정적으로 처리
- **LED 제어**: 온/오프, 토글 등 기본 제어 지원
- **DMA 기반 데이터 전송**: 대용량 데이터 효율적 전송

## 프로젝트 구조

```
pico_driver/
├── pico_driver/          # 핵심 드라이버 코드
│   ├── Driver.c/h        # 드라이버 메인 로직
│   ├── Device.c/h        # USB 디바이스 통신
│   ├── Queue.c/h         # IOCTL 요청 큐 처리
│   └── Trace.h           # 디버깅 로깅
├── pico_test_app/        # 콘솔 테스트 애플리케이션
├── pico_test_mfc/        # GUI 기반 테스트 앱 (MFC)
├── model/                # 추론 모델 파일
├── include/              # OpenCV 등 외부 라이브러리
├── lib/                  # 컴파일된 라이브러리
└── plan/                 # 기술 사양서 및 문서
    ├── MODEL_SPECIFICATION.md
    └── TEMPLATE_GUIDE.md
```

## 주요 IOCTL 인터페이스

| IOCTL | 기능 | 입력 | 출력 |
|-------|------|------|------|
| `IOCTL_PICO_READ_TEMP` | 온도 센서 읽기 | - | 온도값 (float) |
| `IOCTL_PICO_RUN_INFERENCE` | 모델 추론 실행 | 이미지 데이터 | 추론 결과 |
| `IOCTL_PICO_READ_INTERRUPT` | 인터럽트 신호 읽기 | - | 인터럽트 상태 |
| `IOCTL_PICO_DMA_WRITE` | DMA로 데이터 전송 | 대용량 데이터 | 전송 바이트 |
| `IOCTL_PICO_MODEL_LOAD` | 모델 로드 | 모델 경로 | 성공/실패 |

## 빌드 및 설치

### 필수 사항
- Visual Studio 2019 이상
- Windows Driver Kit (WDK) 설치
- CMake 3.16+

### 빌드

```bash
# Visual Studio 솔루션으로 빌드
# pico_driver.sln을 Visual Studio에서 열고 빌드

# 또는 명령줄에서
msbuild pico_driver.sln /p:Configuration=Release
```

### 드라이버 설치

```bash
# 테스트용 자체 서명 인증서로 서명
signtool sign /f cert.pfx /fd SHA256 pico_driver.sys

# 드라이버 설치 (관리자 권한 필요)
pnputil /add-driver pico_driver.inf /install
```

## 유저모드 어플리케이션 테스트 (GUI mfc)

### GUI 테스트 앱 실행
vm 환경에 재배포 패키지가 설치되지 않았다면 MT 빌드 필요 

#### opencv 필수 dll
`opencv_videoio_ffmpeg4120_64.dll`, `opencv_world4120.dll` 


## 모델 정보

FOMO(Faster Objects, More Objects) 기반 얼굴 감지 모델을 사용합니다.

- **입력**: 64×64 그레이스케일 이미지
- **출력**: 8×8 그리드의 클래스 확률값
- **특징**: 매우 가볍고 빠름 (15-40ms)
- **제약**: 정확한 바운딩박스 미제공

자세한 사항은 [MODEL_SPECIFICATION.md](plan/MODEL_SPECIFICATION.md) 참고

## 주요 개선사항

- ✅ 실시간 온도 모니터링 기능 추가
- ✅ Interrupt endpoint를 통한 안정적인 신호 처리
- ✅ 올바른 device PID 적용
- ✅ 상세한 디버깅 로그 추가
- ✅ 이미지 추론 성능 최적화

## 문제 해결

### 드라이버가 설치되지 않는 경우
- Windows Test Mode 활성화 확인 (`bcdedit /set testsigning on`)
- 인증서가 신뢰할 수 있는 루트 저장소에 등록되어 있는지 확인

### Pico와 연결되지 않는 경우
- USB 케이블 및 포트 확인
- 디바이스 매니저에서 "알 수 없는 장치" 확인
- 재시작 후 드라이버 재설치

### 추론 결과가 이상한 경우
- 모델 파일이 올바르게 로드되었는지 확인
- 입력 이미지의 전처리 단계 검증
- [MODEL_SPECIFICATION.md](plan/MODEL_SPECIFICATION.md)의 전처리 과정 참고

## 라이센스

프로젝트 라이센스 정보는 프로젝트 루트 디렉토리 확인

## 참고자료

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [TensorFlow Lite](https://www.tensorflow.org/lite)
- [Edge Impulse](https://edgeimpulse.com/)
