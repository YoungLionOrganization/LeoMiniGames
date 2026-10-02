# v0.7.2 yayımlanan paketler — v0.7.3 ön incelemesi

> Uygulama notu (1 Ekim 2026): Bu belge ilk planın/denetimin tarihsel metnidir. Güncel kod değişiklikleri, testler ve bekleyen fiziksel yayın koşulları [V073_IMPLEMENTATION.md](V073_IMPLEMENTATION.md) içinde izlenir. Aşağıdaki eski sürüm bulguları yeni kodun durumu olarak okunmamalıdır.

29 Eylül 2026. Kaynak tag'i: `3f022fca55016e3f954662e4519d614b0c44bab4`. Bu rapor bir cihaz `logcat` kaydı içermez; kullanıcının `armeabi-v7a` APK'sının açılmadığı bildirimi ile paketlerin bağımsız incelenmesini ayırır.

## Android: kesin paket hatası, muhtemel açılış sebebi

Yayımlanmış **dört tek mimarili** APK'nin içindeki `libssl_3.so` dosyalarının ELF `DT_NEEDED` girdisi `libcrypto.so`; APK'lerin her biri yalnızca `libcrypto_3.so` içeriyor. Aynı dört APK `libssl_3.so` ve `libcrypto_3.so` kitaplıklarını Android başlangıcında yüklemek için listelemiş. Bu uyuşmazlık sistemdeki özel `libcrypto.so` kopyasına güvenilmesine veya `dlopen` başarısızlığına yol açar; modern Android'de bu kütüphane uygulamalara güvenilir biçimde sunulmaz. Android Qt yükleyicisi, kitaplık yüklenemezse başlangıcı başarısız sayar. Bu, kullanıcının açılmama bildirimi için güçlü teknik açıklamadır; cihazdaki kesin hata satırı için `adb logcat` gerekir.

Kök neden: `tools/ci/build_android_openssl.sh` şu komutu kullanır: `patchelf --replace-needed libcrypto.so.3 libcrypto_3.so "$OUT/$abi/libssl_3.so"`. Gerçek `DT_NEEDED=libcrypto.so` olduğundan komut eşleşmez. `tools/validate_android_package.py` ELF mimarisini ve 16 KiB hizalamasını doğrular, ancak `DT_NEEDED` bağlarını kontrol etmediği için hem CI hem artifact build yeşil kalmıştır.

| APK | İncelenen ABI | `libssl_3.so` bağımlılığı | Paket içi karşılığı |
| --- | --- | --- | --- |
| Android-armeabi-v7a | armeabi-v7a | `libcrypto.so` | yalnız `libcrypto_3.so` |
| Android-arm64-v8a | arm64-v8a | `libcrypto.so` | yalnız `libcrypto_3.so` |
| Android-x86 | x86 | `libcrypto.so` | yalnız `libcrypto_3.so` |
| Android-x86_64 | x86_64 | `libcrypto.so` | yalnız `libcrypto_3.so` |

Universal APK aynı üretim betiğini kullanır; dört ABI'nin içeriği ayrı doğrulanmalı. Paketlerin minSdk=28, targetSdk=36, versionCode=702; incelenen tek ABI paketlerde v2/v3 imza blokları var. Yayımlanmış dört APK, mevcut `tools/validate_android_package.py` denetiminden geçmişti; bu kontrolün bağımlılık yönünden eksik olduğu görüldü.

## Diğer platformlar

| Alan | Gerçekleştirilen kontrol | Sonuç / sınır |
| --- | --- | --- |
| GitHub CI | Yayımlanan SHA için tüm raporlanan işler: Windows, Ubuntu, Debian, Arch, macOS, Android, iOS/iPadOS | Yeşil; CI donanımda uygulamayı açma kanıtı değildir. |
| Build Release Artifacts | 27 genel dosya; ilgili platform işlerinin tamamı | Yeşil; Android paketindeki kırık ELF bağımlılığı yine de geçmiş. |
| Windows x86_64 ZIP | ZIP CRC ve 96 PE dosyasının x86_64 mimarisi; DLL import taraması | Arşiv ve mimariler sağlam. `VCRUNTIME140*.dll` / `MSVCP140*.dll` importları var ve ZIP içinde bu dosyalar yok. Temiz Windows ortamında redistributable önkoşulu ayrıca doğrulanmalı. Kullanıcının önceki installer testi mevcut bir Windows makinedeydi. |
| macOS universal ZIP | ZIP CRC ve 108 Mach-O dosyasının iki mimari dilimi (`x86_64`, `arm64`), çözülmeyen paket içi `@rpath` taraması | Bu taramalarda sorun görülmedi. macOS üzerinde çalıştırma, Gatekeeper ve imza testi burada yapılmadı. |
| Linux Ubuntu 22.04 x86_64 portable | tarball 1.865 üye ile açıldı, ana ELF `ldd` bağımlılıkları listelendi | Başlangıç ve oyun akışı için grafik oturumu gerekli. Yerel kaynak derlemesi ortam değişince tamamlanamadı; başarılı uzak CI işleri gerçek paket çalıştırma yerine geçmez. |
| Installer scriptleri | `node tests/test_installer_scripts.js` | Mevcut kurulum yönlendirme ve kısayol testleri geçti. |

Kaynak: [v0.7.2 Release](https://github.com/YoungLionOrganization/LeoMiniGames/releases/tag/v0.7.2), [Qt'nin Android OpenSSL paketleme yönergesi](https://doc.qt.io/qt-6/android-openssl-support.html), [Qt Android ek kütüphane yükleme özelliği](https://doc.qt.io/qt-6/cmake-target-property-qt-android-extra-libs.html), [Qt Windows dağıtım notları](https://doc.qt.io/qt-6/windows-deployment.html).
