# Frok Rendering Engine

1인 개발로 진행한 DirectX 12를 이용한 렌더링 엔진입니다.

## 개발 목적

- 컴퓨터 그래픽스 복습 겸 Windows에서 제공하는 라이브러리를 공부해보는 것이 목표입니다.
- 게임 엔진의 기반이 되는 API인 만큼 공부를 하고 넘어가는 편이 좋다는 취지 하에 학습 목표로 공부하기 시작했습니다.

## 특징

- 렌더링 엔진 구조는 게임 엔진인 Unity를 묘사하는 방향으로 개발을 진행했습니다.
- 로직을 총괄하는 Core 부분을 정적 라이브러리화 하여 개발 편리성을 높혔습니다.
- 정적 라이브러리를 사용하는 Client 구조로 나뉘어져 있습니다.
- 디버깅을 위한 멀티 렌더링 타겟을 지원합니다.

- [적용 영상](https://youtu.be/-y9h2oLrzcM)

## 사용 기술

Frok Rendering Engine를 개발하기 위해 다음과 같은 기술을 사용했습니다.

- C++ - DirectX 12가 지원하는 언어가 C++이기 때문에 사용했습니다.
- DirectX 12 - Windows에서 제공하는 멀티미디어 API입니다.
- FBX - 3D FBX 애니메이션을 실행하기 위해서 사용했습니다.

## 프로젝트 설명
### Engine

#### 엔진
- 엔진은 게임의 로직 부분을 총괄하는 클래스입니다.
- DirectX 12의 디바이스와 커맨드 큐, 스왑체인, 루트 서명, 그리고 서명자 힙들을 초기화 합니다.
- 각종 매니저들을 초기화를 진행하며, 이 초기화된 요소들을 업데이트 하는 방식으로 진행됩니다.
- 업데이트 방식은 유니티의 Update -> FixedUpdate(물리 효과에 주로 사용) -> LateUpdate(카메라 등에서 사용)의 구조를 비슷하게 가져왔습니다.
-  씬 매니저를 통해서 내용을 렌더링하는 구조로 이루어져 있습니다.

##### MRT(Multi-Rendering Target)
- 디버깅을 위한 멀티 렌더링 타겟을 지원합니다.
- Forward-Deffered 렌더링 방식을 차용해서 개발을 진행했으며, 셰이더의 계산 진행에 따른 결과물을 텍스처에 저장을 해서, 이를 UI를 통해서 읽어오는 방식으로 개발을 진행했습니다.
- 크게 Depth-Stencil / Swap-Chain / Shadow / Deffered / Lighting 으로 나눠서 이를 출력하는 방식으로 구현을 진행했습니다.

#### 매니저
매니저는 크게, 오디오, 충돌, 게임 매니저, ImGui, 입력, 인스턴싱, 리소스, 씬, 타이머가 있습니다.

##### 오디오
- 오디오 매니저는 FMOD를 통해서 오디오를 실행하는 방식으로 구현했습니다.
- 오디오들은 로딩을 할 때 불러오는 방식이며, 키 값을 이용해서 재생을 하고 멈추거나, 볼륨을 조절할 수 있는 방식입니다.

##### 충돌
- 충돌 매니저는 충돌 컴포넌트를 가진 게임 오브젝트를 리스트로 저장해서 순회를 하며, 각종 게임 오브젝트의 충돌을 검사합니다.
- 그 외 게임 오브젝트를 클릭했을 경우, Picking이 되었는지 검사하는 기능 또한 충돌 매니저가 담당합니다.

##### 게임 매니저
- 게임의 룰을 저장하고, 게임에서 필요한 필수 기능 등을 저장하기 위한 매니저입니다.
- UI를 On/Off 등의 기능은 이 매니저가 담당하게 제작을 했습니다.

##### ImGui
- 에디터를 위한 UI 매니저입니다.
- 유명 UI 오픈소스인 ImGUI를 사용해서 개발했습니다.

##### 입력
- 입력 매니저는 게임의 입력을 담당하는 클래스입니다.
- 입력은 enum class를 이용해서 미리 약속된 입력들을 키 값으로 저장하며, 이 입력은 게임 매니저나, MonoBehavior 클래스를 이용해서 기능을 구현할 수 있게 합니다.

##### 인스턴싱
- 인스턴싱 매니저는 게임의 최적화를 위해서 만들어진 물건입니다.
- 게임 오브젝트도 같은 것을 만들더라도, 만약 머터리얼이나 그 외 등등의 요소가 같은 경우 이런 것을 매번 로딩하는 것은 리소스나 메모리 낭비가 됩니다.
- 이런 현상을 막기 위해 비슷한 것들은 인스턴싱을 진행해서, ID값을 이용하면 버퍼에 저장된 리소스를 다시 사용할 수 있게 만들어 놓았는데, 이를 관리하기 위한 매니저입니다.

##### 리소스
- 리소스 매니저는 게임에서 사용되는, 메시 정보들을 가지고 있고, 또는 셰이더나, 머터리얼, 그리고 FBX 파일 등등을 로딩해서 이를 관리하는 매니저입니다.
- 이 매니저를 이용해서 키 값을 통해 기존에 미리 불러온 셰이더나 머터리얼 등등을 가져와서 사용할 수 있게 되어 있습니다.

##### 씬
- 씬은 게임 오브젝트를 게임 오브젝트, 카메라, 조명 등등을 관리하기 위한 클래스입니다.
- 이 클래스를 통해서 현재 불러온 레벨의 게임 오브젝트를 관리할 수 있습니다. 그 외 게임 컨텍스트의 갱신을 담당합니다.
- 씬 매니저는 이러한 씬의 흐름을 관리하기 위해 제작된 클래스입니다. 또한 씬 매니저를 통해서 각종 UI 등등을 관리하기 위해 제작된 레이어 등을 관리합니다.

##### 타이머
- 게임의 프레임이나 시간 등을 관리하는 클래스입니다.

#### 오브젝트
- 게임에 사용되는 근간인 클래스, 게임 오브젝트를 관리하기 위한 클래스입니다. 게임 오브젝트는 컴포넌트를 장착해서, 애니메이션, 충돌, 메시, 등등을 관리할 수 있으며, 구체화된 표현은 MonoBehavior를 장착해서 구현할 수 있습니다.

#### 셰이더
- 개발에 필요한 셰이더 등을 보관하는 필터입니다.

#### 유틸
- 개발에 필요한 타입이나, 함수 등을 보관합니다. 그 외 외부 라이브러리 등을 넣습니다.

## 사용중인 외부 라이브러리
ImGui, FMOD, FBX SDK
