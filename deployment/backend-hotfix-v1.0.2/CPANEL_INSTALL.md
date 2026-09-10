# cPanel Installation — LeoMiniGames Hub

This guide deploys:

- website: `https://leominigames.younglion.xyz/`
- API: `https://leominigames.younglion.xyz/api/v1/...`
- private publishing page: `https://leominigames.younglion.xyz/admin`
- metadata database: cPanel MySQL/MariaDB
- mod package storage: **Cloudflare R2, not cPanel**

The backend is intentionally plain PHP 8.1+ and has **no Composer dependencies**, which makes it suitable for ordinary shared cPanel hosting.

---

# 1. Prepare Cloudflare R2 first

## 1.1 Create the bucket

In Cloudflare Dashboard:

1. Open **Storage & databases → R2 → Overview**.
2. Create a bucket named, for example:

```text
leominigames-mods
```

3. Use **Standard** storage class.

For the first tests, you may enable the generated `r2.dev` public development URL. Put that exact URL into `r2.public_base_url` later.

For public launch, use a custom domain such as:

```text
https://mods-cdn.younglion.xyz
```

Do not CNAME your own hostname to the `r2.dev` hostname. Cloudflare documents `r2.dev` as a development endpoint and rate-limits it.

## 1.2 Create R2 S3 credentials

Create an R2 API token/credential limited to this bucket with Object Read & Write access.

You need:

```text
Cloudflare Account ID
R2 Access Key ID
R2 Secret Access Key
Bucket name
Public R2 URL
```

Never put the Secret Access Key into the Qt app or JavaScript. It belongs only in the cPanel private PHP config.

## 1.3 Configure R2 CORS

The private `/admin` page uploads directly from the browser to R2, so PUT must be allowed from the website origin.

Use the included:

```text
docs/r2-cors.json
```

Allowed origin:

```text
https://leominigames.younglion.xyz
```

Allowed method:

```text
PUT
```

After CORS is configured, browser uploads do not pass through cPanel.

---

# 2. Create the cPanel subdomain/domain

Open cPanel:

```text
Domains → Create A New Domain
```

Enter:

```text
leominigames.younglion.xyz
```

Do **not** share the main site's document root.

Use a dedicated document root. A convenient layout is:

```text
/home/CPANELUSER/public_html/leominigames/public
```

The uploaded project itself will live at:

```text
/home/CPANELUSER/public_html/leominigames/
```

Only the `public/` directory must be web-accessible. `app/`, `config/`, `database/`, `tools/`, and `docs/` stay outside the subdomain document root.

If your cPanel provider chooses a slightly different directory automatically, that is fine; preserve the same relationship:

```text
project-root/
    app/
    config/
    ...
    public/   ← document root
```

Wait for DNS to resolve.

---

# 3. Enable HTTPS

Normally cPanel AutoSSL requests a certificate after the new subdomain is created.

Check:

```text
Security → SSL/TLS Status
```

Make sure `leominigames.younglion.xyz` has a valid certificate.

Do not configure the LeoMiniGames application to use plain HTTP. Mod metadata contains integrity hashes and download endpoints, so the API should be HTTPS-only.

---

# 4. Select PHP

Open:

```text
Software → MultiPHP Manager
```

Choose `leominigames.younglion.xyz`.

Select:

```text
PHP 8.2
```

or newer supported 8.x.

Required extensions:
- `pdo`
- `pdo_mysql`
- `json`
- `mbstring` (recommended; a safe fallback is included if unavailable)
- `curl`
- `openssl`

Most cPanel PHP installations already contain them.

If cURL is unavailable and you cannot enable it, set:

```php
'verify_public_object' => false,
```

in `config/config.php` only after manually confirming that direct R2 uploads work. Keeping verification enabled is preferable.

---

# 5. Create the database

Use:

```text
Databases → Database Wizard
```

Older cPanel versions may call this **MySQL Database Wizard**.

Example:

Database:

```text
leominigames
```

Database user:

```text
leomods
```

cPanel may prefix them with your account username:

```text
CPANELUSER_leominigames
CPANELUSER_leomods
```

Generate a strong random DB password.

Grant the user privileges required by the app. `ALL PRIVILEGES` on this one application database is the simplest cPanel setup.

Do not use phpMyAdmin to create the cPanel database/user mapping. Use Database Wizard first.

---

# 6. Upload the backend package

Upload the contents of this ZIP to:

```text
/home/CPANELUSER/public_html/leominigames/
```

Final layout:

```text
public_html/
└── leominigames/
    ├── app/
    ├── config/
    ├── database/
    ├── docs/
    ├── public/
    │   ├── .htaccess
    │   ├── index.php
    │   ├── site.html
    │   ├── admin.html
    │   └── assets/
    ├── tools/
    ├── CPANEL_INSTALL.md
    └── README.md
```

Confirm that the subdomain document root points specifically to:

```text
.../leominigames/public
```

not to `.../leominigames/`.

This prevents visitors from requesting your private config file.

---

# 7. Import the schema

Open:

```text
Databases → phpMyAdmin
```

Select the database you already created with Database Wizard.

Use **Import** and import:

```text
database/schema.sql
```

After import, you should see:

```text
mods
```

table.

---

# 8. Create the private config

In File Manager:

1. Open `config/`.
2. Copy:

```text
config.example.php
```

to:

```text
config.php
```

Edit `config.php`.

## Database section

Example:

```php
'database' => [
    'host' => 'localhost',
    'port' => 3306,
    'name' => 'CPANELUSER_leominigames',
    'user' => 'CPANELUSER_leomods',
    'password' => 'YOUR_DB_PASSWORD',
    'charset' => 'utf8mb4',
],
```

Use the exact names shown by cPanel, including prefixes.

## R2 section

Example:

```php
'r2' => [
    'account_id' => 'YOUR_CLOUDFLARE_ACCOUNT_ID',
    'access_key_id' => 'YOUR_R2_ACCESS_KEY_ID',
    'secret_access_key' => 'YOUR_R2_SECRET_ACCESS_KEY',
    'bucket' => 'leominigames-mods',
    'public_base_url' => 'https://YOUR_BUCKET_PUBLIC_URL',
    'presign_expiry_seconds' => 900,
],
```

Initial testing can use the Cloudflare `r2.dev` URL.

Before public launch, prefer an R2 custom domain such as:

```text
https://mods-cdn.younglion.xyz
```

---

# 9. Generate the private publisher token

If cPanel has Terminal:

```text
Advanced → Terminal
```

Go to the project:

```bash
cd ~/public_html/leominigames
```

Run:

```bash
php tools/generate_token.php
```

It prints:

1. a raw 64-byte hexadecimal token
2. a password hash

Save the **raw token** in your password manager. You enter this into the private `/admin` page when publishing.

Put only the **hash** in `config/config.php`:

```php
'security' => [
    'admin_token_hash' => '$2y$...',
],
```

Never put the raw token into a public JavaScript file, Git repository, or the Qt application.

If Terminal is unavailable, you can generate a long random secret elsewhere and use `password_hash()` from any local PHP 8 installation.

After `config/config.php` is ready, run:

```bash
php tools/self_check.php
```

Fix every `[FAIL]`. `[WARN]` for cURL/mbstring is non-fatal, although cURL is recommended while `verify_public_object` is enabled.

---

# 10. File permissions

Typical shared hosting defaults:

Directories:

```text
755
```

Public PHP/HTML/CSS files:

```text
644
```

Private config:

```text
600
```

or `640` if your hosting setup requires the web server group to read it.

Do not use `777`.

---

# 11. Test the API

Open:

```text
https://leominigames.younglion.xyz/api/v1/health
```

Expected response:

```json
{
  "data": {
    "service": "LeoMiniGames Hub API",
    "version": "1.0.0",
    "status": "ok"
  }
}
```

Then:

```text
https://leominigames.younglion.xyz/api/v1/mods
```

Before publishing anything, `data` should be an empty array.

If you get a 500/503:
- check `config/config.php`
- check DB name/user prefixes
- check PHP error logs in cPanel
- check that PDO MySQL is enabled

If every URL returns 404:
- confirm `.htaccess` was uploaded
- confirm Apache `mod_rewrite` is available
- confirm the domain document root is the `public/` directory

---

# 12. Publish the first mod

Open:

```text
https://leominigames.younglion.xyz/admin
```

This page is deliberately not linked from the public homepage.

Enter the raw admin token.

Fill:
- ID
- version
- name
- author
- category
- description
- minimum LeoMiniGames version
- `Main.qml` or another safe relative entry path
- choose the `.rcc` file

Press:

```text
Upload to R2 & Publish
```

Flow:

```text
browser
  │
  ├─ SHA-256 locally
  │
  ├─ POST metadata → cPanel API
  │                    │
  │                    └─ creates a short-lived signed R2 PUT URL
  │
  ├─ PUT .rcc ──────────────────────────────→ Cloudflare R2
  │
  └─ POST final metadata → cPanel/MySQL
```

At no point is the RCC package permanently written to cPanel disk.

After publishing, refresh:

```text
https://leominigames.younglion.xyz/
```

and the LeoMiniGames app's **Mods** screen.

---

# 13. Build an RCC mod

The Qt application ZIP includes:

```text
mod-sdk/ExampleHelloMod/
```

On Windows with your current Qt installation:

```bat
build_mod.bat "D:\Qt\6.11.1\llvm-mingw_64\bin\rcc.exe"
```

Equivalent command:

```bat
"D:\Qt\6.11.1\llvm-mingw_64\bin\rcc.exe" -binary mod.qrc -o hello_mod-1.0.0.rcc
```

The generated RCC is architecture-independent resource data. The LeoMiniGames app downloads it, verifies SHA-256, registers it with `QResource::registerResource()`, then loads its QML entry.

---

# 14. Recommended DNS/R2 launch path

## Stage A — development

Website/API:

```text
leominigames.younglion.xyz → cPanel
```

R2 files:

```text
pub-....r2.dev
```

This avoids changing your DNS provider while you test.

## Stage B — public release

Keep:

```text
leominigames.younglion.xyz → cPanel
```

Use:

```text
mods-cdn.younglion.xyz → Cloudflare R2 custom domain
```

Cloudflare requires the custom domain/zone to be connected to the R2 account. If `younglion.xyz` currently uses cPanel nameservers and you decide to move authoritative DNS to Cloudflare, copy **all** existing DNS records first — website A/AAAA, cPanel-related records, MX, SPF, DKIM, DMARC, verification TXT records — before changing nameservers.

Do not casually change nameservers if email is active.

---

# 15. Backups

cPanel does not contain mod package files, but it does contain metadata.

Back up:
- MySQL database
- `config/config.php` securely
- project code

R2 should be considered the source for actual `.rcc` packages.

A DB backup is enough to restore titles/descriptions/hashes/download counts, while R2 preserves package bytes.

---

# 16. Security model in this version

There are no user accounts and no public publishing.

Only a holder of the admin bearer token can:
- request R2 upload URLs
- publish/replace metadata
- disable mods

Public users can:
- list published mods
- fetch mod details
- obtain download URLs

The Qt app additionally:
- accepts only `rcc-v1`
- rejects packages over 64 MiB
- validates IDs and entry paths
- verifies SHA-256 before installation
- registers only the downloaded RCC after verification
- exposes only published catalog entries

Before opening publishing to everyone later, add:
- real accounts/authentication
- author ownership
- malware/manual review pipeline
- moderation
- package signing with a public-key signature
- abuse/rate limits
- audit logs
