# LeoMiniGames v 0.7.3 patch planı

> Uygulama notu (1 Ekim 2026): Bu belge ilk planın/denetimin tarihsel metnidir. Güncel kod değişiklikleri, testler ve bekleyen fiziksel yayın koşulları [V073_IMPLEMENTATION.md](V073_IMPLEMENTATION.md) içinde izlenir. Aşağıdaki eski sürüm bulguları yeni kodun durumu olarak okunmamalıdır.

İnceleme tarihi: 30 Eylül 2026 (UTC)
Repo: https://github.com/YoungLionOrganization/LeoMiniGames
Yerel checkout: `/workspace/LeoMiniGames`
Baz commit: `3f022fca55016e3f954662e4519d614b0c44bab4`
Baz sürüm: `main` ve `v 0.7.2` aynı commit. Önerilen hedef: `v 0.7.3`, Android versionCode `703`, oyun API sürümü `0.7` olarak kalır.

## 1. Sonuç ve kapsam

v 0.7.3 için önerim; kayıt koruma, kurulu paket kimliği, oyun oturumunun kapanması, gerçek input/lifecycle davranışı ve release doğrulamasını düzelten bir bakım sürümüdür. Yeni oyun, yeni save formatı veya kapsamlı UI yenilemesi eklemek yerine, mevcut sözleşmenin kodda gerçekten uygulanmasını sağlamalı.

En acil iki alan: katalogdan gelen `version` değerinin dosya yoluna doğrulanmadan eklenmesi ve başarısız/yapılmamış save yüklemesinin ardından koşulsuz force-save yapılması. İlki uygulama veri dizininin dışına yazma riskidir; ikincisi kurtarılabilir oyuncu verisini boş veya yanlış schema ile değiştirme riskidir.

Bu belge uygulama değişikliği değil, uygulanabilir iş planıdır. Kaynak kod, tag, release ve canlı sunucu değiştirilmedi. Deneyler geçici dosyalarda yapıldı.

### Bağlamın sınırları

- Geçmiş ChatGPT sohbetlerini arayan araç bu oturumda mevcut değil. Önceki sohbetler okunmadı. Sohbet kaynaklı istek veya onay uydurulmadı; kullanıcıdan ilgili kararları paylaşması istendi.
- Erişilebilir bağlam: README, CHANGELOG, v 0.7.2 release/validation belgeleri, önceki güvenlik ve oyuncu audit raporları, SDK belgeleri, git geçmişi, kaynak kod, testler ve workflow dosyaları.
- GitHub issue aramasında issue bulunmadı. PR aramasında Android setup ve Java setup için iki bağımlılık PR’ı bulundu; ilgili güncel workflow zaten v4/v5 kullanıyor. Bunlar açık ürün hatası olarak plana taşınmadı.
- `JustLachin/LeoMiniGames` isimli ikinci repo da bulunuyor. Yönetim erişimi bulunan organizasyon reposu esas alındı; ikinci repo ile karşılaştırma yapılmadı.
- Buradaki P0/P1/P2 sıralaması önerilen yayın önceliğidir; bir CVSS sınıflaması değildir.

## 2. İnceleme ve doğrulama kanıtı

İncelenen ana yollar: uygulama başlangıcı/context servisleri; AppController/StackView/GameHost; harici QQmlEngine; save/migration/lifecycle; input/clock/audio; altı built-in oyun; mod ve tema katalog/install/rollback; publisher trust/RCC namespace kontrolleri; Developer Lab/network/keychain; updater; Android ve masaüstü paketleme; release aggregation/publication; eski PHP backend hotfix’i.

### Bu oturumda çalıştırılan kontroller

| Kontrol | Sonuç |
| --- | --- |
| `tools/source_guard.py` | PASS |
| `tools/validate_project.py` | PASS; 6 built-in, 63 QML dosyası |
| `tools/sanity_check.py` | PASS |
| `tools/security_audit.py` | 16 PASS, 0 WARNING, 0 ERROR |
| `tools/validate_i18n.py` | 0 ERROR, 22 coverage warning; Türkçe 113/207 kaynak metin |
| `tools/validate_distribution.py` | 293 PASS |
| `tools/audit_prebuilt_binaries.py` | PASS |
| `tools/validate_v072.py` | PASS |
| `node tests/test_installer_scripts.js` | PASS |
| Release doğrulayıcısına 27 adet sıfır bayt sahte asset | **Hatalı kabul: exit 0 / PASS** |

Loglar aynı dizindeki `check-results.json`, `*.log` ve `asset-validator-probe.json` dosyalarında bulunur. Asset deneyi yalnızca `validate_release_assets.py` kabul açığını kanıtlar; bütün publish workflow’unun sahte paketlerle geçeceği iddia edilmez.

Bu ortamda `cmake`, Qt SDK/qmake ve PHP yürütücüsü bulunmadı. C++ build, CTest, QML runtime, fiziksel cihaz ve PHP runtime testi bu incelemede çalıştırılmadı. Önceki belgelerdeki 8/8 CTest sonucu tarihsel kanıttır; bu oturumun sonucu değildir.

### Canlı backend’e yapılan sınırlı, kimlik bilgisiz GET kontrolleri

| Endpoint | Gözlem |
| --- | --- |
| `/api/v1/health` | HTTP 200; `1.2-schema-repair`, DB/schema ready |
| `/api/v1/mods?limit=1` | HTTP 200; `data` array ve pagination `meta` |
| `/api/v1/themes?limit=1` | HTTP 200; `data` array ve pagination `meta` |
| `/api/v1/updates/v1/check?...linux...current=0.7.2` | HTTP 200; schema 1, `leominigames-update-v1`, v 0.7.2, 27 asset, Ubuntu 24.04 x64 AppImage önerisi |

Developer key gönderilmedi; upload, publish, install veya deploy yapılmadı. Presigned URL query’leri kaydedilen kanıtta maskelendi. Bu dört GET, diğer platformların veya kimlik doğrulamanın sağlıklı olduğunu kanıtlamaz.

## 3. Öncelikli iş listesi

| ID | Öncelik | İş | Kanıt düzeyi | Yayın koşulu |
| --- | --- | --- | --- | --- |
| P073-01 | P0 | Paket dosya yolu ve version containment | Kaynakta doğrulama boşluğu | Zorunlu |
| P073-02 | P0 | Başarısız yüklemeden sonra save koruma | Kaynak akışında somut risk | Zorunlu |
| P073-03 | P1 | Kurulu manifest ile katalog metadata’sını ayırma | Kaynakta somut tutarsızlık | Zorunlu |
| P073-04 | P1 | Install/restart uyumluluğu ve disk transaction | Kaynakta eksik kontroller | Zorunlu |
| P073-05 | P1 | Tek oyun oturumu ve güvenli close sırası | Kaynakta eksik yollar | Zorunlu |
| P073-06 | P1 | Harici engine migration ve QML kök ömrü | Kaynak bulgusu; Qt repro gerekli | Repro + düzeltme zorunlu |
| P073-07 | P1 | Native input, focus ve tuş durumları | Kaynakta somut hatalar | Zorunlu |
| P073-08 | P1 | Timer, pause/resume ve müzik | Kaynakta somut hatalar | Zorunlu |
| P073-09 | P1 | Tema paketinin katalog kimliğiyle eşleşmesi | Kaynakta eksik karşılaştırma | Zorunlu |
| P073-10 | P1 | Katalog şekli, duplicate ID, network sınırları | Kaynakta somut boşluklar | Zorunlu |
| P073-11 | P1 | Ses cache anahtarının normalize edilmesi | Kaynakta somut sınırsızlık | Zorunlu |
| P073-12 | P1 | Update parsing, channel ve stale sonuçlar | Kaynak bulguları | Zorunlu |
| P073-13 | P1 | Artifact içeriği/provenance ve test güvenilirliği | Negatif testte yeniden üretildi | Zorunlu |
| P073-14 | P2 | Keychain job sıralaması | Race hipotezi; backend’e bağlı | Repro sonucu ile karar |
| P073-15 | P2 | Built-in save vaadi, safe area ve çeviri | Kod/belge farkı ve eksik QA | Belge doğruluğu zorunlu |
| P073-16 | P1 | Sürüm atomikliği ve backend dağıtım sınırı | Kaynak/belge farkı | Zorunlu |

## 4. İşlerin ayrıntıları

### P073-01 — Mod version değeri dosya yolunu belirlememeli

**Kanıt:** `src/core/ModManager.cpp:348` katalog version’ını string olarak alıyor. `:440` install ön koşulları id/entry/hash/size denetliyor; version için güvenli segment kontrolü yok. `:563` dizin `mods/<id>/<entry.version>` biçiminde kuruluyor. Sonraki işlemler rename/remove/removeRecursively kullanıyor.

**Tetikleyici:** Katalogdan `version="../../outside"` gelmesi. Doğru bir RCC ve hash sunan üçüncü taraf katalog da bu yolu kullanabilir. Dosya yolu hatası kaynakta görülebiliyor; dış dizine yazan saldırı bu incelemede çalıştırılmadı.

**Değişiklik:**

1. Oyun sürümünün görüntülenen metnini disk anahtarından ayır. Legacy sürüm metinlerini topluca reddetmek yerine diskte hash veya güvenli kodlanmış segment kullan.
2. Her yazma/silme işlemi öncesinde normalize edilmiş hedefin beklenen oyun köküne ait olduğunu doğrula. Symlink/canonical parent sınırını da ele al.
3. `installed.json` içindeki file/version verilerini yeniden açılışta aynı sınırla kontrol et. Host tarafından üretilmiş beklenen path dışında dosya mount edilmesin.
4. `RccPackageInspector::validId` ve ModManager regex’lerini tek kimlik kuralında birleştir; kontrol karakterleri, NUL, newline, slash ve backslash reddedilsin.

**Regresyon:** `../`, `../../`, Windows drive/UNC, `%2f`, newline, boş version, prerelease ve tarihli legacy version; geçici kökün dışında sentinel dosyalar byte-for-byte korunur. Mevcut v 0.5/v 0.6 kurulumları güvenli path migration ile açılır.

### P073-02 — Save yüklenemediyse mevcut kayıt otomatik olarak ezilmemeli

**Kanıt:** `GameSave.cpp:activate` belleği boşaltıyor; `load` başarısız veya schema daha yeni olduğunda false dönüyor. `forceSave()` `:249` koşulsuz autosave yazıyor. `Main.qml:20`, `main.cpp:195`, background ve quit yolları bunu çağırıyor. Close yolunda hem QML hem C++ force-save bulunuyor.

**Tetikleyici:** Oyunun kaydı schema 2, çalıştırılan oyun schema 1; load reddediliyor, kullanıcı geri dönüyor. Boş/eksik belleğin autosave’a yazılması mevcut kayıt ile backup’ın korunmasını tehlikeye atıyor. Oyun QML yüklenmeden hata verse de benzer risk var.

**Değişiklik:**

1. Save oturumu durumunu açık tut: henüz yüklenmemiş, geçerli yüklenmiş, yeni oyun bilinçli başlatılmış, kurtarmadan yüklenmiş, incompatible/corrupt.
2. Lifecycle force-save; incompatible load veya başarısız oyun yaratımı sonrası mevcut dosyayı yazmasın. Oyunun açık bir yeni oyun/replace kararı ayrı yol olsun; legacy `set()` kullanımını koruyan güvenli başlangıç kuralı belirle.
3. Backup’tan okunan oturumun primary’si bozuksa geçerli backup korunarak iyileşsin. Aynı close olayında tekrar yazıp aynı içeriği backup’a döndürme.
4. `autosave` ve adlandırılmış slot semantiğini belgede açıklaştır. `createSlot` davranışı korunur; bu patch’e save transfer eklenmez.
5. Save hata sonucu diagnostics’e ve uygun oyuncu bildirimine taşınsın. Metadata/checksum/schema doğrulamasında eksik/tür yanlış alanlar reddedilsin; migration sonucuna da snapshot/depth/size kontrolü uygulansın.

**Regresyon:** Future schema, eksik migration, callback exception, bozuk primary+geçerli backup, load öncesi QML Error, dolu disk ve permission error. Otomatik kapatma/arka plan/quit sonrası korunması gereken dosyaların hash’i değişmez. Geçerli yeni oyun ve legacy normal save çalışır. Testler gerçek CBOR dosyaları kullanır.

### P073-03 — Katalogdaki yeni sürüm, kurulu eski oyunun runtime sözleşmesini değiştirmemeli

**Kanıt:** `ModManager.cpp:393` katalog saveVersion’ını alıyor; kurulu oyun merge’ünde `:418` yalnızca değer <=1 ise eski değer dönüyor. apiVersion/capabilities/locales için de katalog önceliği var. `applyManifestMetadata` saveVersion’da `qMax` kullanıyor. `main.cpp` oyun açarken bu değerlere bakıyor.

**Tetikleyici:** Kurulu v1 manifest’i save schema 1 kullanıyor; katalogda v2 schema 2 yayınlanıyor. Kullanıcı paketi güncellemeden refresh yapıyor; v1 runtime schema 2 ile aktive olabiliyor. Yeni capabilities de henüz yüklenmemiş eski pakete taşınabiliyor.

**Değişiklik:** `CatalogMetadata` ve `InstalledRuntimeMetadata` ayrımı oluştur. Runtime için api/minApi/requiredCapabilities/saveVersion/settingsSchema/locales/defaultLocale/entry kurulu ve doğrulanmış RCC’den gelsin. Katalog yalnızca sunum, mevcut güncelleme ve origin’e bağlı trust bilgisi sağlar. Version’lar `catalogVersion` ve `installedVersion` olarak ayrı kalır.

**Regresyon:** Kurulu v1 + katalog v2/schema2; katalog refresh, offline restart ve failed update sonrası runtime v1/schema1 kalır. Başarılı update sonrası atomik biçimde schema2’ye geçer. Trust modern/unverified/third-party fixture’ları korunur.

### P073-04 — Install ve restart aynı pakete aynı kararı vermeli

**Kanıt:** `registerInstalled` `ModManager.cpp:946` required capabilities ve API compatibility kontrolü yapıyor; download sonrası install yolu RCC inspection/mount sonrası aynı kontrolü yapmıyor. `saveInstalled():1047` void; open/write/commit başarısızlığı install sonucuna dönmüyor. Uninstall `removeRecursively()` sonucunu yok sayıyor. Replacement rollback bütün metadata’nın snapshot’ını almıyor.

**Değişiklik:**

1. Ortak package validation fonksiyonu: id, entry, manifest, API/minApi, required capabilities ve kurulu runtime metadata. Installed/restart/developer yolları bunu aynı şekilde kullanır.
2. Dosya finalize, mount ve index commit bir transaction olsun. Eski dosya/mount/index, yeni index başarıyla commit edilmeden silinmesin. Başarısızlıkta tüm metadata ve mount geri dönsün.
3. Index yazma hata sonucu döndürsün; success/model sinyalleri commit’ten sonra yayılsın.
4. Running package için update/uninstall oturum lease’i uygula: oyun açıkken aynı RCC unmount/deletion yapılamasın. Kullanıcıya oyunu kapatma eylemi sun.
5. Uninstall disk hatasında gerçekte kurulu paketi UI’dan kaybolmuş gösterme. Eski başarısız `.part/.rollback` kalıntılarının açılışta bounded temizliği olsun.

**Regresyon:** Unsupported API/capability ilk install’da da restart’ta da reddedilir; eski sürüm korunur. Rename, write, commit, mount ve delete hata enjeksiyonu; same-version reinstall; different-version upgrade; işlem sırasında refresh; aktif oyun sırasında update/uninstall.

### P073-05 — Oyun kapatma tek oturum akışından geçmeli

**Kanıt:** `AppController.cpp:closeGame` current state’i temizliyor ve yalnızca gameClosed yayıyor. Lifecycle/save/clock temizliği burada yapılmıyor. `LegacyAppFacade.cpp` bunu doğrudan çağırabiliyor. `Main.qml:134` sadece gameOpened için StackView.push yapıyor; gameClosed için pop/remove senkronizasyonu yok. `main.cpp:192` safety net, `Lifecycle.save/close/unload` çağırmıyor.

**Tetikleyici:** Legacy mod `App.closeGame()` çağırıyor; host sayfası stack’te kalıyor, scoped save güncel state’i snapshot’lamadan kapanabiliyor. Tekrar oyun açılması ikinci GameHost push oluşturabiliyor. Harici item kendi çağrısı içinde senkron unload ile silinebiliyor; reentrancy Qt repro ile doğrulanmalı.

**Değişiklik:** Host-owned tek session coordinator oluştur. Close isteği başladığında input kapat; lifecycle save → kontrollü persist → close/unload → item/engine teardown → service deactivation → navigation güncelle sırasını uygula. İşlem idempotent ve callback içinden tekrar close’a dayanıklı olsun. QML’de çalışan harici item’in teardown’ını güvenli zamanlamayla yap. Yeni hedef oyun geçersizse mevcut oyunu gereksiz kapatma. StackView, yalnızca aktif session’a karşılık gelen bir GameHost tutsun.

**Regresyon:** Header back, Android back, `App.closeGame`, programmatic game switch, quit, load failure, twice close, close çağıran QML callback. Her oturum bir kez snapshot/close/unload olur; boş sayfa ve use-after-free oluşmaz; Developer import clear/logout aktif oturumu düzgün sonlandırır.

### P073-06 — Save migration, oyunun JavaScript engine’iyle çalışmalı

**Kanıt:** `main.cpp:156` GameSave engine’ini ana QQmlApplicationEngine’e bağlıyor. Harici oyun ayrı engine’de yaratılıyor. `GameSave.cpp:198` callback’e ana engine ile üretilmiş script argument’i gönderiyor. QQmlComponent root oluşturulmadan önce `component.create()` çağrılıyor; QML Component.onCompleted o aşamada çalışabilir. Non-QQuickItem error yolunda object deleteLater edilirken engine hemen siliniyor.

**Durum:** Engine sınırı kaynakta kesin; cross-engine callback’in gerçek hata mesajı/sonucu Qt integration testi ile doğrulanmalı. Bu belge çalıştırılmamış runtime sonucunu kesin kabul etmiyor.

**Değişiklik:** External runtime gerçek engine’i save session’a component creation’dan önce bağlasın veya callback için engine-owned bridge kullansın. Engine teardown öncesi QJSValue migrations temizlensin; ana engine’e dönme sırası açık olsun. Root lifecycle/focus ve scoped service başlangıcını tamamlamak için `beginCreate/completeCreate` benzeri kontrollü creation değerlendirilsin. Kök Item yerine başka QObject olduğunda object/engine ömürleri güvenli kapatılsın.

**Regresyon:** Gerçek v 0.7 RCC, harici engine’de schema1→2 JS migration kaydetsin, eski disk kaydını yükleyip nested state’i dönüştürsün. Callback error ve close sırasında tutulan callback; root QObject ve missing import. Legacy fixture’lar yalnızca mount değil, yaratım ve lifecycle açısından da çalıştırılsın.

### P073-07 — Fiziksel input ve focus doğru kaynaktan alınmalı

**Kanıt:** `GameHost.qml:124` Qt Quick KeyEvent’ten `nativeScanCode` okumaya çalışıyor; bu alan Qt Quick KeyEvent’in belgelenmiş QML yüzeyi değil, native QKeyEvent’te bulunur. Linux GameInput, X11 ve evdev kodlarını aynı tabloda kabul ediyor; örneğin X11 code30 başka bir fiziksel tuşken A olarak eşleşiyor. External loaded callback gizli built-in Loader’a focus veriyor. 2048 ayrıca kendi Keys handler’ına sahip.

**Değişiklik:** Native kodu C++ QKeyEvent köprüsünde al; aktif oyun/aktif window/focus sınırı içinde tek kez route et. Linux platform plugin’i için tek doğru keycode uzayını kullan; input kodunu backend’e göre normalize et. Harici QQuickItem focus scope’unu aktive et. UI TextField ve menu kontrollerinde gameplay tuşlarını yakalama. Bir action’a bağlı birden fazla tuş için pressed-source takip et; W+Up basılıyken birini bırakmak diğerini bırakmış sayılmasın. Focus kaybı ve background’da reset uygula.

**Regresyon:** X11/Wayland ayrımı, Windows, macOS; QWERTY/QWERTZ/AZERTY/TR/AZ; arrows ve fiziksel WASD; unknown key; two keys one action; repeat; focus loss; Developer TextField; harici RCC actionPressed; 2048’de hareketin iki kez yapılmaması. Mac scanCode0 ile synthetic unknown event’in A sanılması ayrıca test edilir.

### P073-08 — Pause/resume ve eski callback yeni oyuna taşmamalı

**Kanıt:** MemoryMatch `:157` 650 ms singleShot reset generation’ı bilmiyor; `reset():62` eski callback’i iptal etmiyor. Memory/Minesweeper/ReactionTap QML köklerinde pause/resume hook yok; native oyun class’ları da bu hook’ları sağlamıyor. `GameClock::reset` paused/timeScale durumunu yeni session için normalize etmiyor. `GameAudio::pauseAll` ikinci pause’da resumeMusic=true bilgisini false’a çevirebiliyor; inactive→hidden/suspended çoklu notification yolu var.

**Değişiklik:** Memory pair timer owned/cancellable olsun veya generation token ile reset’ten sonraki eski completion no-op olsun. Built-in root hook’ları native game pause/resume’a bağlansın. Memory/Minesweeper zamanı aktif oyun süresi üzerinden yürüsün; ReactionTap background’a geçerse mevcut round iptal/invalid sayılıp foreground’da eski süreden rekor üretmesin. Session start clock reset+resume ve varsayılan timescale uygulasın. Music pause idempotent olsun, resume intent yalnız stop/new-source/session change’de temizlensin. `pauseAll` ses efektleriyle ilgili gerçek davranışıyla uyumlu olsun.

**Regresyon:** Mismatch→reset→650ms; reset sonrası yeni mismatch. Ardışık inactive/hidden/suspended→active; pause sırasında timer ilerlemez. Pause→close→yeni oyun clock çalışır/timeScale1 olur. Müzik bir kez devam eder; sessiz fallback’te hata yoktur.

### P073-09 — Tema hash’i kadar kimliği ve API’si de doğrulanmalı

**Kanıt:** ThemeCatalog `:540` installThemeRcc’ye yalnız dosya ve SHA veriyor. ThemeManager candidate id’yi JSON’dan alıp o id’yi değiştirebiliyor; katalog id/version ile karşılaştırma yok. themeApiVersion modele yazılıyor, install ön şartında desteklenen API kontrolü yok.

**Tetikleyici:** Catalog A satırının doğru hash’li dosyası manifest’te B id’sini taşıyor. A kuruldu gösterilirken B değişebilir. İleri theme API sürümü mevcut sözleşmeyle kabul edilebilir.

**Değişiklik:** Catalog install expected id/version/theme API taşısın; doğrulama map swap’tan önce yapılsın. Local/sideload install kimliğini package’den almayı sürdürebilir, katalog yolundan açık ayrılır. Bilinmeyen theme API fail-closed olsun. Tema kaynak doğrulaması yalnız `/theme` altını değil bütün RCC’yi kapsasın; count/decompressed-size limitleri ve data-only kuralı uygulansın.

**Regresyon:** A catalog→B manifest; version mismatch; future API; built-in id replacement; theme dışındaki `.qml/.js` dosyası; aktif tema update ve disk hatasında eski tema/texture root korunur. Aynı hash reinstall idempotent kalır.

### P073-10 — Hatalı katalog boş katalog gibi kabul edilmemeli

**Kanıt:** Her iki manager da doğrudan `root.data.toArray()` kullanıyor; data object/string/missing olduğunda boş array’a dönüşüyor. Duplicate id’ler fresh listeye birden fazla girebiliyor. Mod download callback’leri row index taşıyor. Refresh guard var ama install, devam eden refresh sırasında başlayabiliyor; completion model reset’i row’ları kaydırabilir. Binary download yollarında absolute süre sınırı/expected-size erken abort yok; yalnız transfer inactivity timeout ve global boyut sınırı var. Theme URL kontrolleri mod tarafındaki userInfo reddiyle aynı değil.

**Değişiklik:**

1. Şimdiki `data: []` sözleşmesini koru; `data.items` gibi desteklenen legacy biçimleri ortak parser ile bilinçli kabul et. Başka şekillerde önceki iyi model kalsın, hata görünür olsun.
2. Duplicate id’yi deterministik reddet/diagnostic üret. Katalog satır sayısı ve metin/list alanları bounded olsun.
3. Install while loading ve refresh while operation iki yönde korunmalı; async işler row yerine id+operation generation ile satır bulsun. Origin değişirse eski response yeni origin’e trust taşımasın.
4. URL kabul kuralı ortak olsun: HTTPS, userInfo yok, açık host/port politikası, ticket aynı origin. R2/CDN binary URL’leri hash-bound biçimde desteklenmeye devam etsin.
5. Binary response’ta bounded read buffer, expected size aşımında erken abort, absolute deadline ve cleanup. Örnek başlangıç politikası: 30s metadata, 60s inactivity + 10dk absolute package; gerçek düşük hızlı cihaz testinde ayarlanır.
6. Pagination bug olarak ilan edilmez: canlı katalog şu an 18 mod/15 tema ve limit100’e sığıyor. >100 satır fixture’ında eksik listelenme varsa bounded pagination ekle.

**Regresyon:** Malformed/duplicate catalog, refresh sırasında install, network origin change, stale reply, model reset, exact-size+1, chunked body, slow trickle, R2 redirect, error cleanup; offline installed liste korunur.

### P073-11 — Named audio cache sınırlı kalmalı

**Kanıt:** `AudioManager.cpp:65` source’u `name.trimmed().toLower()` ile buluyor, fakat cache lookup/insert `:67/:71` ham name ile yapılıyor. `"click"`, `" CLICK "`, farklı boşluk/case kombinasyonları aynı ses için farklı QSoundEffect yaratıyor. DynamicEffects 32 sınırı bu ayrı named map’i sınırlamıyor. Legacy Audio.play harici oyunlara açık.

**Değişiklik:** Normalized named key lookup/insert boyunca tek anahtar kullanılsın; cache en fazla tanımlı builtin ses sayısı kadar olsun. Unknown name için bounded diagnostics; restore volume sırasında dynamic effect’in gain’ini kaybetme problemi ayrı küçük düzeltme olarak ele alınsın.

**Regresyon:** Binlerce whitespace/case varyasyonu tek effect’e gider; unknown sesler cache büyütmez; dynamic 32 sınırı, releasePrefix ve soundEnabled/volume davranışı korunur. Kaynakların gerçek decoder durumları olan Qt smoke testi eklenir.

### P073-12 — Update kararları katı ve channel’a bağlı olmalı

**Kanıt:** UpdateService SemVer regex’i empty prerelease segmentlerini ve numeric leading zero’ları reddetmiyor; toInt overflow kontrolü yok. `isAllowedUpdateUrl` host/path kontrol ediyor ama portu sınırlamıyor. Request NoLessSafe redirects kullanıp son host’u sonradan denetliyor. Yeni check başında eski URL/version/notes tamamen temizlenmiyor; parse error eski eylem hedeflerini tutabilir. Stable mode ayrıca prerelease’i yerelde reddetmiyor.

**Değişiklik:** Standalone test edilebilir SemVer parser/comparator; numeric taşma/leading-zero/empty segment kontrolü; stable prerelease filtresi. Metadata request same-origin/manual kontrollü redirects. İzinli effective port443; normalleştirilmiş ve beklenen gateway route’ları. Her request’in reply’si ve channel generation’ı capture edilsin; önceki completion yeni sonucu değiştiremesin. Yeni check/error’da stale action hedefleri temizlensin veya UI açıkça last-known-good ve current-error’ı ayırsın. Auto retry zamanı başarısız check için 12 saat sessizlik doğurmasın; bounded retry/backoff olsun.

**Regresyon:** SemVer resmi precedence örnekleri; `1.0.0-01`, `1.0.0-rc..1`, çok büyük integer, build metadata; stable/preview; older/equal/newer; 443/8443; foreign redirect; channel change + late finished; ilk success sonra malformed response; failed check sonrası openUpdate.

### P073-13 — Release asset sayısı, artifact doğruluğu değildir

**Kanıt:** `tools/release/validate_release_assets.py` filename glob/count/forbidden kontrolleri yapıyor; non-empty/content format kontrolü yok. Bu oturumda wildcard’ları FAKE ile doldurup 27 sıfır bayt dosya oluşturuldu; script bunları kabul etti. `publish-release.yml` remote upload doğrulamasında yalnız isim setini karşılaştırıyor. `tests/test_catalog_compat.cpp` C assert kullanıyor; Release/NDEBUG build’de assertions kalkabilir. CTest RCC fixture’ları ağırlıkla mount testleri; gerçek UI/lifecycle kapsamı yok.

**Değişiklik:**

1. Wildcard cardinality yerine beklenen 27 tam adı/platform/arch matrix’i doğrula. Non-empty ve makul min/max; ZIP/tar/DMG/APK/PE/AppImage format smoke kontrolü ve beklenen entry’ler.
2. SHA256/size/source commit/platform/arch içeren internal artifact manifest oluştur. Public asset sayısı 27 korunabilir; manifest internal workflow evidence olarak taşınır.
3. Upload sonrası remote size/state/digest varsa digest, yoksa bounded download-and-hash doğrula. İsim eşitliği tek kontrol olmasın.
4. QtIFW Updates.xml version, üç arch, payload varlığı/size/hash doğrulansın; başka sürüm payload’ı yayınlanmasın.
5. Publisher-only delta ile build reuse mevcut tasarımı belgede açıkla. Binary/source/package değiştiyse fresh exact-target build ve CI şartı sürsün; reusable candidate’de build SHA ve publisher workflow SHA ayrı kaydedilsin.
6. Release testlerinde assert yerine release’de çalışan fail/check mekanizması. Fixture RCC dependency listesine manifest/QML/assets ekle; incremental build fixture değişikliğini kaçırmasın.
7. Yeni testler source-string varlığını değil davranış/hata sonucunu ölçsün. Mevcut static validators korunur fakat tek başına release acceptance olmaz.

**Regresyon:** 27 boş dosya kesin FAIL; doğru sayıda yanlış arch isimleri FAIL; corrupted ZIP/APK FAIL; eksik DLL/ELF FAIL; wrong hash/size, stale SHA, duplicate repository, partial upload, stale draft, public existing release. Publish recovery staging/mocked API ile doğrulanır; gerçek release bunun için oluşturulmaz.

### P073-14 — Keychain completion iptali, disk işlemini iptal etmiş sayılmamalı

**Kanıt:** DeveloperManager generation check read/write completion’ında var. Logout generation artırıp DeletePasswordJob başlatıyor; eski WritePasswordJob’un gerçek backend yazımı job callback’inin ignore edilmesiyle iptal edilmiyor. Delete completion’da generation kontrolü yok. Mevcut developer_auth_races testi injected network kullanıyor ve keychain’i özellikle devre dışı bırakıyor.

**Durum:** Kullanılan OS keychain job queue garantisine bağlı race hipotezi; kanıtlanmış credential sızıntısı olarak raporlanmaz.

**Plan:** Injectable credential-store abstraction ve fake delayed write/read/delete ekle. Store işlemlerini sırala; logout delete, önceki write bitince de kesin uygulanmalı. Eski delete yeni login’in savedKey state’ini değiştirmesin. QtKeychain’in Windows/macOS/Linux/Android kuyruğunu incele; race yoksa bulguyu kapat ama testsiz güvenceyi belgeye dönüştür.

**Regresyon:** Remember→logout→late write; logout→new remember→old delete; locked keyring; delete error; restart’ta key’nin yeniden görünmemesi. Plaintext fallback yasağı ve opt-in korunur.

### P073-15 — Oyuncu beklentisi ile mevcut built-in davranışı eşleşmeli

**Kanıt:** README persistence tablosu Blackjack/Minesweeper/2048/MemoryMatch için game state/stats yazıyor. Native oyunlar constructor/reset ile yeni board/round başlatıyor; GameLocalStats rekor/istatistik tutuyor. İlgili QML sayfalarında load/save hook yok. Save force çağrısı kendi başına bu native board’u snapshot’lamaz. `GameHost.syncViewport` normal cihazda safeTop/Bottom 0 geçiriyor; gerçek platform inset akışı burada yok. Built-in board sayfaları kare board + header/stats ile kısa yatay ekranlarda taşabilir.

**Patch kapsamı kararı:** v 0.7.3’te önce mevcut vaadi düzelt: hangi oyun yalnız stats tutuyor açık yazılsın. Yeni board persistence bu incelemede uygulanacak küçük bir hotfix değildir; kullanıcı sohbetindeki açık resume gereksinimi gelirse ayrı scope kararı verilir. Save Transfer/.lmgsave export-import v 0.8.0’a bırakılır. Formatı değiştirmeden resume eklenmesi mümkün olsa da her oyunun state validation/migration/testi gerekir.

**QA ve küçük düzeltmeler:** Qt 6.10 build’de gerçek safe-area API’si + Qt 6.5 fallback seç; simulated profile gerçek inset kanıtı sayılmasın. 360×640, 640×360, tablet, desktop DPI ve font büyütmede footer/input erişimi test edilir; sorun çıkan sayfalara width+height bounded board/scroll yerleşimi eklenir. Yeni hata/eylem metinleri en/az/tr’de tam; 22 dil için deterministik fallback ve placeholder parity korunur. GameLocalStats’ın bounded read ve save-error raporlaması eklenir.

**Regresyon:** Altı built-in launch/round/reset/stat reload; Memory timer correction; 2048 merge kuralları ve hareket yokken tile eklenmemesi; Minesweeper ilk click güvenliği; Blackjack ace/natural; ReactionTap false-start. TR/AZ/EN ve bir RTL locale, 200% ölçek, klavye focus.

### P073-16 — Sürüm ve dağıtım kaynakları birlikte güncellenmeli

**Kanıt:** CMake, main.cpp, build-artifacts env, installer XML ve release JSON’larda 0.7.2 tekrar ediyor. validate_v072 kendi içinde 0.7.2/702 hardcode ediyor. Eski backend-hotfix-v1.0.2 `public/index.php` yalnız mod/admin routes içeriyor; live health 1.2-schema-repair. Eski Http.php body size sınırı ve yeni authorization fallback’i içermiyor. Release notes’ta sözü edilen güncel companion bu eski klasörle aynı şey değil.

**Değişiklik:**

1. Canonical version metadata’dan CMake/runtime/package workflow/installer değerlerini üret veya release-agnostic validator ile eşitliğini zorunlu kıl. Android 703 ve uygulama 0.7.3; SDK/API 0.7 olarak kalır.
2. `release/release.json`, `release/assets.json`, `release/notes/v 0.7.3.md`, README/CHANGELOG/RELEASES, installer config/package XML, build workflow ve testler birlikte güncellenir. Tarihsel v 0.7.2 notları değişmez.
3. `validate_v072` release invariants’ını version’dan bağımsızlaştır; v 0.7.3 regression kontrollerini davranış testleriyle ekle. Publisher-only allowlist yeni validator adı için bilinçli güncellensin.
4. Eski PHP klasörü historical/reference-only diye açık işaretlensin. Canlı backend üzerine bu eski paketi kopyalama yönergesi verme. Güncel backend repo/commit ve public API contract ayrı kaydedilsin; backend değişikliği ancak gereksinim varsa ayrı PR/release olsun.
5. F-Droid belgeleri current source-available license ile tutarlı kalsın; bu patch lisans değişikliği veya F-Droid uygunluğu iddiası taşımaz.

**Regresyon:** Her artifact içinde version 0.7.3; Android upgrade aynı signing identity ve 703; QtIFW baseline/AVX2/ARM64 repository metadata 0.7.3;27 beklenen public asset; notes dosyası build öncesinde var; source ZIP yeni planlanan backend paketini yanlışlıkla üretim companion olarak içermez.

## 5. Uygulama sırası ve PR ayrımı

| Sıra | PR kapsamı | İşler | Ön koşul | Yaklaşık mühendis-gün |
| --- | --- | --- | --- | --- |
| 1 | Storage güvenliği + save koruma | 01–02 | Negatif fixture/testler | 2–3 |
| 2 | Package runtime metadata + transaction | 03–04 | 01 | 2–3 |
| 3 | Session/engine close + migration | 05–06 | 02–03 | 2–3 |
| 4 | Input/lifecycle/game timer/audio | 07–08,11 | 05 | 2–3 |
| 5 | Theme/catalog/update | 09–10,12 | 01,03–05 | 2–3 |
| 6 | Release testleri ve provenance | 13 | Aday fixture’ları | 1–2 |
| 7 | Keychain, belge/QA ve sürüm | 14–16 | Önceki PR’lar | 2–3 |

Toplam kaba tahmin: **13–20 mühendis-gün**, fiziksel cihaz erişimi ve çok platformlu build bekleme süreleri hariç. Bu takvim kodlama+test kapsamına göre tahmindir; kullanıcı sohbetlerinden yeni özellik beklentisi gelirse yeniden hesaplanır. Session coordinator mevcut API’yi koruyarak sınırlı tutulmalı; v 0.7.3 büyük mimari yeniden yazıma dönüşmemeli.

İlk uygulama branch’i `patch/v 0.7.3`; küçük PR’larda her bulgu için önce failing regression, sonra düzeltme. Bu incelemede branch/PR oluşturulmadı. Dependent işler bitmeden final version bump/release candidate commit’i oluşturulmaz.

## 6. Test ve kabul matrisi

| Katman | Zorunlu senaryolar | Kanıt |
| --- | --- | --- |
| C++ storage | Path containment, failed load/save, schema, backup, index/rename failure | Disk hash’leri + CTest çıktısı |
| Runtime/QML | Gerçek RCC yaratımı, migration, lifecycle, close reentrancy, focus | Headless Qt integration + QML warning allowlist |
| Legacy | v 0.5 canonical/prefix, v 0.6 services, modern 0.7; malicious namespace reject | Fixture ve mümkünse gerçek market package corpus |
| Network | Fake NAM: malformed/duplicate, row races, size/deadline, origin, late completion | Deterministik server/reply fixture |
| Gameplay | Memory reset/timer, Minesweeper timer, Reaction invalid round, clock/audio | Fake clock/generation + Qt smoke |
| Release |27 empty/invalid assets reject, archive contents, SHA/provenance, draft recovery | Offline tooling tests + staging API |
| Windows | Portable; fresh install;0.7.2→0.7.3 update; repair/modify/uninstall; DPI | Baseline x64; AVX2 ve ARM64 lane evidence |
| Linux | Ubuntu 22.04/24.04, Debian/Arch; X11/Wayland, temiz AppImage launch, TLS/audio | Build/CTest + clean runtime smoke |
| Android |4 ABI + universal;16KiB; signature; upgrade 703; touch/back/background | Artifact validator + arm64 fiziksel cihaz |
| Apple | macOS arm64/x64/universal; iOS/iPadOS safe area/lifecycle | Build kanıtı; signed/device QA ayrı işaretli |
| Localization/UI | EN/AZ/TR; RTL; landscape/small window; enlarged fonts | Ekran akışı ve warning çıktısı |

Mevcut Qt CI 6.10.2 ana test hedefidir. README minimum Qt 6.5 iddiası için en az configure/build/selected runtime compatibility lane ekle veya gerçek desteklenen minimumu belgeyle düzelt. Windows ARM64 şu anda BUILD_TESTING=OFF; native runner test bağımlılıkları uygunsa CTest açılır. Fiziksel test yapılamayan hücreler PASS diye işaretlenmez.

## 7. Release gate ve rollback

**Release candidate kabulü:**

- P073-01/02 giderildi; hiçbir negative storage testinde protected dosya değişmiyor.
- Zorunlu P1 işlerinin testleri geçti;06 gibi runtime-bağımlı bulgular Qt ortamında sonuçlandırıldı.
- Sekiz mevcut CTest + yeni davranış testleri Release configuration’da anlamlı assertion çalıştırıyor.
-6 built-in ve legacy corpus açılıyor; session close/bindings için yeni beklenmedik QML warning yok.
- Statik suite green; çeviri coverage eksikleri kayıtlı, yeni kritik metinler en/az/tr’de mevcut.
- Binary/source/package değişiklikleri için aynı hedef commit’te green CI ve fresh Build Release Artifacts var.
-27 tam public artifact’in içerik/size/hash/platform/version doğrulaması ve üç QtIFW repo kontrolü tamam.
- Backend download/gateway contract smoke testleri platform/arch/install/channel matrisi üzerinden yürütüldü.
- En az Windows x64 upgrade ve Android arm64 background/upgrade akışları fiziksel veya eşdeğer kontrollü runtime ortamında doğrulandı; Apple QA durumu ayrıca raporlandı.

**Yayın sırası:** Validate mode → aday kanıt/manifest incelemesi → authorized manual publish → matching draft’a verified upload → QtIFW repository publication → release public → backend cache/gateway ve clean download smoke. Yayın yetkisi mevcut protected Environment/allowlist mekanizmasından geçer; bu plan yayın işlemi yapmaz.

**Rollback:** Save format/API değişmediği için 0.7.2 ile veri uyumluluğu korunmalı; eski sürümün save bugs’ı nedeniyle rollback öncesi save backup alınmalı. Windows repository pointer için old/new SHA kaydet, CAS/lease ile geri al. Public release ortaya çıkmışsa sadece draft flag’iyle tüm transaction geri alınmış kabul etme. Android versionCode 703 kurulumunu 702 ile normal downgrade edemezsin; acil düzeltme 704 gerekir. Sorunlu sürüm için gateway önerisini durdurma ile çalışan paketleri otomatik geriletmeyi birbirine karıştırma.

## 8. Bu patch’e dahil edilmeyen işler

Save Transfer/export-import; yeni .lmgsave formatı; oyun API 0.8; yeni oyunlar; süreç tabanlı hostile-QML sandbox; yeni backend/OAuth mimarisi; lisans modeli değişikliği; UI/theme token sisteminin yeniden tasarımı; genel bağımlılık upgrade kampanyası. Bunlar v 0.7.3’ün regresyon riskini ve test alanını gereksiz büyütür. Geçmiş sohbetlerden açık bir gereksinim gelirse uygun sürüm/kapsam ayrıca kararlaştırılır.

## 9. Sohbet bağlamı geldiğinde kapatılacak eksikler

1. v 0.7.3 için önceden kararlaştırılmış kullanıcı şikâyetleri ve kesin kabul şartları.
2. İlgili diğer repo/mod paketleri ile güncel backend kaynak repo ve commit’i.
3. Built-in resume beklentisinin gerçekten mevcut ürün taahhüdü olup olmadığı.
4. Gerçek cihazlarda görülen platform/ekran/oyun/installer hata örnekleri.

Bu bilgiler gelmediği için planın kod ve canlı public contract temeli tamamlandı; sohbet geçmişiyle doğrulanmış gereksinim listesi tamamlanmış olarak sunulmuyor.
