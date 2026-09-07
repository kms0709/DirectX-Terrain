DirectX-Terrain

## 학습 과정

1.  기본 평면 그리드 ( Basic Flat Grid )

2. 펄린 노이즈 지형 ( Perlin Noise )

3. 높이맵 이미지 지형 ( Height Map )

4. 텍스처 스플래팅 정점 경사도 / 높이 기반 ( Texture Splatting )

5. 쿼드트리 컬링 ( QuadTree Culling )

6. 거리 기반 LOD 지형 1 ( Distance LOD 1 )

6-2. 고급 거리 LOD 지형 2 ( 스티칭 & 지오머핑 )

7. 하드웨어 테셀레이션 지형 ( Tessellation )

8. 스카이맵 ( SkyDome / SkyBox )

9. 동적 왜곡 구름 ( Perturbed Clouds )

10. 무한 지형 청크 ( Infinite Chunks )

## 목표 1. 공통 프레임워크 제작

DirectX 11 사용
ImGui 를 이용한 프레임워크 UI 제작
유니티 베이스 구조
- Scene
- Input
- Component
- GameObject

## 과정 1. 일단 모르겠으니 간단하게 프롬프트 작성

```
D3D11 을 이용한 Framework 를 제작해 줘.
- Visual Studio 2022 를 사용할 것.
- Direct3D 초기화와 Window 초기화를 나눌 것.
- Framework Editor UI 에 사용 할 버전에 맞는 Imgui 를 부착할 것.
- Unity 의 Component 패턴과 life cycle 을 참고하여 설계할 것.
- Framework 에 대한 설명과 가이드를 Markdown 형식으로 한국어로 만들 것.
  - 설명에 사용한 Prompt 도 있어야 함.
```

## 문제점 1. Scene 을 하나만 쓰는 구조
main -> Application -> Scene 으로 넘어감.
생각한 건 부모 Scene 을 상속받아서 여러 개의 Scene 을 생성할 수 있는 것.
현재 구조는 Scene 을 하나만 쓸 수 있고, Scene 에 내부 구조가 노출 됨.

## 프롬프트 수정안 1. Scene 구조 수정 및 새로운 기능 추가

```
현재 프로젝트를 다음과 같이 수정해 줘.
- Scene 은 여러 개 만들 수 있어야 한다.
  - Scene Class 를 상속 받아서 새로운 Scene 을 만든다.
  - GameObject 는 Scene 을 상속 받은 클래스 안에서 생성한다.
- GameObject 들은 Hierarchy 에 보여야 한다.
  - Hierarchy 에서 GameObject 를 선택하면 해당 GameObject 위치에 Gizmo 가 보여야 한다.
- Editor 에 현재 Scene 에 GameObject 를 추가하는 기능과 UI 가 있어야 한다.
  - GameObject 에 Component 를 추가할 수 있는 기능과 UI 가 있어야 한다. ( Inspector 기능 )
  - 모든 GameObject 는 필수적으로 Transform Component 가 있다.
- 3D Model 을 위한 Mesh Component 를 추가한다.
  - Model 은 Box, Sphere, Cylinder, Plane 가 있다.
  - Inspector 에서 Mesh Component Model 을 선택할 수 있다.
```

## 과정 2. 에디터에서 화면을 둘러볼 수 있는 카메라 추가 ( 컴포넌트 아님 )

```
Editor 에서 화면을 둘러볼 수 있는 카메라를 추가해 줘.
- GameObject 에 붙이는 Component 가 아니다.
- UI 창 밖에서 마우스 우클릭을 한 상태에서 다음과 같은 동작이 되야 한다.
  - W, A, S, D 로 이동
  - 드래그 시 시점 변환
- UI 창 밖에서 마우스 휠로 확대, 축소가 되어야 한다.
```

## 문제점 2. Gizmo 가 카메라 방향에 상관없이 고정되는 문제

GameObject 선택 시, Gizmo 가 GameObject 의 Z축을 기준으로 카메라의 방향에 맞춰 보여야 하는데
카메라의 시점이 바뀌어도 항상 일정한 문제.

## 프롬프트 수정안 2. Gizmo 축 수정 및 기능 추가

Camera 에 따른 Gizmo 를 올바르게 보이게 하고,
유니티와 비슷하게 기능을 추가했다.

```
GameObject 선택 시 나오는 Gizmo 를 다음과 같이 수정해 줘.
- GameObject 의 Z축을 기준으로 Camera 의 위치에 맞게 Gizmo 가 보여져야 한다.
- GameObject 를 선택하고 Gizmo 가 보이는 상태에서 W를 누르면 이동, E 를 누르면 회전, R 을 누르면 크기 Gizmo 가 보여야 한다.
- Gizmo 를 잡고 드래그 시 잡은 축을 기준으로 이동, 회전, 크기 변환이 되어야 한다.
```

## 문제점 2-1. Gizmo 선택 시 Gizmo 가 깜빡거리는 문제

공사 중

## 과정 3. Terrain Component 추가

공사 중