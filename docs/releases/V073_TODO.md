# LeoMiniGames v0.7.3 — yayın engelleri ve test planı

> Uygulama notu (1 Ekim 2026): Bu belge ilk planın/denetimin tarihsel metnidir. Güncel kod değişiklikleri, testler ve bekleyen fiziksel yayın koşulları [V073_IMPLEMENTATION.md](V073_IMPLEMENTATION.md) içinde izlenir. Aşağıdaki eski sürüm bulguları yeni kodun durumu olarak okunmamalıdır.

Bu liste, yayımlanmış `v0.7.2` tag'i (`3f022fca55016e3f954662e4519d614b0c44bab4`) ve yayımlanan paketlerin 29 Eylül 2026 denetimine dayanır. Buradaki maddeler **v0.7.3 için yapılacak işlerdir**, uygulanmış düzeltmeler değildir. `MAJOR.MINOR.PATCH` kuralına göre oyun/mod API'sini değiştirmeyen servis yamasıdır.

## P0 — Android uygulama açılmıyor

- [ ] `tools/ci/build_android_openssl.sh` içinde `libssl_3.so` bağımlılığının **gerçek** `DT_NEEDED` değerini kontrol et. v0.7.2 APK'sındaki değer `libcrypto.so`; betik yalnızca `libcrypto.so.3` adını `libcrypto_3.so` olarak değiştirmeye çalıştığı için sonuçta çözülmeyen `libcrypto.so` kalmış. OpenSSL 3.5.8 çıktısına uygun adla değiştir veya iki olası adı da kapsa; `patchelf --page-size 16384` sonrasında `readelf -d` ile `SONAME=libssl_3.so` ve `NEEDED=libcrypto_3.so` değerlerini zorunlu kıl. Beklenen ad yoksa betiği başarısız sonlandır.
- [ ] `tools/validate_android_package.py` denetimine ELF `DT_NEEDED` bağımlılıklarını ekle. Paket içi bütün `.so` dosyalarının ihtiyaçları aynı ABI altındaki dosyalar veya yalnızca izin verilen Android NDK sistem kütüphaneleriyle çözülebilmeli. Özellikle `libssl_3.so` → `libcrypto_3.so` ve iki OpenSSL kitaplığının 16 KiB hizalaması kontrol edilmeli; yayımlanmış v0.7.2 APK'leri doğrulayıcıda **başarısız** olmalı.
- [ ] Beş APK'nin (`armeabi-v7a`, `arm64-v8a`, `x86`, `x86_64`, universal) tamamını yeniden üretip gerçek APK içeriğini doğrula; universal paket içinde dört ABI'nin her birini ayrı denetle. İmzayı, yükseltme uyumluluğunu, versionCode=703 ve mevcut kurulumun verilerini koruyarak yükseltmeyi kontrol et.
- [ ] Android emülatör ve gerçek telefonlarda temiz kurulum + v0.7.2 üzerine yükseltme yap; `adb logcat` ile ilk açılışı, birkaç dakikalık kullanımı ve yeniden başlatmayı denetle. Kullanıcının denediği `armeabi-v7a` yolunu özellikle doğrula; Redmi 7, SM-A346E ve eldeki diğer cihazların gerçek ABI listesini `adb shell getprop ro.product.cpu.abilist` ile belirle. QML ekranı, katalog HTTPS/TLS, Developer Lab anahtar saklama, çevrimdışı açılış, ses ve kaydetme yollarını deneyerek ikinci hataları ara.
- [ ] Android CI'ye APK kurup açan cihaz/emülatör smoke testi koy. Yalnızca C++ derleme, ZIP içeriği ve ELF hizalaması açılış başarısını kanıtlamaz.

## P1 — diğer platformların paketleri

- [ ] Windows x86_64, x86_64 AVX2 ve ARM64 ZIP/Setup paketlerini temiz sanal makinelerde ilk kurulum, mevcut kurulum üstüne güncelleme, Maintenance Tool ve kaldırma için çalıştır. Denetlenen yayımlanmış x86_64 ZIP'de `VCRUNTIME140.dll` ve `MSVCP140.dll` yok, PE içe aktarmaları bu çalışma zamanlarını istiyor. Bu DLL'lerin veya uygun Visual C++ Redistributable'ın gerçekten sağlandığını/sessiz kurulduğunu doğrula; eksikse paketlemeyi düzelt. AVX2'yi yalnızca destekleyen CPU'da çalıştır.
- [ ] Ubuntu 22.04/24.04 x86_64, Ubuntu 24.04 ARM64, Debian 13 ve Arch paketlerini temiz kurulumda gerçek masaüstü oturumuyla aç. Portable tarball, AppImage ve native paketlerde Qt platform eklentisi, QML, ses, ağ, kayıt ve güncelleme bağlantılarını kontrol et. Bu testleri mevcut başarılı CI/CTest'e ek olarak yap.
- [ ] macOS Intel, Apple Silicon ve universal ZIP/DMG paketlerini gerçek macOS makinelerde aç; iki mimari dilimini, Gatekeeper, uygulama içi kaynaklar, ses, dosya erişimi ve DMG davranışını kontrol et. Kısa vadeli imzasız dağıtım kısıtları kullanıcıya doğru anlatılmalı.
- [ ] iOS/iPadOS için CI'nin ürettiği imzasız test paketlerini dağıtım sürümü olarak adlandırma. İmzalı geliştirme kurulumunda cihaz ve simülatör açılışını ayrıca doğrula.

## Tamamlanma şartı

`v0.7.3` ancak beş Android APK'sinin gerçek ELF bağımlılıkları kapalı, cihaz/emülatörde açılış ve yükseltme başarılı, tüm hedef platformların temiz makine açılışları belgelenmiş, CI ve Build Release Artifacts yeni tam SHA'da başarılı ve yayın ön kontrolü geçmişse yayımlanmalı. Üretim backend'i ayrıca kurulmadıysa TLS/katalog bütünleşmesi doğrulandı diye işaretlenmemeli.
