# Viewport and responsive layout

Properties: safeWidth, safeHeight, safeTop, safeBottom, density, portrait, landscape, valid; changed notifies geometry. The host updates the viewport for the game area. safeHeight subtracts top/bottom insets; safeWidth is clamped width (there are no exposed left/right inset properties in 0.7). Values start positive to keep division/radius calculations valid, but valid is not proof that layout has settled.

Use the root Item's actual width/height for local anchors/layout and Viewport for host viewport hints. The root is already hosted in an inset-aware game container; do not blindly subtract safeTop/safeBottom twice. Native safe-area margins are available with Qt >=6.9; the Qt 6.8 fallback is zero, so mobile device QA remains necessary. density is a host device-pixel-ratio hint, not a blanket multiplier for every Qt Quick coordinate.

Bound sizes with Math.max/Math.min and use Flickable/ScrollView when content cannot fit. Test portrait, 640×360 landscape, long translations, keyboard focus and large font settings. Host device profiles simulate geometry; they do not prove physical touch targets, IME/notch behavior or audio routing. Viewport.update/windowInsets are owned by the host, not game layout callbacks.
