# Pico Driver: KMDF 기반 외장형 연산 가속 시스템

본 프로젝트는 **Raspberry Pi Pico**를 활용하여 Windows 환경에서 특정 연산을 분담하는 외장형 가속기 인터페이스를 구현한 프로젝트입니다. 범용 MCU의 한계를 커널 수준의 드라이버 설계로 보완하여 
고속 데이터 전송 및 실시간 제어 파이프라인을 구축했습니다. 
와 PC를 USB로 연결하여 센서 데이터를 읽고 머신러닝 추론을 수행하는 Windows KMDF 기반 드라이버입니다.

작성할 내용 
- kernel debugging setting 하기

---

## 핵심 기술 스택(Core Tech Stack)

- Kernel Architecture: Windows Driver Foundation(KMDF) 기반 드라이버 설계 및 I/O 큐 관리
- Memory Management: **Direct I/O 및 MDL(Memory Descriptor List)**을 활용한 제로 카피 지향 데이터 전송
- Hardware Interface: USB Bulk/Interrupt 엔드포인트 통신
- Edge AI: TinyML(TensorFlow Lite for Microcontroller) 기반 FOMO 얼굴 감지 모델 이식
- Vision Processing: OpenCV 4.12.0 및 MFC를 활용한 호스트 어플리케이션 개발

---

## 시스템 아키텍처 및 설계 의도

1. MCU를 가속기로 활용하기 위한 전략
   일반적인 NPU와 달리 병렬 연산 최적화가 부족한 MCU의 구조적 차이를 인정하고 실제 NPU를 연동하기 전
   windows의 커널과 driver에 대한 이해를 위한 설계
2. 고속 데이터 전송: Direct I/O & MDL  
   MCU 하드웨어의 한계로 대용량 데이터를 처리할 수 없지만 CPU의 오버헤드를 최소화하기 위해 `METHOD_BUFFERED` 대신 `DIRECT I/O` 방식을 채택
   커널 내 불필요한 메모리 복사를 제거하여 호스트 PC의 자원 점유율을 최적화 

---

## 주요 기능

- **실시간 온도 읽기**: Pico의 온도 센서 데이터를 주기적으로 읽음
- **이미지 추론**: FOMO 기반 얼굴 감지 모델을 활용한 실시간 객체 인식
- **인터럽트 처리**: Pico의 인터럽트 신호를 안정적으로 처리
- **LED 제어**: 온/오프, 토글 등 기본 제어 지원
- **DMA 기반 데이터 전송**: 대용량 데이터 효율적 전송

---

## Kernel Debugging Setting (WinDbg)
드라이버의 안정성 확보 및 런타임 분석을 위해 아래와 같은 디버깅 환경을 구축하여 테스트를 진행
Oracle VirtualBox 사용 

자세한 세팅 아래 블로그 참조  
https://velog.io/@wang_ki/windbg-kernel-debugging-환경-세팅

---

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

---

## 주요 IOCTL 인터페이스

| IOCTL | 기능 | 입력 | 출력 |
|-------|------|------|------|
| `IOCTL_PICO_READ_TEMP` | 온도 센서 읽기 | - | 온도값 (float) |
| `IOCTL_PICO_RUN_INFERENCE` | 모델 추론 실행 | 이미지 데이터 | 추론 결과 |
| `IOCTL_PICO_READ_INTERRUPT` | 인터럽트 신호 읽기 | - | 인터럽트 상태 |
| `IOCTL_PICO_DMA_WRITE` | DMA로 데이터 전송 | 대용량 데이터 | 전송 바이트 |
| `IOCTL_PICO_MODEL_LOAD` | 모델 로드 | 모델 경로 | 성공/실패 |

---

## 빌드 및 설치

### 필수 사항
- Visual Studio 2019 이상
- Windows Driver Kit (WDK) 설치
- 모델 추론 테스트 시, opencv(4.12.0) 설치 필요 

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

---

## 유저모드 어플리케이션 테스트 (GUI mfc)

### GUI 테스트 앱 실행
vm 환경에 재배포 패키지가 설치되지 않았다면 MT 빌드 필요 

#### opencv 필수 dll
`opencv_videoio_ffmpeg4120_64.dll`, `opencv_world4120.dll` 

<img width="774" height="587" alt="image" src="https://github.com/user-attachments/assets/4c878541-092c-4c98-9280-e1570e74e6c0" />

### 테스트 영상
<img width="1152" height="620" alt="녹화_2026_04_14_23_14_58_215" src="https://github.com/user-attachments/assets/1a6e1a19-49db-447d-a2e0-aad14aa305d5" />

---

## serial 통신 디버깅 세팅
cdc interface로 mcu에서 발생하는 로그를 실시간으로 확인할 수 있다. 
```python
import serial
import time

# 포트 설정 (장치관리자에서 확인)
PORT = 'COM27'
BAUD = 115200

try:
    # timeout을 설정해야 장치가 응답 없을 때 무한 대기에 빠지지 않습니다.
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Connected to {PORT}")

    while True:
        if ser.in_waiting > 0:  # 읽을 데이터가 있을 때만 처리
            try:
                # decode('utf-8', 'ignore')를 써야 이상한 바이트가 섞여도 안 죽습니다.
                line = ser.readline().decode('utf-8', 'ignore').strip()
                if line:
                    print(f"[LOG] {line}")
            except Exception as e:
                print(f"Read error: {e}")
        
        time.sleep(0.01)  # CPU 점유율 과다 방지

except serial.SerialException as e:
    print(f"Could not open port {PORT}: {e}")
except KeyboardInterrupt:
    print("\nStop logging.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
```

포트는 장치 관리자에서 확인하면 된다. 
> win+x+m -> 장치 관리자
> 보기 -> 연결별 디바이스
> <img width="781" height="572" alt="image" src="https://github.com/user-attachments/assets/2a517297-958b-4020-bfc3-cd9da6231766" />

<img width="386" height="652" alt="image" src="https://github.com/user-attachments/assets/35f01f0c-cc0b-44fb-93a7-732fa08c6475" />

---

## 블로그 작성 
이슈 및 기록을 위한 기술 블로그 작성 
> https://velog.io/@wang_ki/series/windows-driver

---

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

---

## 다음 목표 
- SDK Layer 구축: `DeviceIoControl`을 추상화하여 사용자에게 `led_on()`, `run_inference()`등 직관적인 API를 제공하는 Wrapper DLL 개발
- Native NPU Driver 개발: 본 프로젝트에서 검증된 KMDF 아키텍처를 바탕으로 실제 AI 가속기 전용 드라이버로 확장 개발 예정(USB -> PCIe)

## 라이센스

프로젝트 라이센스 정보는 프로젝트 루트 디렉토리 확인

## 참고자료

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- [TensorFlow Lite](https://www.tensorflow.org/lite)
- [Edge Impulse](https://edgeimpulse.com/)
