# Box Demo
![screenshot](https://github.com/chahoseong/game-graphics-demo/blob/main/Projects/Box/Screenshot.png)
이 데모는 박스에 **이동(Translation)**, **회전(Rotation)**, **비례(Scaling)**을 적용하고 
**카메라 시점(View)** 및 **투영(Projection)**을 통해 물체를 그리는 프로그램입니다.

## 사용 기술
- C++20
- Direct3D 11
    - Constant Buffer
    - Rasterizer State
- ImGui

## 학습 내용
- World/View/Projection 행렬의 역할과 좌표계 변환 흐름
- Constant Buffer를 사용하여 GPU로 데이터를 전달하는 방법
- Rasterizer State를 활용하여 Backface Culling, Wireframe 적용

## 알게된 점
### D3D11_USAGE 특징

|Usage  |CPU  |GPU       | 용도                        |
|-------|-----|----------|-----------------------------|
|Dynamic|Write|Read      |CPU에서 자주 데이터를 갱신할 때|
|Default|     |Read/Write|일반적인 경우                 |

- https://learn.microsoft.com/ko-kr/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

### ImGui Win32 메시지 루프
```cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

    ...
}
```
ImGui를 Win32 환경에서 사용할 경우, `ImGui_ImplWin32_WndProcHandler()` 함수를 선언하고
윈도우 메시지 프로시저를 처리하는 함수에 윈도우 메시지를 보내주어야 합니다.