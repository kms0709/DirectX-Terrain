# D3D11 Component Framework 설명 및 가이드

## 1. 목표와 범위

이 프로젝트는 게임 또는 툴 제작의 출발점으로 사용할 수 있는 작은 Windows 전용 프레임워크입니다. 여러 Scene, Primitive 3D 모델 렌더링과 Dear ImGui 기반 편집기를 제공하며, 외부 에셋 로더·물리·직렬화는 의도적으로 비워 두었습니다.

기준 환경은 다음과 같습니다.

- Visual Studio 2022, MSVC, Windows 10/11 SDK
- C++20
- Direct3D 11 및 DXGI
- Dear ImGui `v1.92.9`와 공식 Win32/DX11 백엔드
- ImGuizmo `1.10`
- CMake 3.24 이상

## 2. 설계 구조

```text
Application
├─ Window             Win32 등록/생성/메시지 처리
├─ D3D11Renderer      Device/Context/SwapChain/RenderTarget
├─ SceneRenderer      Camera/Shader/Primitive Mesh 렌더링
├─ EditorLayer        Dear ImGui 프레임과 편집기 위젯
│  └─ EditorCamera    Component가 아닌 편집 전용 카메라
└─ SceneManager
   └─ Scene[]          Scene 상속 클래스로 콘텐츠 정의
      └─ GameObject[]
         └─ Component[]  Unity식 기능 조립과 생명주기
```

`Window::Initialize()`와 `D3D11Renderer::Initialize()`는 완전히 분리되어 있습니다. `Application`만 양쪽을 알고 있으며, 생성된 `HWND`와 클라이언트 크기를 렌더러에 전달합니다. 창 크기 변경도 콜백으로 연결하여 Window가 Direct3D 타입에 의존하지 않습니다.

## 3. 프레임 흐름

한 프레임은 아래 순서로 실행됩니다.

```text
Win32 메시지 처리
→ 현재 Scene::Update
→ ImGui 새 프레임 및 EditorCamera 입력 처리
→ D3D11 RenderTarget 지우기
→ 현재 Scene의 Mesh 렌더링
→ Editor UI 구성
→ ImGui DrawData 렌더링
→ SwapChain Present
→ 현재 Scene::LateUpdate
```

VSync는 Framework Editor에서 켜고 끌 수 있습니다. 창이 최소화되면 0 크기 Resize를 무시하고, 다시 표시되었을 때 SwapChain 버퍼와 RenderTarget을 재생성합니다.

## 4. Component 패턴과 생명주기

`GameObject`는 여러 `Component`를 소유합니다. Component는 자신의 GameObject를 `GetGameObject()`로 참조하고, `GetComponent<T>()`로 형제 Component를 찾을 수 있습니다. 모든 GameObject는 생성자에서 `TransformComponent`를 자동 생성하므로 Transform을 제거하거나 중복 추가할 수 없습니다.

호출 규칙은 Unity의 개념을 단순화한 형태입니다.

1. `AddComponent<T>()` 직후 `Awake()`를 한 번 호출합니다.
2. GameObject와 Component가 활성 상태이면 `OnEnable()`을 호출합니다.
3. 첫 Update 직전에 `Start()`를 한 번 호출합니다.
4. 매 프레임 `Update(deltaTime)`을 호출합니다.
5. 해당 프레임의 일반 Update가 모두 끝난 뒤 `LateUpdate(deltaTime)`을 호출합니다.
6. 비활성화할 때 `OnDisable()`을 호출합니다.
7. 파괴할 때 활성 Component에는 `OnDisable()`을 먼저, 이후 `OnDestroy()`를 한 번 호출합니다.

Scene을 바꾸면 이전 Scene의 활성 Component에는 `OnDisable()`이, 새 Scene의 활성 Component에는 `OnEnable()`이 호출됩니다. Update와 렌더링은 SceneManager가 가리키는 현재 Scene에만 수행됩니다.

현재 버전에는 Unity의 Transform 부모/자식 계층, Prefab, 런타임 지연 삭제, Script 실행 순서 설정이 없습니다. Scene Update 도중 컨테이너를 직접 변경하는 기능도 아직 제공하지 않습니다.

## 5. Scene 추가와 전환

게임용 Scene은 `Scene`을 상속하고 `OnCreate()` 안에서 GameObject를 생성합니다. `CreateGameObject()`는 파생 Scene에서만 접근할 수 있습니다.

```cpp
class GameScene final : public Framework::Scene
{
public:
    GameScene() : Scene("Game Scene") {}

protected:
    void OnCreate() override
    {
        auto& player = CreateGameObject("Player");
        player.GetTransform().position = { 0.0f, 1.0f, 0.0f };
        player.AddComponent<Framework::MeshComponent>(Framework::PrimitiveModel::Cylinder);
    }
};
```

Application 초기화에서 등록합니다.

```cpp
m_sceneManager.AddScene<GameScene>();
```

Framework Editor의 Hierarchy 상단 Scene 콤보에서 현재 Scene을 바꿀 수 있습니다. Editor의 `+ Add GameObject`는 현재 Scene에 빈 GameObject를 추가하는 편집 전용 진입점이며, 이 경우에도 Transform은 자동 생성됩니다.

## 6. 새 Component 작성

`Component`를 상속하고 필요한 콜백만 재정의합니다. 콜백은 프레임워크만 호출할 수 있도록 `protected`에 두는 것을 권장합니다.

```cpp
class MoverComponent final : public Framework::Component
{
protected:
    void Start() override
    {
        m_transform = GetGameObject().GetComponent<Framework::TransformComponent>();
    }

    void Update(float deltaTime) override
    {
        if (m_transform)
            m_transform->position.x += 2.0f * deltaTime;
    }

private:
    Framework::TransformComponent* m_transform = nullptr;
};
```

GameObject에는 다음처럼 붙입니다.

```cpp
auto& player = scene.CreateEditorGameObject("Player");
player.AddComponent<MoverComponent>();
```

Component의 소유권은 GameObject에 있으며 `std::unique_ptr`로 관리됩니다. `GetComponent<T>()`의 반환 포인터를 장기간 보관할 경우 대상 GameObject의 수명보다 길어지지 않도록 주의해야 합니다.

## 7. Mesh Component와 Primitive 모델

Inspector의 **Add Component** 버튼으로 Mesh를 추가할 수 있습니다. Mesh 패널의 Model 콤보는 다음 네 타입을 제공합니다.

- `Box`: 육면체
- `Sphere`: 위도/경도 분할 구
- `Cylinder`: 측면과 상·하단을 포함한 원기둥
- `Plane`: XZ 평면

모델과 색상을 Inspector에서 즉시 바꿀 수 있습니다. Primitive 정점/인덱스 버퍼는 `SceneRenderer` 초기화 때 한 번 생성되고 GameObject의 Transform으로 World 행렬을 구성합니다.

## 8. 렌더링 기능 확장

`D3D11Renderer`는 Device, Immediate Context, SwapChain, Color/Depth Target 생성과 Present를 담당하고, `SceneRenderer`는 기본 셰이더와 Primitive 렌더 패스를 담당합니다. 지형 또는 외부 모델 렌더링을 추가하려면 다음 순서가 적절합니다.

1. `Graphics` 아래에 Shader, Buffer, Texture 래퍼를 추가합니다.
2. HLSL 셰이더를 프로젝트에 추가하고 빌드 시 `fxc` 또는 `dxc`로 컴파일합니다.
3. 카메라·메시·렌더러 역할을 각각 Component로 작성합니다.
4. Scene Update 이후, ImGui 렌더링 이전에 게임 렌더 패스를 호출합니다.
5. 렌더 패스를 SceneRenderer에서 더 작은 전용 Renderer 클래스로 분리합니다.

Device나 Context를 전역 변수로 만들기보다 생성자 또는 렌더 컨텍스트 인자로 전달하면 테스트와 수명 관리가 쉬워집니다.

## 9. Editor UI 사용과 확장

`EditorLayer::Draw()`가 편집기 패널의 시작점입니다. 현재 제공 기능은 다음과 같습니다.

- Framework Editor: 프레임 시간, 배경색, VSync, ImGui Demo
- Hierarchy: Scene 전환, 현재 Scene에 GameObject 추가, GameObject 선택
- Inspector: 이름/활성 상태, Transform 편집, Mesh/Rotator Component 추가와 설정
- Scene Overlay: 선택된 GameObject의 로컬 축에 맞는 이동·회전·크기 Gizmo

새 Component 타입을 Inspector에 노출하려면 `DrawInspector()`의 Add Component 팝업과 해당 Component 패널을 추가합니다. 규모가 커지면 Component별 Inspector drawer 등록 테이블로 분리하는 것이 좋습니다.

### EditorCamera 조작

`EditorCamera`는 GameObject에 추가하는 Component가 아니며 `EditorLayer`가 직접 소유합니다. ImGui가 입력을 사용하는 UI 창 위에서는 동작하지 않고, UI 창 밖의 장면 영역에서만 다음 입력을 처리합니다.

- 마우스 오른쪽 버튼을 누른 상태의 `W`, `A`, `S`, `D`: 카메라 기준 전후좌우 이동
- 마우스 오른쪽 버튼을 누른 상태의 드래그: Yaw/Pitch 시점 회전
- 마우스 휠: 현재 바라보는 방향으로 전진/후진하여 확대·축소
- `Left Shift`: 이동 속도를 3배로 증가

우클릭 탐색을 시작하면 Win32 mouse capture를 설정하므로 버튼을 창 바깥에서 놓아도 정상 종료됩니다. Pitch는 카메라가 뒤집히지 않도록 제한되며, 큰 프레임 지연 시 이동량이 급증하지 않게 delta time을 제한합니다. Framework Editor의 `Reset` 버튼으로 초기 위치와 시점을 복원할 수 있습니다.

### Transform Gizmo 조작

Hierarchy에서 GameObject를 선택하면 ImGuizmo가 EditorCamera의 View/Projection 행렬과 GameObject의 World 행렬을 이용해 Gizmo를 그립니다. 따라서 카메라 위치가 바뀌어도 원근과 축 방향이 올바르게 유지되며, GameObject가 회전했다면 Z축을 포함한 Gizmo 축도 해당 로컬 방향을 따릅니다.

- `W`: 이동(`TRANSLATE`) 모드
- `E`: 회전(`ROTATE`) 모드
- `R`: 크기(`SCALE`) 모드
- 축 핸들 드래그: 한 축만 변환
- 두 축 사이의 평면 핸들 드래그: 해당 평면 안에서 변환

모든 모드는 `LOCAL` 좌표계를 사용합니다. 드래그 결과로 반환된 행렬은 Position, Rotation(도 단위), Scale로 분해하여 `TransformComponent`에 즉시 반영합니다. Inspector 값과 실제 Mesh 렌더링도 같은 프레임에 갱신됩니다. 우클릭 카메라 탐색 중에는 `W/E/R` 모드 전환을 무시하므로 `W`를 카메라 이동에 안전하게 사용할 수 있습니다.

ImGui 버전은 `CMakeLists.txt`의 `GIT_TAG`에 고정되어 있습니다. 버전을 올릴 때는 공식 릴리스 태그로 변경한 뒤 Win32/DX11 백엔드도 같은 태그의 파일을 함께 빌드하고, Debug/Release 구성을 모두 확인하십시오.

## 10. Visual Studio 2022 빌드

Developer PowerShell 또는 일반 PowerShell에서 저장소 루트를 기준으로 실행합니다.

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-debug
```

Release 빌드는 다음과 같습니다.

```powershell
cmake --build --preset vs2022-release
```

또는 Visual Studio 2022에서 폴더 자체를 열면 `CMakePresets.json`을 인식합니다. 전통적인 Solution 작업 방식은 첫 구성 후 생성된 `Build/D3D11Framework.sln`을 열면 됩니다.

### 자주 생기는 문제

- CMake가 VS 2022를 찾지 못함: Visual Studio Installer에서 **Desktop development with C++**, MSVC v143, Windows SDK를 설치합니다.
- Dear ImGui 다운로드 실패: 프록시/방화벽과 Git 설치 여부를 확인한 뒤 CMake 구성을 다시 실행합니다.
- Debug Device 생성 실패: 코드가 Graphics Tools 미설치 오류를 감지하고 Debug Layer 없이 한 번 더 생성합니다. D3D 디버그 메시지가 필요하면 Windows 선택적 기능에서 Graphics Tools를 설치합니다.
- 화면 찢어짐 또는 입력 지연: Editor에서 VSync 옵션을 조정합니다.

## 11. 설계 및 생성에 사용한 프롬프트

아래 프롬프트를 요구사항으로 사용해 이 프로젝트를 설계하고 생성했습니다.

```text
D3D11을 이용한 간단한 Framework를 제작해 줘.

- Visual Studio 2022를 사용할 것.
- Direct3D 초기화와 Window 초기화를 나눌 것.
- Framework Editor UI에 사용할 버전에 맞는 ImGui를 부착할 것.
- Unity의 Component 패턴과 life cycle을 참고하여 설계할 것.
- Framework에 대한 설명과 가이드를 Markdown 형식으로 한국어로 만들 것.
  - 설명에 사용한 Prompt도 있어야 함.
```

구체화 과정에서는 다음 원칙을 추가로 적용했습니다.

```text
Win32 창과 D3D11 렌더러가 서로 직접 의존하지 않게 하고 Application에서 조립한다.
Dear ImGui 공식 Win32 및 DX11 백엔드를 같은 릴리스 버전으로 고정한다.
Component 수명은 GameObject가 소유하고 Awake, Start, Update, LateUpdate,
OnEnable, OnDisable, OnDestroy를 중복 없이 호출한다.
샘플 Component와 Inspector 성격의 편집 UI를 제공한다.
RAII와 ComPtr를 사용해 종료 시 리소스가 안전하게 정리되게 한다.
```

기능 확장에는 다음 후속 프롬프트를 사용했습니다.

```text
현재 프로젝트를 다음과 같이 수정해 줘.

- Scene은 여러 개 만들 수 있어야 한다.
  - Scene Class를 상속 받아서 새로운 Scene을 만든다.
  - GameObject는 Scene을 상속 받은 클래스 안에서 생성한다.
- GameObject들은 Hierarchy에 보여야 한다.
  - Hierarchy에서 GameObject를 선택하면 해당 GameObject 위치에 Gizmo가 보여야 한다.
- Editor에 현재 Scene에 GameObject를 추가하는 기능과 UI가 있어야 한다.
  - GameObject에 Component를 추가할 수 있는 기능과 UI가 있어야 한다. (Inspector 기능)
  - 모든 GameObject는 필수적으로 Transform Component가 있다.
- 3D Model을 위한 Mesh Component를 추가한다.
  - Model은 Box, Sphere, Cylinder, Plane가 있다.
  - Inspector에서 Mesh Component Model을 선택할 수 있다.
```

Editor 카메라 추가에는 다음 프롬프트를 사용했습니다.

```text
Editor에서 화면을 둘러볼 수 있는 카메라를 추가해 줘.

- GameObject에 붙이는 Component가 아니다.
- UI 창 밖에서 마우스 우클릭을 한 상태에서 다음과 같은 동작이 되어야 한다.
  - W, A, S, D로 이동
  - 드래그 시 시점 변환
- UI 창 밖에서 마우스 휠로 확대, 축소가 되어야 한다.
```

Transform Gizmo 확장에는 다음 프롬프트를 사용했습니다.

```text
GameObject 선택 시 나오는 Gizmo를 다음과 같이 수정해 줘.

- GameObject의 Z축을 기준으로 Camera의 위치에 맞게 Gizmo가 보여져야 한다.
- GameObject를 선택하고 Gizmo가 보이는 상태에서 W를 누르면 이동,
  E를 누르면 회전, R을 누르면 크기 Gizmo가 보여야 한다.
- Gizmo를 잡고 드래그 시 잡은 축을 기준으로 이동, 회전, 크기 변환이 되어야 한다.
```
