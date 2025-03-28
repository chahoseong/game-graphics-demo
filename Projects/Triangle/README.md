# Triangle Demo
![screenshot](https://github.com/chahoseong/game-graphics-demo/blob/main/Projects/Triangle/Screenshot.png)
이 데모는 Direct3D를 초기화 하고 Vertex Buffer 사용하여 삼각형을 보여주는 프로그램입니다.

## 학습 내용
- Window 생성 및 메시지 루프
- Direct3D 초기화
- 기본적인 그래픽스 파이프라인 실습
    - Vertex Shader
    - Pixel Shader
- C++20 Module

## 알게된 점
### Swap Chain을 생성 시 주의사항
- Swap Chain을 생성할 때는 ```SwapEffect```를 ```DXGI_SWAP_EFFECT_FLIP_DISCARD```로 설정합니다.
이 방식은 **최신의 Flip 모델 스왑 방식으로, 버퍼 내용을 복사하지 않고 포인터만 교체**하여 효율적으로 프레임을 전환합니다.
- **Flip 모델**을 사용하려면 **버퍼 개수(BufferCount)를 최소 2개 이상**으로 지정해야 합니다.
(BufferCount < 2일 경우 ResizeBuffers()에서 오류가 발생할 수 있습니다.)
이를 실행하기 위해서는 Swap Chain을 생성할 때 버퍼 개수를 반드시 2개 이상 설정해야 합니다.

### Refresh Rate 설정
- **창 모드(Windowed Mode)**
    - DXGI_RATIONAL 구조체의 값을 다음과 같이 설정하면,
    시스템이 자동으로 최적의 Refresh Rate를 사용합니다.
    ```cpp
    Numerator = 0;
    Denominator = 1;
    ```
- **전체화면 모드(Fullscreen Mode)**
    - 현재 해상도에서 사용 가능한 Refresh Rate 목록 중에서 **명시적으로 선택**해주어야 합니다.
    - 이를 위해 ```IDXGIOutput::GetDisplayModeList()``` 등을 사용해 지원 가능한 모드를 조회한 후 설정합니다.

### Render Target View Binding
- **Swap Chain의 ```SwapEffect```를 ```Flip``` 방식으로 사용할 경우**, ```Present()```가 호출되면 **백 버퍼가 매 프레임마다 변경**됩니다.
- ```RenderTargetView(RTV)```는 **특정 백 버퍼 리소스(ID3D11Texture2D)** 에 바인딩되어 있기 때문에,
```Present()``` 이후에는 **기존 RTV가 더 이상 유효한 백 버퍼를 가리키지 않을 수 있습니다**.
- 따라서 매 프레임마다 **현재 백 버퍼에 대한 RTV를 다시 설정해줘야** 합니다.
- ```GetBuffer(0)```을 호출하면 **현재 사용할 백 버퍼 하나만 가져올 수 있으므로**,
해당 버퍼에 대응하는 RTV를 **매번 생성하거나, 생성된 RTV를 캐싱하여 재사용**해야 합니다.
- 이 과정을 통해 **항상 올바른 백 버퍼에 렌더링이 이루어지도록 보장**할 수 있습니다.
