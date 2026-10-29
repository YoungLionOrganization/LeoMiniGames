# LeoMiniGames kurulum rehberi

Bu rehber v0.7.3 paketlerini kapsar. Paketleri yalnızca [GitHub Releases](https://github.com/YoungLionOrganization/LeoMiniGames/releases) sayfasından indirin. Bir sürüm henüz yayımlanmamışsa Actions çıktıları test adayıdır. Kullanmak için Qt, CMake veya bir derleyici kurmanız gerekmez.

## Hangi paketi seçmeliyim?

| Sistem | Seçim | Alternatif |
| --- | --- | --- |
| Windows, Intel/AMD 64 bit | `Windows-x86_64-Setup.exe` | Kurulumsuz `.zip`; AVX2 destekli işlemciler için ayrı `x86_64-AVX2` paketi |
| Windows on ARM | `Windows-ARM64-Setup.exe` | Aynı mimarinin `.zip` paketi |
| Ubuntu 22.04/24.04, Intel/AMD 64 bit | `Linux-Ubuntu-22.04-x86_64` veya `Linux-Ubuntu-24.04-x86_64` | `.AppImage`, `.tar.gz` veya `-Setup.run` |
| Ubuntu 24.04, ARM64 | `Linux-Ubuntu-24.04-arm64` | `.AppImage`, `.tar.gz` veya `-Setup.run` |
| Debian 13 | Mimarinize göre `Debian-13-x86_64.deb` / `Debian-13-arm64.deb` | Flatpak |
| Fedora 43 | Mimarinize göre `Fedora-43-x86_64.rpm` / `Fedora-43-arm64.rpm` | Flatpak |
| Arch Linux, Intel/AMD 64 bit | `Arch-x86_64.pkg.tar.zst` | `Linux-x86_64.flatpak`; uyumlu portable paket |
| Flatpak destekli Linux | `Linux-x86_64.flatpak` veya `Linux-aarch64.flatpak` | Dağıtımınıza uygun native paket |
| macOS | Apple Silicon için `macOS-arm64`, Intel için `macOS-x86_64` | Her ikisini içeren `macOS-universal` |
| Android | Çoğu yeni cihazda `Android-arm64-v8a.apk` | Emin değilseniz daha büyük `Android-universal.apk` |

Tam adların başında `LeoMiniGames-v0.7.3-` bulunur. Linux mimarisini `uname -m` ile öğrenebilirsiniz: `x86_64` Intel/AMD, `aarch64` ARM64 anlamına gelir. Arch ARM için ayrı native paket üretilmez; ARM64 Flatpak farklı bir seçenektir. AUR veya Flathub mağazasında yayımlanmış paket iddiası yoktur; buradaki Flatpak dosyaları doğrudan indirilen bundle'lardır.

## Dosya bütünlüğü

İndirdiğiniz dosyanın SHA-256 değerini aynı release açıklamasının **SHA-256** bölümünde o dosya için verilen değerle karşılaştırın. Linux'ta örnek:

```sh
sha256sum LeoMiniGames-v0.7.3-Arch-x86_64.pkg.tar.zst
```

Windows PowerShell'de örnek:

```powershell
Get-FileHash .\LeoMiniGames-v0.7.3-Windows-x86_64-Setup.exe -Algorithm SHA256
```

## Windows

Setup dosyasını açıp kurulum konumunu ve kısayolları seçin. Mevcut kurulum bulunduğunda güncelleme, değişiklik/onarım ve kaldırma işlemleri Maintenance Tool'a yönlendirilir. Başlat menüsündeki bakım aracını daha sonra da kullanabilirsiniz.

Portable ZIP kullanıyorsanız tüm arşivi ayrı bir klasöre çıkarın ve `LeoMiniGames.exe` dosyasını açın. Yalnız EXE'yi başka klasöre taşımayın; yanındaki Qt ve çalışma zamanı dosyaları gereklidir. Baseline paket AVX2 gerektirmez. 32 bit Windows paketi üretilmez.

## Arch Linux

Native paket için güncel Arch sistemi üzerinde:

```sh
sudo pacman -Syu
sudo pacman -U ./LeoMiniGames-v0.7.3-Arch-x86_64.pkg.tar.zst
LeoMiniGames
```

Güncellerken yeni sürümün paketini aynı `pacman -U` yöntemiyle kurun. Kaldırmak için `sudo pacman -R leominigames` kullanın. Uygulama kayıtları kullanıcı dizininde kalır.

### Arch ve diğer Linux sistemlerinde Flatpak

Arch'ta Flatpak yoksa `sudo pacman -S flatpak` ile kurun. Diğer dağıtımlarda sisteminizin Flatpak paketini kullanın. İlk kurulumda KDE çalışma zamanının indirilmesi gerekir; bundle tek başına bütün runtime'ı içermez.

```sh
flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user ./LeoMiniGames-v0.7.3-Linux-x86_64.flatpak
flatpak run xyz.younglion.leominigames
```

ARM64 için dosya adında `x86_64` yerine `aarch64` seçin. Yerel bundle yükseltmesinde önce uygulamayı veri silmeden kaldırıp yeni dosyayı kurabilirsiniz:

```sh
flatpak uninstall --user xyz.younglion.leominigames
flatpak install --user ./LeoMiniGames-v0.7.3-Linux-x86_64.flatpak
```

`--delete-data` kullanmak kayıtları da siler; normal yükseltmede kullanmayın. Flatpak kayıtları `~/.var/app/xyz.younglion.leominigames/` altında tutulur. Native kurulumun verisi buraya otomatik taşınmaz. Dosya seçiminde portal penceresini kullanın.

## Debian ve Fedora

Debian 13, x86_64 örneği:

```sh
sudo apt install ./LeoMiniGames-v0.7.3-Debian-13-x86_64.deb
```

Fedora 43, x86_64 örneği:

```sh
sudo dnf install ./LeoMiniGames-v0.7.3-Fedora-43-x86_64.rpm
```

ARM64 sistemde `arm64` dosyasını seçin. Kaldırma için sırasıyla `sudo apt remove leominigames` veya `sudo dnf remove leominigames` kullanılır. DEB/RPM/Arch paketleri dağıtımın Qt kitaplıklarını kullanır; farklı dağıtım veya sürümde zorla kurmak yerine Flatpak ya da uygun portable paket seçin.

## Linux kullanıcı kurucusu, AppImage ve portable arşiv

### Setup.run

Ubuntu 24.04 x86_64 örneği:

```sh
bash ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64-Setup.run
```

Root yetkisi istemez. Varsayılan konum `~/.local/opt/LeoMiniGames` olur. Farklı bir **ayrı uygulama klasörü** için dosya adından sonra `--prefix /tam/yol/LeoMiniGames` ekleyin. Menü ve başlatıcı istemiyorsanız `--no-integration` ekleyin. Başka bir programın bulunduğu klasörü hedef seçmeyin.

Yükseltmede yeni Setup.run dosyasını aynı prefix ile çalıştırın. Başarısız yükseltme önceki uygulamayı ve kısayolları geri yükler. Geri yükleme de başarısızsa bildirilen kurtarma klasörünü saklayın.

Kaldırma:

```sh
bash ~/.local/opt/LeoMiniGames/uninstall.sh --uninstall
```

Özel prefix kullandıysanız o klasördeki `uninstall.sh` dosyasını kullanın.

### AppImage

```sh
chmod +x ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.AppImage
./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.AppImage
```

FUSE sağlanmayan sistemlerde aynı dosyayı `--appimage-extract-and-run` seçeneğiyle çalıştırabilir veya portable tar.gz seçebilirsiniz. Kaldırmak için AppImage dosyasını silmek yeterlidir; kullanıcı verisi ayrı kalır.

### Portable tar.gz

```sh
tar -xzf ./LeoMiniGames-v0.7.3-Linux-Ubuntu-24.04-x86_64.tar.gz
./LeoMiniGames.AppDir/AppRun
```

AppDir içindeki tek dosyayı taşımayın; klasörün tamamını koruyun. Adında `-native.tar.gz` bulunan arşivler dağıtımın Qt paketlerine bağlı tanılama paketleridir; genel portable paketin yerine geçmez.

## Android ve macOS

Android'de uygun APK'yi cihazın paket yükleyicisiyle açın. Aynı imza ve uygulama kimliğiyle güncelleme kayıtları korur. İmza uyuşmazlığı çıkarsa uygulamayı hemen kaldırmayın; mevcut kayıtları koruyarak doğru imzalı paketi seçin. Android için minimum sürüm Android 9 / API 28'dir. AAB doğrudan kurulmaz.

macOS'ta mimarinize uygun DMG'yi açıp uygulamayı Applications klasörüne taşıyın. ZIP alternatifi de vardır. Dağıtımda imza/noterleme durumu release notlarında belirtilir; Gatekeeper uyarısını değerlendirirken dosyanın kaynağını ve checksum'unu doğrulayın. iOS/iPadOS imzasız ZIP'leri son kullanıcı kurucusu değildir; geliştirici imzalama ortamı gerekir. HarmonyOS için v0.7.3 HAP paketi yoktur.

## Kayıtları koruma ve sorun giderme

Uygulamayı kapatarak kullanıcı veri klasörünün tamamını yedekleyin. Native Linux'ta varsayılan veri kökü `~/.local/share/YoungLion/LeoMiniGames`, Flatpak'ta aynı veri sandbox içindedir. `XDG_DATA_HOME` ayarlanmışsa native konum buna göre değişir. Paket biçimini değiştirmek verileri otomatik taşımaz; v0.7.3'te Save Transfer sihirbazı yoktur.

İlk açılışta altı yerleşik oyundan birini açıp kapatın, kaydın tekrar açıldığında korunduğunu ve katalog/tema ekranını kontrol edin. Ses yoksa sistem ses çıkışını ve uygulama ses ayarını kontrol edin. Linux keyring kapalıysa Developer Lab anahtarını kalıcı saklama çalışmayabilir; anahtarınızı hata raporuna eklemeyin.

Bir hata bildirimine işletim sistemi/sürümü, mimari, paket dosya adı, uygulama sürümü ve tekrar etme adımlarını ekleyin. Katalog sunucusunun 500 yanıtı vermesiyle yerel açılış hatası farklı sorunlardır. [Hata bildirimi](https://github.com/YoungLionOrganization/LeoMiniGames/issues/new/choose) ve [teknik Linux paket rehberi](https://github.com/YoungLionOrganization/LeoMiniGames/blob/main/docs/LINUX_PACKAGES.md) ek ayrıntı sağlar.
