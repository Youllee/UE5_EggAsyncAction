# EggAsync

언리얼 엔진 5.8용 블루프린트 비동기 액션 플러그인입니다. 취소 가능한 딜레이, Enhanced Input 이벤트 바인딩, 여러 값 형식의 Easing 액션을 제공합니다.

## 설치

이 저장소의 파일을 프로젝트의 `<프로젝트>/Plugins/EggAsync/`에 복사한 뒤 프로젝트 파일을 다시 생성하고 빌드하세요. 언리얼 엔진 5.8과 Enhanced Input 플러그인이 필요합니다.

저장소 루트가 플러그인 폴더입니다. `EggAsync.uplugin`, `Source/`, `Resources/`를 함께 복사하면 됩니다. `Docs/`에는 노드 이미지와 동작 예시가 있습니다. `Binaries/`와 `Intermediate/`는 빌드 과정에서 생성되므로 저장소에 포함하지 않습니다.

## 제공 액션

### 취소 가능한 딜레이

`Cancelable Delay (Async)`는 지정한 시간이 지나면 `Completed`를 호출합니다. 반환된 `Async Action`에서 `Cancel`을 호출하면 대기를 끝내고 `Canceled`를 호출합니다. 대기 시간이 0 이하이면 다음 Tick에 완료합니다. 일시정지 중에도 시간을 진행하는 옵션을 제공합니다.

### 입력 액션 바인딩

`Bind Input Action (Async)`는 Enhanced Input의 `Triggered`, `Started`, `Ongoing`, `Canceled`, `Completed` 이벤트와 입력 값을 전달합니다. 반환된 액션을 취소하면 바인딩이 해제됩니다. `SetPause`는 바인딩을 유지하면서 이벤트 전달만 일시 중지합니다. 플레이어의 입력 컴포넌트가 교체되면 새 컴포넌트에 다시 바인딩해야 합니다.

![Cancelable Delay와 Bind Input Action 블루프린트 노드](Docs/AsyncAction.png)

### Easing 액션

`Async Easing Action`은 `Float`, `Vector2D`, `Vector`, `Color`, `Rotator` 값을 보간합니다. Easing 곡선, 반복, 왕복, 초당 업데이트 횟수 제한, 명시적 완료와 취소, 일시정지 중 Tick을 지원합니다.

![Float, Vector2D, Vector, Rotator, Color Easing 블루프린트 노드](Docs/AsyncEasingAction.png)

아래는 Easing 액션의 동작 예시입니다.

![Easing 액션 동작 예시](Docs/EasingActionExample.gif)

## 라이선스

MIT 라이선스입니다. 자세한 내용은 [LICENSE](LICENSE)를 참고하세요.
