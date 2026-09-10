# Plugin Levels and Practical Game Scope

Plugin level is a capability/trust boundary, not a quality rating.

## Level 1 — Legacy/basic RCC

QML/JavaScript resources using the legacy package contract. Best for small self-contained 2D games: Tic-Tac-Toe, Minesweeper, Blackjack, Sudoku, simple platformers, Snake, Breakout, basic card/board games and small arcade games.

Possible but increasingly awkward: large persistent worlds, complex AI, large procedural simulation, heavy pathfinding, advanced networking, custom renderers, or large data pipelines. Level 1 does not get arbitrary native code.

## Level 2 — Modern RCC

Still sandboxed RCC/QML/JavaScript, but can use LeoMiniGames services: GameSave, GameSettings, lifecycle, input actions, viewport, audio, haptics, i18n, stats, achievements, diagnostics, `GameTheme`, deterministic `GameRandom`, `GameClock`, and `GameEvents`.

**Candy Crush-style match-3:** fully appropriate for Level 2. Board simulation, animation, boosters, level data, achievements, saves, audio and touch input fit the API well.

**Stardew Valley-style 2D farming/RPG:** technically possible in Level 2 if designed around Qt Quick and data-driven systems. A moderate tile world, farming, inventory, NPC schedules, quests, dialogue, day/night state and saves are feasible. For a very large world, heavy NPC simulation, sophisticated pathfinding, mod-inside-mod systems or performance-critical procedural logic, Level 2 becomes restrictive and requires careful chunking/caching.

Other good Level 2 targets: tower defense, deck builders, roguelites, turn-based RPGs, visual novels, rhythm games, management games, factory/idle games, 2D tactics, puzzle campaigns and lightweight multiplayer clients when an approved networking API is available.

## Level 3 — Trusted Native

Reviewed native C++/Qt extension loaded only through YoungLion/authorized verified-publisher trust metadata. Manifest text cannot grant this level.

A Stardew-scale game is much more comfortable here: native simulation/pathfinding, custom models, optimized map streaming, native libraries approved by policy and more specialized rendering are possible. Advanced emulation, custom codecs, physics libraries or deep platform integration can also be feasible when reviewed.

Level 3 still does not mean “anything goes.” The host may deny dangerous filesystem/process/system APIs, unsupported ABI versions or unreviewed dependencies. It is also not intended to turn LeoMiniGames into a full AAA 3D engine; highly complex 3D titles are better served by a dedicated engine unless a future rendering API explicitly supports them.

## Summary

| Game type | L1 | L2 | L3 |
|---|---|---|---|
| Tic-Tac-Toe / Sudoku / Minesweeper | Excellent | Excellent | Unnecessary |
| Candy Crush / match-3 | Possible | Excellent | Unnecessary |
| Platformer / Breakout / Snake | Good | Excellent | Optional |
| Deck builder / roguelite / VN | Possible | Excellent | Optional |
| Tower defense / 2D tactics | Limited-Good | Excellent | Excellent for heavy simulation |
| Stardew-like farming RPG | Limited | Feasible | Excellent |
| Large simulation / heavy AI | Poor | Limited-Feasible | Best fit |
| Custom native renderer/library | No | No | Yes, reviewed |
| Arbitrary native OS/process access | No | No | Not automatically; policy-controlled |
