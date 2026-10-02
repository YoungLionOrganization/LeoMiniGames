# v0.7.3 tema ve ses ek incelemesi

1 Ekim 2026. Önceki iki patch planı ve builtin/TLS değişikliklerinin üzerine gelir. Oyun/mod API 0.7 ve native plugin ABI 1.0 korunur. Yeni güvenlik araştırması yapılmadı.

## Ses: mevcut API ve giderilen sorunlar

Tek host API zaten mevcuttu: modern `GameAudio.playEffect`/`playMusic`; eski `Audio.play`/`playUrl`/`preload` korunuyordu. İnceleme şu eksikleri gösterdi:

- Bütün efektler yalnız `QSoundEffect` yolundaydı; WAV dışı formatlar ve reddedilen WAV için fallback yoktu. RCC kaynağı doğrudan decoder'a verilerek native backend'lerin kaynak erişim farklarına bırakılıyordu.
- Master ses seviyesi değişince önceden uygulanan gain/grup seviyesi kayboluyordu; grup değişikliği aktif efektleri güncellemiyordu. Legacy named `playUrl(url,gain)` gain'i kullanmıyordu.
- `pauseAll` yalnız müziği duraklatıyordu. Efektlerin yükleme sonrasında background'da başlaması ve legacy efektlerin sürmesi engellenmiyordu. Manuel lifecycle pause/resume doğrudan sese bağlı değildi; launcher'da açık oyun yokken background bildirimi atlanıyordu.
- Müzik decoder hatası oyun API/host teşhisine aktarılmıyordu. `capabilities()` müzik/grupları belirtmiyor; Multimedia bulunmayan build normal bir ses build'i gibi sessizce ilerleyebiliyordu.

`AudioClip` launcher tarafından yönetilen ortak oynatma katmanıdır. RCC sesi decoder ömrüne bağlı, boyutu sınırlı geçici yerel dosyaya taşınır. WAV önce düşük gecikmeli QSoundEffect kullanır; reddedilirse aynı istek QMediaPlayer'a geçer. Diğer formatlar ve müzik media backend'ini kullanır. Modern/legacy efektler aynı 32 öğelik cache ve kaynak/gain yolunu paylaşır. Decoder dosya tanıtıcıları geçici dosyalar silinmeden kapatılır; paket unload/close aktif sesleri ve package cache'ini temizler.

Master × group × gain saklanır ve ayar değişikliğinde tekrar uygulanır. Mute/pause pending efektleri iptal eder; background çağrıları ses başlatmaz; foreground eski efektleri tekrar çalmaz. Müzik pozisyonu/bekleyen isteği tekrar eden pause/resume sırasında korunur; pause durumunda yeni müzik de foreground'u bekler. Efekt/müzik hataları `GameAudio.audioError` ve host diagnostics'e gider. Eski overloadlar ve named sesler korunur. URL path'i doğru bileşen API'siyle çözülür; encoded asset adlarında `#` karakteri fragment'e dönüşmez ve Qt FullyDecoded uyarısı üretilmez.

Normal build Qt Multimedia'yı zorunlu ister; kasıtlı `LEOMINIGAMES_ENABLE_AUDIO=OFF` test/headless build'i available=false ve boş playback capabilities döndürür. Android her ABI'de, Windows/Linux/macOS staged ve archive payload'larında gerçek media backend plugin'i arar. Bu dosya kontrolü fiziksel ses çıkışı veya bütün codec'lerin çalışması kanıtı değildir.

## Tema düzeltmeleri

- Relative asset değerleri token/alias sorgusunda RCC root'una çevrilmiyordu; şimdi owning theme root'u kullanılır.
- Kısmi surface override bütün default surface'i kaldırıyordu; radius/border ve nested gradient stop'ları artık default'tan tamamlanır. Her temanın asset root'u ayrı çözülür.
- Alias cycle ve inherited alias üzerinden cycle kurulumu reddedilir. Theme API tipi, sürüm ve alias/surface map tipleri yükleme/replace öncesi denetlenir. Eski eksik API/sürüm alanları varsayılanlarla çalışır.
- Tema SDK'da izin verilen font dosyalarının otomatik font kaydı henüz yoktur; mevcut destek gibi sunulmaz, ilerisi için not edilmiştir.

## Test kanıtı ve bekleyen işler

Linux x86_64 Release, Qt 6.8.2, FFmpeg backend:

- 18/18 CTest geçti; son ses değişikliklerinin ardından ilgili ses/tema testleri de geçti. Ses testi 31 davranış assertion'ı içerir.
- Yeni ses testi gerçek RCC dosyasını kullanır; QSoundEffect'in reddettiği geçerli Ogg stream'ini QMediaPlayer'a geçirir ve gerçek PCM buffer çıktısı ölçer; tekrar oynatmayı da doğrular. Gain/grup/master, legacy çağrılar, pause/mute, async müzik/resume, unload/geçici dosya ve 32 effect cache sınırı test edilir.
- Multimedia açıkça kapatılarak ayrı Release build ve ses testi geçti; unavailable/boş capabilities/hata raporu doğrulandı.
- 13/13 Python paket testi; Android eksik media backend ve desktop payload media backend kontrolleri dahil. Tema testleri relative/aliased asset, partial/nested surface, cycle/API/version reddi ve active removal fallback'i kapsar.
- Source/project/release/distribution ve Bash/Python kontrolleri geçer. Distribution 293 PASS, 0 ERROR.

Bu test fiziksel hoparlörden sesi ölçmez. Yeni APK/Windows/macOS/Apple mobile paketleri bu makinede çalıştırılmadı. Her platformda PCM WAV + compressed effect + looping music; pause/background/foreground, mute/group/master, unload, Bluetooth/headphone yönlendirmesi ve OS interruption testi yapılmalıdır. Önceki GitHub push 403 engeli nedeniyle yeni aday CI/build'i uzakta başlamadı.

İleriye bırakılan host özellikleri: aynı effect için eşzamanlı sınırlı voice pool (mevcut davranış yeniden başlatmadır), seçilebilir output device, açık mobil audio focus/ducking politikası, desteklenmeyen formatların evrensel transcoding'i, tema fontlarının otomatik kaydı. Geliştiricinin platform bazlı QML yamaları yazması ortak API'nin önerilen yolu değildir.

Detaylı geliştirici sözleşmesi: [AUDIO_API.md](../sdk/AUDIO_API.md), [THEMING.md](../THEMING.md).
