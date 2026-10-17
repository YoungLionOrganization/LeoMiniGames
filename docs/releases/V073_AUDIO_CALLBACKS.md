# v0.7.3 audio callback lifetime fixes

17 October 2026. Follows [runtime fixes](V073_RUNTIME_FOLLOWUPS.md) and [Linux installer recovery](V073_INSTALLER_RECOVERY.md).

A synchronous audio resource error could invoke a game's error handler while `AudioClip::play()` was still running. Stopping or replacing music in that handler deleted the clip, and the old call then accessed its freed state. A native regression reproduced a segmentation fault when the music error handler called `stopMusic()`.

Playback now checks the clip's lifetime after calls that may publish errors, including media-source setup. Music retirement clears the active pointer before destroying the old clip, so callbacks can install a replacement. Request generations prevent an interrupted outer music request from overwriting a newer nested request and leaving a second player behind.

Regression coverage includes stopping failed music from its error callback, replacing failed music, starting a replacement during retirement, and nested replacement while another playback request is starting. The nested-request test first failed at `newest nested music request wins without orphaned playback`; the corrected test passed. Existing real decoder, background pause/resume, volume composition and asynchronous temporary-file cleanup checks remain enabled.

Verification uses the private Qt 6.10.2 Windows x86_64 Debug kit with Multimedia. The full native suite passed 22/22 after these fixes. Fresh GitHub results are tracked in PR #7. These checks do not substitute for physical audio output/device release QA.
