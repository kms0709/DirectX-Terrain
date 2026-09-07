# 프롬프트

이 문서는 ChatGPT를 이용하여 작성했습니다.
프롬프트 사용 및 작업 내역은 [여기](Prompt.md)를 참고하세요.

# D3D11 Component Framework

Visual Studio 2022와 C++20을 기준으로 만든 소형 Direct3D 11 프레임워크입니다. Win32 윈도우, Direct3D 초기화, Dear ImGui 편집기, Unity에서 착안한 Scene 및 Component 생명주기를 서로 분리했습니다.

## 빠른 시작

필요 환경은 Visual Studio 2022의 **Desktop development with C++** 워크로드와 CMake 3.24 이상입니다.

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-debug
```

생성된 `Build/D3D11Framework.sln`을 Visual Studio 2022로 열어 `D3D11Framework`를 시작 프로젝트로 실행할 수도 있습니다. 최초 구성 시 CMake가 공식 Dear ImGui `v1.92.9` 소스를 내려받으므로 인터넷 연결이 필요합니다.

상세한 구조 및 확장 방법은 [한국어 가이드](Docs/FrameworkGuide.ko.md)를 참고하십시오.

## 주요 구성

- `Window`: Win32 클래스 등록, 창 생성, 메시지 펌프, 크기 변경 이벤트
- `D3D11Renderer`: D3D11 Device/Context, DXGI SwapChain, Color/Depth Target
- `SceneRenderer`: 기본 카메라, 셰이더와 Primitive Mesh 렌더 패스
- `EditorLayer`: Scene 전환, Hierarchy, 선택 Gizmo, Inspector UI
- `SceneManager`, `Scene`, `GameObject`, `Component`: 다중 Scene과 Unity식 생명주기
- `TransformComponent`: 모든 GameObject가 자동으로 갖는 필수 Component
- `MeshComponent`: Box, Sphere, Cylinder, Plane 모델과 색상
- `RotatorComponent`: 사용자 Component 작성 예시

Dear ImGui는 MIT License이며 CMake 구성 시 공식 저장소에서 받아 정적으로 링크합니다.
