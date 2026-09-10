# LeoMiniGames Hub v1.0.2 download hotfix

This fixes HTTP 500 errors from:

`GET /api/v1/mods/{id}/download`

## cPanel update

Replace only these existing files on the deployed Hub:

- `app/ApiController.php`
- `app/R2Signer.php`

Do **not** replace `config/config.php`. It contains the live database, R2 and admin-token configuration.

The updated download endpoint:

- uses the configured public R2/custom-domain URL when one exists;
- otherwise generates a short-lived presigned R2 GET URL, so the bucket can stay private;
- does not fail the whole download if updating the download counter fails;
- returns backend version `1.0.2` from `/api/v1/health`.

After copying the files, verify:

`https://leominigames.younglion.xyz/api/v1/health`

and then:

`https://leominigames.younglion.xyz/api/v1/mods/dungeon_delver/download`

The latter should return JSON with a `data.url` HTTPS URL instead of HTTP 500.
