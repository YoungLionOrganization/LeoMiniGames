# LeoMiniGames Hub — cPanel backend v1.0

Companion backend/frontend for LeoMiniGames v0.4.0.

Target host:

```text
https://leominigames.younglion.xyz
```

## What lives on cPanel

- responsive public mod catalog frontend
- private publisher page (`/admin`)
- PHP API (`/api/v1`)
- MySQL/MariaDB mod metadata
- R2 presigned-upload signer

## What does NOT live on cPanel

The `.rcc` mod packages themselves. The publisher browser uploads them directly to Cloudflare R2 using a short-lived signed PUT URL.

## Authentication model

There are no user accounts yet. Public users have read-only catalog/download-ticket access. Publishing and disabling mods requires one private Bearer publisher token whose password hash is stored in `config/config.php`.

Do not give mod authors the publisher token. Authors build an RCC package with the SDK; the publisher reviews and publishes it.

## API

Public:

```text
GET /api/v1/health
GET /api/v1/mods
GET /api/v1/mods/{id}
GET /api/v1/mods/{id}/download
```

Private publisher:

```text
POST   /api/v1/admin/upload-url
POST   /api/v1/admin/mods
DELETE /api/v1/admin/mods/{id}
```

See `docs/API.md` for payloads.

## Requirements

- PHP 8.1+
- PDO + PDO MySQL
- JSON
- OpenSSL
- cURL recommended (used to verify published R2 objects)
- MySQL/MariaDB
- Apache rewrite support
- Cloudflare R2 bucket + S3 credentials

No Composer packages or Node.js are required on cPanel.

## Install

Read `CPANEL_INSTALL.md` from start to finish. After creating `config/config.php`, run:

```bash
php tools/self_check.php
```


## v1.0.1 R2 browser-upload hotfix
Presigned PUT URLs use the virtual-hosted R2 form `https://<bucket>.<account>.r2.cloudflarestorage.com/<key>`. This lets R2 resolve the bucket during browser OPTIONS preflight and apply the bucket CORS policy. The publisher explicitly sends `Content-Type: application/octet-stream`.
