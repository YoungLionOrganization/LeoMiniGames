# v0.7.3 — altı builtin ve Android TLS ek düzeltmesi

1 Ekim 2026. Kullanıcının yeni talebi, önceki planda 0.8'e ertelenen builtin board/hand resume işini v0.7.3 kapsamına aldı. Bu değişiklik önceki `28f22ec` uygulamasının üzerine gelir. Oyun/mod API'si 0.7 ve native plugin ABI'si 1.0 korunur; altı builtin'in içerik sürümü 1.1.0 olur.

## Altı oyunun yeni sisteme bağlanması

`BuiltinGame` native adapter'ı, `GameHost` tarafından doğrudan `GameLifecycle`'a bağlanır. `load → start → pause/resume → save → close → unload` çağrıları QML sayfası yerine gerçek native modele gider. `GameSave` altında `builtin_state` sürüm 1 envelope'u kullanılır; LMGSAVE formatı değişmez. Player action sinyalleri 150 ms içinde tek atomic checkpoint'e birleştirilir; background/close/quit mevcut zorunlu save yolunu kullanır. Bozuk veya gelecekteki envelope/snapshot yazmaya karşı korunur; v0.7.2 boş autosave'leri yeni oturuma dönüşür ve `local_stats.cbor` içindeki eski rekorlar korunur.

| Oyun | Geri yüklenen durum | Ek düzeltme |
| --- | --- | --- |
| XOX | Tahta ve doğru oyuncu sırası; bitmiş tur durumu tahtadan türetilir. | Pause sırasında hamle/reset engellenir; turn sayıları ve çift winner reddedilir. |
| Blackjack | Deste sırası, oyuncu/dealer elleri, kapalı kart ve tur sonucu. | Constructor artık rastgele bir el dağıtıp rekor değiştirmez. `start` yalnız yeni oturumda dağıtır. Restore sonuç/ace/natural kurallarını doğrular, tekrar ödül yazmaz. Çift shuffle/deal ses çağrısı kaldırıldı. |
| Minesweeper | Zorluk, gerçek gizli mayın haritası, açılmış hücreler, bayraklar, aktif süre. | Adjacency gerçek haritadan yeniden hesaplanır; first-click güvenliği korunur; bitişte süre son kesriyle güncellenir. |
| 2048 | Tahta, skor, hamle sayısı; won/game-over tahtadan türetilir. | Pause girdisi engellenir; yalnız geçerli power-of-two tile değerleri kabul edilir; bir hamlede çiftlerin bir kez birleşmesi test edilir. |
| Memory Match | Kart dizilimi, eşleşmeler, tek seçilmiş kart, hamle ve aktif süre. | Bekleyen yanlış çift restore sırasında kapatılır; eski timer yeni tahta üzerinde çalışmaz; eşleşmiş çift ve symbol frekansları doğrulanır. |
| Reaction Tap | Son ölçüm, tur sayısı, toplam/ortalama ve eski rekor. | `waiting/ready` süreçlerindeki duvar saati ölçümü restore'da `idle` olur. Background süresi yeni bir rekor üretmez. |

Altı sayfa `GameAudio.playEffect` kullanır; grup volume/modern audio lifecycle yolu geçerlidir. Eski QML pause wrapper'ları kaldırıldı. Responsive scroll düzenleri ve 2048'in merkezi `GameInput` yönlendirmesi korunur. Yeni load/TLS hata metinleri EN/TR/AZ'dedir. Restore/yazma hataları GameHost içinde görünür; korunmuş eski kayıt sessizce yeni oturumla değiştirilmez.

## Ekran görüntüsündeki `TLS backend: none; available: none`

İki ayrı paket gereksinimi vardır:

1. Önceki patch, gerçek `libssl_3.so → libcrypto_3.so` ELF dependency adını düzeltmişti. Bu düzeltme korunur.
2. Yayımlanmış v0.7.2 armeabi-v7a APK'sının **derlenmiş binary AndroidManifest.xml** dosyasında `extractNativeLibs=false` bulundu. TLS plugin'i ZIP içinde mevcut ve store/uncompressed idi. Qt 6.10.2 `QFactoryLoader`, Android'de native library dizinindeki `libplugins_tls_*.so` dosyalarını listeler; çıkarılmayan dosyalar bu taramada görünmez. Bu paket bulgusu ekran görüntüsündeki backend yokluğuyla uyumludur; yeni APK'nin kullanıcının cihazındaki sonucu henüz ölçülmedi.

Uygulanan çözüm:

- Manifest `android:extractNativeLibs="true"`; ortak Gradle packaging helper `jniLibs.useLegacyPackaging=true`. Her CI/release APK/AAB yolu aynı helper'ı kullanır. Native ELF 16 KiB koruması ve `zipalign -P 16` kontrolü devam eder.
- Android build, `Qt6::QTlsBackendOpenSSLPlugin` yoksa durur; `qt_import_plugins` ile plugin açıkça deployment'a dahil edilir.
- Başlangıç, JNI `ApplicationInfo.nativeLibraryDir` yolunu Qt plugin aramasına ekler, `_3` OpenSSL suffix'ini korur, Android'de `openssl` backend'ini seçer ve backend/library/path teşhisini loglar.
- APK validator kaynak XML'e güvenmez: aapt'ın gerçek binary manifest'inde extraction boolean'ını denetler. False/missing attribute yayın kontrolünden geçmez.
- Mod katalog hatası uygulanabilir ve çevrilmiş mesaj verir. HTTPS veya sertifika kontrolü devre dışı bırakılmaz.
- Android emulator smoke testi normal QML açılışından sonra, kurulu uygulamayı `applicationArguments=--tls-smoke-test` ile yeniden başlatır. Gerçek Qt/OpenSSL stack'i katalog HTTPS endpoint'ine bağlanmalı, sertifika doğrulanmalı, HTTP 200 gelmeli ve `PASS: HTTPS catalog probe; certificate validated;` logu çıkmalıdır. `curl` sonucu bu koşulun yerine geçmez. Timeout/failed backend testi başarısız kılar.

> Sonraki tema/ses incelemesi host playback katmanını geliştirdi; güncel ek çalışma [V073_THEME_AUDIO_UPDATE.md](V073_THEME_AUDIO_UPDATE.md).

## Doğrulama ve sınırlar

Yerel ortam Linux x86_64, Qt 6.8.2 ve Release build; Multimedia etkindir.

- **17/17 CTest**: önceki suite + native builtin state persistence + gerçek Qt TLS plugin discovery. Native suite **71 davranış assertion'ı** ile bütün oyunların save/reopen, CBOR round-trip, pause, invalid state koruması ve legacy boş save yükseltmesini çalıştırır.
- **11/11 Python package-validator testi**: binary manifest native extraction pozitif/negatif testleri dahil.
- Yerel gerçek HTTPS probe **geçti**: `backend=openssl`, sertifika doğrulaması ve katalog HTTP 200. Yerel runtime OpenSSL 3.5.7; Android pinned 3.5.8 pipeline bu testle karıştırılmaz.
- Plugin search path boşaltıldığında TLS backend bulunamaması ve probe'un ağ isteği öncesi başarısız olması, doğru path geri geldiğinde backend recovery, ayrı offline CTest'te doğrulanır.
- Yayımlanmış v0.7.2 APK yeni binary-manifest koşulunda reddedildi; önceki yanlış DT_NEEDED kontrolü de korunur.
- Source/project/sanity/release/distribution ve installer kontrolleri geçer. i18n: 0 error, 22 mevcut coverage uyarısı. Mevcut security static check: 0 error, 1 önceki pattern uyarısı; yeni güvenlik araştırması yapılmadı.

Bu ortamda yeni imzalı Android APK üretilmedi veya telefona yüklenmedi. Android'e özgü JNI/deployment/extraction değişiklikleri yeni tam SHA'da CI/artifact build ve cihaz/emulator logcat ile doğrulanmalıdır. Önceki platform QA/yayın koşulları sürer. Kullanıcının şu an kurulu APK'sı kaynak kod değişince kendiliğinden güncellenmez; düzeltilmiş paket kurulmalıdır. Public release/push veya backend değişikliği yapılmadı.
