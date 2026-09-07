# DirectX-Terrain

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