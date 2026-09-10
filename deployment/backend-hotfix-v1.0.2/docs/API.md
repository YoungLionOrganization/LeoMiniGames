# LeoMiniGames Hub API v1

Base:

```text
https://leominigames.younglion.xyz/api/v1
```

## Public

### GET `/health`

Returns service health.

### GET `/mods`

Query parameters:
- `q`
- `category`
- `sort=updated|downloads|name`
- `page`
- `limit` (1–100)

Response:

```json
{
  "data": [
    {
      "id": "hello_mod",
      "name": "Hello Mod",
      "description": "Example",
      "author": "YoungLion",
      "version": "1.0.0",
      "category": "Game",
      "icon": "MOD",
      "min_app_version": "0.4.0",
      "package_format": "rcc-v1",
      "entry": "Main.qml",
      "sha256": "...",
      "size_bytes": 12345,
      "downloads": 12,
      "updated_at": "2026-08-25 13:00:00",
      "download_endpoint": "/api/v1/mods/hello_mod/download"
    }
  ],
  "meta": {
    "page": 1,
    "limit": 30,
    "total": 1,
    "pages": 1
  }
}
```

### GET `/mods/{id}`

Returns one published mod.

### GET `/mods/{id}/download`

Increments the download counter and returns the current R2 URL plus integrity metadata.

```json
{
  "data": {
    "id": "hello_mod",
    "version": "1.0.0",
    "url": "https://...",
    "sha256": "...",
    "size_bytes": 12345
  }
}
```

## Private publisher API

All private routes require:

```http
Authorization: Bearer YOUR_RAW_ADMIN_TOKEN
```

The server stores only a password hash of the admin token.

### POST `/admin/upload-url`

JSON:

```json
{
  "id": "hello_mod",
  "version": "1.0.0",
  "sha256": "<64 hex chars>",
  "size_bytes": 12345
}
```

Returns a short-lived R2 presigned PUT URL. Upload the `.rcc` directly to that URL.

### POST `/admin/mods`

Finalizes/publishes metadata after the direct R2 upload.

### DELETE `/admin/mods/{id}`

Disables a mod from the public catalog. It does not delete the R2 object.

## Package policy

Current online format: `rcc-v1`.

Safety limits:
- 64 MiB per package
- SHA-256 required
- relative `.qml` entry only
- package must be publisher-reviewed
- no public upload account system in v1
