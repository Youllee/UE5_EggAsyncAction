# EggAsync

Blueprint async actions for Unreal Engine 5.8. The plugin provides cancelable delays, Enhanced Input event bindings, and easing actions for float, vector, color, and rotator values.

## Install

Copy this repository into `<YourProject>/Plugins/EggAsync/`, then regenerate your project files and build the project. The plugin requires Unreal Engine 5.8 and the Enhanced Input plugin.

The repository root is the plugin directory: `EggAsync.uplugin`, `Source/`, and `Resources/` belong together. Generated `Binaries/` and `Intermediate/` directories are intentionally excluded.

## Actions

- **Cancelable Delay (Async):** Waits for the given duration, then fires `Completed`. Call `Cancel` on the returned async action to fire `Canceled` instead. A duration of zero or less completes on the next tick. The optional pause setting keeps time advancing while the game is paused.
- **Bind Input Action (Async):** Forwards Enhanced Input's `Triggered`, `Started`, `Ongoing`, `Canceled`, and `Completed` events. Call `Cancel` to remove its bindings. `SetPause` suppresses forwarded events while keeping the bindings. If the player's input component is replaced, create a new binding for the new component.
- **Async Easing Action:** Interpolates float, Vector2D, vector, color, or rotator values. Supports multiple easing curves, looping, round trips, update rate limits, explicit completion, cancellation, and ticking while paused.

## License

MIT. See [LICENSE](LICENSE).
