# Lifecycle API

Legacy lifecycle methods `load`, `start`, `pause`, `resume`, `background`, `foreground`, `save`, `close`, and `unload`, plus their signals, remain exposed through `Lifecycle`.

The host owns attachment of the external root object; external games do not receive the old raw `attach()` mutator because it could rebind the host lifecycle to arbitrary objects. Existing games normally did not need to call it themselves because `GameHost` attached the loaded root.

On application inactivity the host emits pause/background/save and commits `GameSave.forceSave()`. On return it emits foreground/resume. Closing a game also releases external engine/resources/audio state.
