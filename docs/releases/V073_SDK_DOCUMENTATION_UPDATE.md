# v0.7.3 dokümantasyon ve SDK tamamlama

1 Ekim 2026. Önceki runtime/builtin/TLS ve tema/ses değişikliklerinin üzerine gelir. Uygulama 0.7.3, QML host API 0.7, rcc-v1, tema API 1 ve native ABI/IID 1.0 ayrı tutulur. Eski RCC çağrıları veya native interface struct'ı değiştirilmedi; yeni güvenlik araştırması yapılmadı.

## Dokümantasyon

SDK Overview/Quickstart ve bütün public context-object property/method/signal referansı tamamlandı. Referans doğrudan C++ başlıklarından üretilir; CI drift ve src/main.cpp context coverage kontrolü yapar. GameRuntime.capabilities property'sinin method gibi çağrılması, eski API 0.6 başlıkları, locale normalizasyonu, clock timeScale sınırı, keychain Remember davranışı ve henüz uygulanmamış settings editor/font/native yayın araçları açıklamaları düzeltildi.

Save/slot/schema/backup, lifecycle sırası, settings, audio, input/focus, viewport/safe-area, theme/live revision, i18n/live language, random/clock/events, stats/achievements/haptics/logger, HTTPS izinleri, native SDK, compatibility, troubleshooting ve yayın acceptance sayfaları kodla eşleştirildi. Theme SDK format/schema ve release guide'ın candidate/platform QA koşulları tamamlandı. Yerel Markdown linkleri kontrol edilir; tarihsel release/denetim metinleri güncel uygulama kaydıyla karıştırılmaz.

## SDK örnekleri ve build

- ExampleHelloMod legacy manifest/Audio çağrılarını korur.
- ExampleModernMod 1.1.0 API 0.7 ve minimum app 0.7.3 kullanır. Root lifecycle hooks, schema 1→2 migration, hatalı kayıt koruması, her hamlede kayıt, tek keyboard/touch aksiyonu, stats/achievement, optional haptics, canlı theme/language/settings bağları ve short-screen scroll tamamlandı. Focused Qt button + global fire aynı tuş için iki hamle üretmez.
- Mevcut C++ GameSave.hasStoredSlot(slot) QML invokable oldu. Backup-only veriyi primary listSlots ile karıştırmamak için additive read-only API'dir; old API çağrıları değişmez. Modern örnek bozuk backup-only kaydı yeni oturum olarak yazmaz.
- ExampleTheme 1.2.0, eski default tema kopyası yerine görülebilir Graphite Gold palette, partial gradient ve RCC içindeki SVG texture'ı gösteren küçük bir overlay'dir; geri kalan değerler default'tan tamamlanır. Theme API 1 açıkça belirtilir.
- Native SDK, public IGamePlugin interface'iyle bağımsız derlenen QObject/QML örneği içerir. Desktop test build'i örneği compile eder; host'a yüklemez veya native review/trust izni vermez. Private BuiltinGame/lmg_core public SDK gibi sunulmaz.
- Ortak Python builder source/QRC/resource kontrolleri, Qt 6 rcc/zlib derleme, atomic RCC replacement ve SHA-256 sidecar üretir. Linux/macOS ve Windows wrappers her çalışma dizininden çalışır; eski batch literal newline hataları giderildi. Build başarısızsa mevcut output korunur. Tool bir tam host/publisher validator'ı veya yayın/sign aracı değildir.

## Kanıt ve sınırlar

Linux x86_64, Release, Qt 6.8.2:

- **19/19 CTest geçti**. Gerçek SDK RCC'leri external engine'de yüklendi; migration/reopen, pause, keyboard/button parity ve focused keyboard, achievement, live language/status/settings/theme, short landscape, corrupt backup-only koruması ve legacy Audio örneği doğrulandı.
- **10/10 SDK tool testi geçti**: required packaged icon/locale, duplicate/unsafe RCC, schema/capability tipleri, private import, Qt 5 reddi, failed-build output koruması ve batch newline biçimi.
- Native SDK ayrı proje olarak ve desktop test build içinde derlendi. Shell wrapper /tmp çalışma dizininden çağrılarak çalıştırıldı; Windows batch gerçek Windows'ta çalıştırılmadı, source/format kontrolü yapıldı.
- **13/13 paket testi**, source/project/sanity/release/distribution kontrolleri geçer. Distribution 293 PASS, 0 ERROR. i18n 0 ERROR, 22 mevcut host coverage uyarısı.
- Markdown link/context/signature kontrolleri CI ve artifact workflow'a eklendi. Bu CI tanımları kaynakta hazırdır; önceki GitHub push 403 engeli nedeniyle yeni aday uzak CI'da çalışmadı.

Physical device/speaker/Bluetooth, signing/upgrade ve 21-platform release QA tamamlandı diye işaretlenmez. SDK örneği ve authoring schema publisher approval veya backend production kurulum kanıtı değildir. Save Transfer/cloud sync, otomatik font yükleme, shared settings editor/VS Code, native signing/trust yayın otomasyonu ve voice-pool/audio-focus gibi ilerideki özellikler mevcut destek gibi anlatılmaz. Lisans metinleri veya publisher hakları değiştirilmedi.

Başlangıç: [SDK Overview](../sdk/OVERVIEW.md), [Quickstart](../sdk/QUICKSTART.md), [API Reference](../sdk/API_REFERENCE.md).
