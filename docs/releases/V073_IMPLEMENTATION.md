# v0.7.3 uygulama ve doğrulama kaydı

1 Ekim 2026. Çalışma dalı: `patch/v0.7.3`; başlangıç SHA'sı: `3f022fca55016e3f954662e4519d614b0c44bab4` (`v0.7.2`). Bu kayıt hem [ilk patch planının](V073_PATCH_PLAN.md) 16 işini hem kullanıcının [TODO](V073_TODO.md) ve [platform denetimi](V073_PLATFORM_AUDIT.md) belgelerini izler. API sürümü 0.7, uygulama sürümü 0.7.3, Android versionCode 703. Yeni güvenlik açığı araştırması kullanıcının talebiyle kapsam dışında bırakılmıştır; onaylanmış plandaki kayıt/paket bütünlüğü düzeltmeleri ve mevcut CI denetimleri uygulanmıştır.

> Dokümantasyon/SDK sonraki tamamlama kaydı: [V073_SDK_DOCUMENTATION_UPDATE.md](V073_SDK_DOCUMENTATION_UPDATE.md).

> Tema/ses için sonraki inceleme ve ortak playback düzeltmeleri: [V073_THEME_AUDIO_UPDATE.md](V073_THEME_AUDIO_UPDATE.md).

> Sonraki kullanıcı isteğiyle altı builtin’in tam oturum kaydı v0.7.3 kapsamına alındı. Ekran görüntüsündeki TLS sorunu için `extractNativeLibs` düzeltmesi ve gerçek HTTPS smoke kontrolü eklendi. Güncel ek çalışma: [V073_BUILTIN_TLS_UPDATE.md](V073_BUILTIN_TLS_UPDATE.md). Aşağıdaki tablo ilk uygulama anındaki kapsamı kaydeder.

## İlk planın kod değişiklikleri

| İş | Uygulanan değişiklik | Davranış doğrulaması / sınır |
| --- | --- | --- |
| P073-01 | `PackagePaths.h`: kanonik mod dizini, symlink/dizin kaçışı reddi, sürüm metni yerine SHA-256 disk anahtarı; ortak kesin ID kontrolü. | `catalog_operations`: dış dizin/symlink ve uninstall kontrolleri. |
| P073-02 | `GameSave`: başarısız veya gelecekteki şemaya ait yükleme sonrasında otomatik yazmayı engelleme; kurtarılan backup'ı koruma; aynı payload/schema için tekrar yazmama; CBOR metadata tipleri ve migration derinlik/boyut sınırı. | `save_safety`: future schema, load-before-write, bozuk veri, backup ve tekrar kaydetme. |
| P073-03 | Katalog yenilenirken kurulu sürümün gerçek manifest/API/capabilities/save schema bilgilerini koruma; harici runtime'a kurulu RCC'nin bilgilerini verme. | `catalog_operations`: katalog yeni, kurulu paket eski olduğunda runtime metadata korunur. |
| P073-04 | `PackageCompatibility.h`: install/restart/Developer import ortak uyumluluk; aday doğrulama eski RCC'yi kaldırmadan önce; indeks commit hatasında rollback; çalışan paket için kullanım kilidi; uninstall staging/restore; eski `.part` temizliği. | `catalog_operations`: incompatible update, aktif paket, indeks hata/geri dönüş. Elektrik kesintisi ve bütün dosya sistemi hata kombinasyonları test edilmiş sayılmaz. |
| P073-05 | `AppController` tek, tekrar girişe dayanıklı kapanış; save/forceSave/close/unload sırası; QML sayfasını engine retirement öncesi kaldırma; quit ve Developer temizliği aynı koordinatörle. | `external_engine_session`, altı builtin'in QML oturum testleri; kapanış bir kez ve JS callback bittikten sonra destruction. |
| P073-06 | Save migration actual harici `QQmlEngine` ile component yaratılmadan önce bağlanır; engine değişiminde callback temizlenir; root/engine destruction ertelenir. | `external_engine_session`: nested migration, v0.5/v0.6/v0.7 gerçek fixture QML yükleme, reentrant close. |
| P073-07 | Native `QKeyEvent`, gerçek focus kökü, metin alanı hariç tutma, focus/background reset, physical/logical token takibi, Linux xcb/Wayland scan normalizasyonu; 2048 ortak action servisi. | `patch_runtime`: aynı action için iki tuş, release ve clock/input reset. Windows/macOS native keyboard cihaz testi bekler. |
| P073-08 | Memory callback iptal/pause/resume; Memory/Mines aktif zamanında kesirli saniyeyi koruma; Reaction background round'u geçersiz kılma; clock reset, idempotent audio pause. | `patch_runtime` timer/reset/false-start/background testleri; gerçek ses cihazında kontrol bekler. |
| P073-09 | Tema ID/sürüm/API kontrolü, yalnız tema namespace/veri dosyaları, boyut sınırı; gerçek RCC mount prefix'ini asset URL'den ayırma; remove hata geri dönüşü. | `theme_package_identity`: uyumsuz kimlik, cache yolu, namespace, gerçek unmount ve başarılı remove. |
| P073-10 | Mod/tema katalog malformed/duplicate yanıtlarında mevcut modeli koruma; loading/install yarışını engelleme; origin değişiminde eski yanıtı reddetme; indirmede buffer/boyut/deadline sınırı. | `catalog_operations` injected network. Canlı üretim backend'i değiştirilmedi; canlı uçtan uca TLS/katalog doğrulanmış değildir. |
| P073-11 | Audio cache/release için aynı normalize edilmiş kaynak anahtarları; music pause/resume state koruma. | Multimedia modülüyle Release derleme. Gerçek platform ses çıkışı yayın QA'sında. |
| P073-12 | Ortak strict SemVer: overflow, prerelease, stable/preview; 443/same-origin URL; reply generation; stale action temizliği; başarısız automatic check tekrar denenebilir. | `patch_runtime`, `catalog_operations`: parser precedence, stale reply/channel/link ve stable prerelease reddi. |
| P073-13 | Tam 27 asset adı; nonempty/format/archive/native dependency/Windows CRT kontrolü; SHA-256/size/source SHA internal manifest; remote digest/size; üç QtIFW depo sürümü/payload SHA-1; Release'de etkin assertion ve gerçek fixture dependency. | `test_package_validators.py`, Release CTest. GitHub draft/upload/recovery uçtan uca çalıştırılmadı. |
| P073-14 | Injectable credential store; sistem QtKeychain jobs sıralı; logout delete eski write ardından çalışır; stale read/write state'i değiştirmez; plaintext fallback kapalı. | `credential_store_order`: gecikmiş read/write/delete. Gerçek OS keyring/locked-store testleri platform QA'sında. |
| P073-15 | Builtin'lerin yalnız istatistik tuttuğu doğru belgelenir; kısa ekranda scroll/adaptive layout; Qt >=6.9 native safe area, 6.8 fallback; bounded local stats ve yazma hatası raporu; EN/TR/AZ yeni hata metinleri. | Altı builtin QML launch/close ve 640×360 testi; QML uyarıları smoke testini başarısız kılar. Native safe area, RTL, 200% font ve cihaz dokunma QA'sı bekler. Board resume/save transfer 0.8 kapsamıdır. |
| P073-16 | CMake/runtime/package/installer/workflow/release metadata 0.7.3/703; genel sürüm validator'ı; notes/changelog; eski backend açıkça historical reference. | Genel release/static validator'lar. Backend kurulmadı; production paket/tag/release yayımlanmadı. |

## Sonradan verilen platform planı

| Denetim bulgusu | Kodda uygulanan çözüm | Gerçek yayın kanıtı |
| --- | --- | --- |
| Android `libssl_3.so` yanlış `DT_NEEDED` | OpenSSL script gerçek `libcrypto.so` veya `.so.3` adını `libcrypto_3.so` ile değiştirir; SONAME/needed zorunlu. | Yayımlanmış armeabi-v7a APK yeni bağımlılık kontrolünde reddedilir. Yeni beş APK'nin gerçek build çıktısı bekler. |
| APK içeriği / bütün ABI'ler | Her `.so` için aynı ABI dependency closure; public NDK allowlist; 64-bit ELF ve iki TLS libinde 16 KiB; universal dört ABI; package/versionCode kontrolü. | Üretim APK'lerinde `apksigner` ve `zipalign`; mevcut v0.7.2 sertifika digest'i `release/android-signing.json` ile sabitlenir. Yeni imzalı APK/upgrade testi bekler. |
| CI uygulamayı hiç açmıyor | x86 API28, x86_64/universal API35 emulator install/start/process/logcat evidence; hem CI hem imzalı artifact workflow. | Workflow kodu eklendi; bu yerel dal için GitHub çalışması henüz yapılmadı. ARM telefon, birkaç dakikalık kullanım ve v0.7.2 üstüne upgrade ayrıca bekler. |
| Windows CRT eksik | Aynı mimaride MSVC redist DLL'leri ZIP ve installer ortak payload'ına eklenir; PE mimarisi/import closure doğrulanır. | Synthetic PE pozitif/negatif testleri geçer. Üç mimaride temiz Windows kurulum/upgrade/modify/uninstall bekler. |
| Linux/macOS/iOS/iPadOS temiz açılış | 21 hedefli QA şablonu ve doğrulayıcı; exact source SHA ve aday artifact digest ile yayın ön koşulu; Apple mobile imzasız çıktılar test artifact olarak belgelenir. | Linux yerel offscreen testi diğer platform/hardware kanıtı sayılmaz. Temiz GUI sistemleri, Gatekeeper, native audio/TLS/keyring ve iOS signing testi bekler. |

## Yerel doğrulama

Yerel SDK: Qt 6.8.2, GCC, Linux x86_64; Multimedia etkin. CI üretim SDK'sı Qt 6.10.2 olarak korunur. Release (`NDEBUG`) testleri assertion'ları kaldırmaz.

- Release CTest: **15/15 geçti**; 15 davranış/entegrasyon testi; bunlar save, runtime, catalog/trust, network, Developer auth, lifecycle/legacy engine, keychain, theme ve builtin QML oturumlarını kapsar.
- Python paket doğrulayıcıları: **10/10 geçti**; 32/64-bit ELF, eski OpenSSL adı, eksik native bağımlılık, yanlış ABI/hizalama, bozuk arşiv, eksik/yanlış Windows CRT, APK certificate parser ve candidate QA/QtIFW integrity.
- Source/project/sanity, mevcut security/static check, distribution, prebuilt audit ve canonical release validator; installer JavaScript kontrolleri; Bash syntax ve Python compile kontrolleri.
- Distribution: 293 PASS, 0 ERROR. i18n: 0 ERROR, 22 mevcut kapsam uyarısı; diğer diller deterministic source fallback kullanır. Security static check: 15 PASS, 0 ERROR, 1 secret-persistence pattern uyarısı; gerçek credential writes QtKeychain üzerinden ve plaintext fallback kapalıdır. Bu çalışma yeni güvenlik araştırması veya tam güvenlik sertifikası değildir.

Test komutları `docs/BUILDING.md` ve `.github/workflows/ci.yml` içinde. Yerel kaynak ZIP’i de oluşturuldu ve içerik validator’ından geçirildi. Yerel derleme çıktıları/logları repo dışındadır. Release paketi, tam candidate SHA'da GitHub CI ve artifact build ile yeniden üretilmelidir.

## Yayına kalan koşullar

`release/platform-qa.template.json` varsayılanları bilerek başarısızdır. Gerçek 21 hedef sonucunu environment/tester/time/evidence, artifact_name ve SHA-256 ile doldurun; `tested_artifacts` içinde her hedefin bütün yayın formatlarını (ZIP/Setup, tar/AppImage, ZIP/DMG) ayrı hash ile kaydedin; Android actual ABI, Windows servicing ve macOS Gatekeeper alanları zorunludur. **Record candidate platform QA** workflow'u bu JSON'u denetler ve `platform-qa` artifact üretir. Publish Release `qa_run_id` bu koşunun numarasıdır; publish aşaması tested digest'i gerçek candidate manifest ile karşılaştırır. QA geçmeden draft/tag/yayın aşamasına gidilmez.

Bu makinede Android cihaz/emulator SDK, Windows, macOS veya iOS imzalama ortamı bulunmadığından bu sonuçlar üretilemedi. QA şablonu geçer gösterilmedi. Her iki planın yerel kod değişiklikleri uygulandı; release acceptance bütün platform kanıtları gelince tamamlanır. Kullanıcının talebi kod uygulamasıdır; remote push, production backend değişikliği veya public yayın yapılmadı.
