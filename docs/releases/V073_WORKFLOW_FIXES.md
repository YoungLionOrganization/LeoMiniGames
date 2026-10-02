# v0.7.3 workflow hata incelemesi ve düzeltmeleri

2 Ekim 2026. İncelenen uzak aday: `e2a72d83630aef2f858fd7000fd146f5b50440cd` (`main`). [CI 37041857225](https://github.com/YoungLionOrganization/LeoMiniGames/actions/runs/37041857225) ve [artifact 37041956508](https://github.com/YoungLionOrganization/LeoMiniGames/actions/runs/37041956508) tam log ZIP'leri okunmuştur. Başarısız işlerin ortak nedenleri aşağıdadır; derleme/yayın kapıları gevşetilmemiştir.

| Hata grubu | Log bulgusu | Düzeltme |
| --- | --- | --- |
| macOS SDK CTest | Üç mimaride `sdk_examples`: native stil özelleştirme uyarıları ve `SDK launch has no QML errors` assertion'ı. | Test, gerçek ana uygulamadaki gibi QQuickStyle Basic seçer. SdkButton doğrudan QtQuick.Controls.Basic import eder; aynı bileşen bağımsız önizlemede de çalışır. |
| Windows SDK CTest | MSVC/LLVM-MinGW ve x64 artifact testinde SDK testi abort ediyor; Windows logu assertion adını göstermiyor. | Aynı stil düzeltmesi; platformdan bağımsız geçici INI settings, QStandardPaths test modu ve her çalışmada benzersiz uygulama veri/cache alanı. Assertion başarısızlığı stderr'e numarasıyla açıkça yazılır. Windows'taki kesin assertion adı eski logda yoktur; yeniden CI doğrulaması gerekir. |
| Android emulator | x86/x86_64/universal CI ve imzalı artifact işi 300 saniyelik boot beklemesinde exit 124. Emülatör logu job stdout'unda yok. | Host emulator çağrısından Android Qt library/plugin/QML ortamını temizle; explicit ABI/pixel_2/5554 port ve snapshot'sız software GPU başlat. Emülatör PID'sini izle, erken kapanırsa hemen gerçek logu göster, boot timeout'unda logcat topla. Boot bütçesi 600 saniye; TLS probe ve canlı uygulama şartları korunur. Qt ortam çakışması kodda giderildi; eski emülatör artifact logu indirilemediği için kesin başlangıç hata metni doğrulanmadı. |
| Windows ARM64 package | `...VC/Redist/MSVC/v145/arm64` bulunamıyor. | Env'nin verdiği ilk redist diziniyle sınırlanma; vswhere ile kurulu VS redist sürümlerini hedef mimari için sırayla ara. v145'te ARM64 yoksa v143 ARM64 CRT seçilebilir. Hedef CRT yoksa açık hata; PE mimarisi ve imported DLL closure denetimi korunur. |
| Kaynak aktarımı | Uzak main'de beş shell dosyası 100755 yerine 100644. | Yerel Git dalında executable bitler korundu; workflow shell dosyalarını zaten bash ile çağırıyor. Kaynak ZIP kullananlar için ayrıca main tabanlı uygulanabilir binary diff sağlandı. |

Uzak main kaynak ZIP içeriğini tek commit ile taşıdığı için önceki yerel SDK/Dependabot commitleri ancestry'de yoktu. Uzak main çalışma dalına merge edildi; runtime kodu aynıydı. Önceden incelenen üç Dependabot güncellemesi ve JDK 17 semantic doğrulayıcısı korunmuştur; [inceleme kaydı](V073_DEPENDABOT_UPDATE.md).

## Kanıt

- Linux Qt 6.8.2 Release yeniden build ve 19/19 CTest başarılı.
- 10/10 SDK araç ve 13/13 paket testleri başarılı.
- 3 Android shell regresyon testi: üç ABI'de sahte araçlarla boot/launch/TLS akışı, temiz host Qt ortamı; erken emülatör ölümü; eksik APK reddi. Bu testler gerçek cihaz/emülatör testi değildir.
- PowerShell 7.5.3 ile 3 MSVC discovery regresyonu: v145 x64 / v143 ARM64 layout'unda doğru mimari seçimi ve eksik ARM64 CRT reddi. Tüm tools/tests PowerShell dosyaları parser denetiminden geçti. Her Windows artifact işi bu testi gerçek runner'da da çalıştırır.
- Dağıtım denetimi: 296 PASS, 0 WARNING, 0 ERROR. SDK API/dokümantasyon bağlantıları ve source guard/proje/sanity denetimi başarılı.
- Yeni adayın gerçek Windows/macOS/Android GitHub sonuçları bu yerel kanıtların yerine geçmez; uzak gönderim ve tekrar CI durumuna göre ayrıca kaydedilmelidir. İmzalı artifact/yayın veya cihaz QA tamamlandığı iddia edilmez.
