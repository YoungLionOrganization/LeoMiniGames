# v0.7.3 Dependabot incelemesi ve uygulaması

1 Ekim 2026. Üç açık Dependabot PR'ı okunarak değişiklikleri yerel `patch/v0.7.3` dalına uygulanmıştır. Uzak PR'lar kabul edilmedi; kullanıcı kabul edecektir. Bunlar build/release araçlarıdır, oyun bağımlılığı ya da Qt/JDK sürümü yükseltmesi değildir.

| PR | Güncelleme | Değerlendirme ve kabul önerisi |
| --- | --- | --- |
| [#3](https://github.com/YoungLionOrganization/LeoMiniGames/pull/3) | install-qt-action 4.3.1 → 4.4.1; SHA `a9c63c7c123f3069cff414e7e482d95dfa9d8125` | Önerilir. Android/WASM için `all_os` host varsayılanı ve aqt çıktı ayrıştırma düzeltmeleri; Windows py7zr varsayılanı 1.1.0'a sabitleniyor. Açık PR'daki 44 kontrol başarılı. Explicit Qt 6.10.2, host, arch, modules ve aqt seçenekleri korunur. |
| [#4](https://github.com/YoungLionOrganization/LeoMiniGames/pull/4) | download-artifact v7 → v8 | Önerilir. Digest uyuşmazlığı artık varsayılan olarak hata verir; v0.7.3 yayın bütünlüğü koşullarıyla uyumlu. Açık PR'daki 44 kontrol başarılı. upload-artifact v7 ile kullanılan arşiv indirme/çıkarma düzeni korunur; `skip-decompress` açılmaz, `digest-mismatch` gevşetilmez. |
| [#5](https://github.com/YoungLionOrganization/LeoMiniGames/pull/5) | setup-java v5 → v6 | Önerilir, ancak mevcut kırmızı kontrol düzeltilip CI tekrar çalıştırıldıktan sonra kabul edilmeli. Temurin JDK 17 ve Android build seçenekleri korunur. v6 Node24/ESM kullanır; çağıran workflow için ESM geçişi uyumludur ve GitHub hosted runner'lar kullanılır. |

## #5'in kırmızı kontrolü

GitHub CI çalışması `36797929687` static validation'da başarısız; derleme işleri atlanmış. PR SHA `303eac3e0ce22840a339ab83b93f54c3fdb494c6` ayrı worktree'ye alınarak `tools/validate_distribution.py` çalıştırıldı:

```text
ERROR: CI semantic actions/setup-java@v5
SUMMARY: 292 PASS, 0 WARNING, 1 ERROR
```

Eski doğrulayıcı bir action sürümünü literal metin olarak zorunlu tutuyordu. Kontrol artık hem CI hem artifact workflow'undaki Android ve universal işlerinde resmi `actions/setup-java` referansı, tek kurulum adımı, Temurin dağıtımı ve JDK 17 girdisini doğrular. Major sürüm güncellemesi Java gereksinimini değiştirmez. Bu düzeltme #5'in kendi diff'inde yoktur; güncellenmiş patch dalı alınmalı veya aynı doğrulayıcı değişikliği PR'a taşınmalıdır.

## Doğrulama ve sınırlar

- Güncellenmiş yerel sürüm: dağıtım denetimi 296 PASS, 0 WARNING, 0 ERROR.
- Aynı düzeltilmiş doğrulayıcı PR #5 kaynak ağacında geçti; JDK girdileri 21 yapıldığında iki Android işi doğru biçimde reddedildi.
- Üç action'ın upstream `action.yml` girdileriyle 21 kullanım karşılaştırıldı; bilinmeyen input ya da eski referans kalmadı.
- 13/13 paket doğrulayıcı testi geçti. Runtime C++/QML kodu değişmedi; önceki 19/19 CTest sonucu bu kaynaktan önce alınmıştır.
- Uzak açık PR kontrolleri eski main tabanına aittir; birleşik v0.7.3 adayının uzak CI sonucu değildir. Yerel değişiklikler GitHub'a gönderilmedi; mevcut gönderim engeli sürüyor. Uzak PR merge, release, imzalama ve cihaz QA yapılmadı.

Upstream değişiklik kaynakları: [Qt action 4.4.1](https://github.com/jurplel/install-qt-action/releases/tag/v4.4.1), [download-artifact v8](https://github.com/actions/download-artifact/releases/tag/v8.0.0), [setup-java v6](https://github.com/actions/setup-java/releases/tag/v6.0.0).
