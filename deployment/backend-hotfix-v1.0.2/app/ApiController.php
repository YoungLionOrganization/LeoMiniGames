<?php
declare(strict_types=1);

final class ApiController
{
    private readonly ModRepository $mods;
    private readonly R2Signer $r2;

    public function __construct(
        private readonly array $config,
        PDO $pdo
    ) {
        $this->mods = new ModRepository($pdo);
        $r2 = $config['r2'];
        $this->r2 = new R2Signer(
            $r2['account_id'],
            $r2['access_key_id'],
            $r2['secret_access_key'],
            $r2['bucket']
        );
    }

    public function health(): never
    {
        Http::json([
            'data' => [
                'service' => 'LeoMiniGames Hub API',
                'version' => '1.0.2',
                'status' => 'ok',
                'time' => gmdate(DATE_ATOM),
            ],
        ]);
    }

    public function listMods(): never
    {
        $query = trim((string)($_GET['q'] ?? ''));
        $category = trim((string)($_GET['category'] ?? ''));
        $sort = (string)($_GET['sort'] ?? 'updated');
        $limit = max(1, min(100, (int)($_GET['limit'] ?? 30)));
        $page = max(1, (int)($_GET['page'] ?? 1));
        $offset = ($page - 1) * $limit;

        $rows = $this->mods->listPublished($query, $category, $sort, $limit, $offset);
        $total = $this->mods->countPublished($query, $category);
        $data = array_map(fn(array $row): array => $this->publicMod($row), $rows);

        Http::json([
            'data' => $data,
            'meta' => [
                'page' => $page,
                'limit' => $limit,
                'total' => $total,
                'pages' => max(1, (int)ceil($total / $limit)),
            ],
        ]);
    }

    public function getMod(string $id): never
    {
        $id = Validation::modId($id);
        $row = $this->mods->findPublished($id);
        if (!$row) {
            Http::json(['error' => ['code' => 'not_found', 'message' => 'Mod not found.']], 404);
        }
        Http::json(['data' => $this->publicMod($row)]);
    }

    public function download(string $id): never
    {
        $id = Validation::modId($id);
        $row = $this->mods->findPublished($id);
        if (!$row) {
            Http::json(['error' => ['code' => 'not_found', 'message' => 'Mod not found.']], 404);
        }

        // Download counters must never make a valid download unavailable.
        try {
            $this->mods->incrementDownloads($id);
        } catch (Throwable $e) {
            error_log('LeoMiniGames Hub download counter update failed for ' . $id . ': ' . $e->getMessage());
        }
        $url = $this->downloadUrl($row);

        Http::json([
            'data' => [
                'id' => $row['id'],
                'version' => $row['version'],
                'url' => $url,
                'sha256' => $row['sha256'],
                'size_bytes' => (int)$row['size_bytes'],
            ],
        ]);
    }

    public function createUploadUrl(): never
    {
        Http::requireAdmin($this->config);
        $body = Http::bodyJson();

        $id = Validation::modId((string)($body['id'] ?? ''));
        $version = Validation::version((string)($body['version'] ?? ''));
        $sha256 = Validation::sha256((string)($body['sha256'] ?? ''));
        $size = (int)($body['size_bytes'] ?? 0);
        $max = (int)$this->config['app']['max_mod_bytes'];

        if ($size <= 0 || $size > $max) {
            Http::json(['error' => ['code' => 'invalid_size', 'message' => 'Mod package size is invalid or too large.']], 422);
        }

        $objectKey = "mods/{$id}/{$version}/{$id}-{$version}.rcc";
        $expires = (int)($this->config['r2']['presign_expiry_seconds'] ?? 900);

        Http::json([
            'data' => [
                'upload_url' => $this->r2->presignPut($objectKey, $expires),
                'object_key' => $objectKey,
                'sha256' => $sha256,
                'expires_in' => $expires,
                'method' => 'PUT',
            ],
        ]);
    }

    public function publishMod(): never
    {
        Http::requireAdmin($this->config);
        $body = Http::bodyJson();

        $mod = [
            'id' => Validation::modId((string)($body['id'] ?? '')),
            'name' => Validation::text($body['name'] ?? '', 120, 'name'),
            'description' => Validation::text($body['description'] ?? '', 3000, 'description'),
            'author' => Validation::text($body['author'] ?? '', 120, 'author'),
            'version' => Validation::version((string)($body['version'] ?? '')),
            'category' => Validation::text($body['category'] ?? 'Mod', 64, 'category'),
            'icon' => Validation::text($body['icon'] ?? 'MOD', 16, 'icon'),
            'min_app_version' => Validation::version((string)($body['min_app_version'] ?? '0.4.0')),
            'package_format' => (string)($body['package_format'] ?? 'rcc-v1'),
            'entry' => Validation::entry((string)($body['entry'] ?? 'Main.qml')),
            'object_key' => Validation::text($body['object_key'] ?? '', 512, 'object_key'),
            'sha256' => Validation::sha256((string)($body['sha256'] ?? '')),
            'size_bytes' => (int)($body['size_bytes'] ?? 0),
        ];

        if ($mod['package_format'] !== 'rcc-v1') {
            Http::json(['error' => ['code' => 'invalid_package_format', 'message' => 'Only rcc-v1 is accepted.']], 422);
        }

        $expectedPrefix = "mods/{$mod['id']}/{$mod['version']}/";
        if (!str_starts_with($mod['object_key'], $expectedPrefix)) {
            Http::json(['error' => ['code' => 'invalid_object_key', 'message' => 'Object key does not match mod id/version.']], 422);
        }

        $max = (int)$this->config['app']['max_mod_bytes'];
        if ($mod['size_bytes'] <= 0 || $mod['size_bytes'] > $max) {
            Http::json(['error' => ['code' => 'invalid_size', 'message' => 'Invalid mod package size.']], 422);
        }

        if (($this->config['app']['verify_public_object'] ?? true) === true) {
            $publicBase = $this->configuredPublicBaseUrl();
            if ($publicBase !== '' && !$this->publicObjectExists($this->r2->publicUrl($publicBase, $mod['object_key']), $mod['size_bytes'])) {
                Http::json([
                    'error' => [
                        'code' => 'r2_object_unavailable',
                        'message' => 'Uploaded R2 object could not be verified through the public download URL.',
                    ],
                ], 422);
            }
        }

        $this->mods->upsert($mod);
        $row = $this->mods->findPublished($mod['id']);
        Http::json(['data' => $this->publicMod($row ?: $mod)], 201);
    }

    public function disableMod(string $id): never
    {
        Http::requireAdmin($this->config);
        $id = Validation::modId($id);
        if (!$this->mods->disable($id)) {
            Http::json(['error' => ['code' => 'not_found', 'message' => 'Mod not found.']], 404);
        }
        Http::noContent();
    }

    private function publicMod(array $row): array
    {
        $id = (string)$row['id'];
        return [
            'id' => $id,
            'name' => (string)$row['name'],
            'description' => (string)$row['description'],
            'author' => (string)$row['author'],
            'version' => (string)$row['version'],
            'category' => (string)$row['category'],
            'icon' => (string)$row['icon'],
            'min_app_version' => (string)$row['min_app_version'],
            'package_format' => (string)$row['package_format'],
            'entry' => (string)($row['entry_path'] ?? $row['entry'] ?? 'Main.qml'),
            'sha256' => (string)$row['sha256'],
            'size_bytes' => (int)$row['size_bytes'],
            'downloads' => (int)($row['downloads'] ?? 0),
            'updated_at' => (string)($row['updated_at'] ?? gmdate(DATE_ATOM)),
            'download_endpoint' => '/api/v1/mods/' . rawurlencode($id) . '/download',
            // Direct URL lets v0.5.0+ clients download even if the counter/ticket route is temporarily unavailable.
            'download_url' => $this->publicDownloadUrl($row),
        ];
    }

    private function configuredPublicBaseUrl(): string
    {
        $base = trim((string)($this->config['r2']['public_base_url'] ?? ''));
        if ($base === '' || !str_starts_with($base, 'https://') || str_contains($base, 'YOUR_PUBLIC_R2_HOST')) {
            return '';
        }
        return rtrim($base, '/');
    }

    private function publicDownloadUrl(array $row): string
    {
        $base = $this->configuredPublicBaseUrl();
        if ($base === '') {
            return '';
        }
        return $this->r2->publicUrl($base, (string)$row['object_key']);
    }

    private function downloadUrl(array $row): string
    {
        $publicUrl = $this->publicDownloadUrl($row);
        if ($publicUrl !== '') {
            return $publicUrl;
        }

        // Keep the R2 bucket private if no public domain/r2.dev URL is configured.
        // Native LeoMiniGames clients can download this short-lived signed GET URL directly.
        $expires = (int)($this->config['r2']['download_presign_expiry_seconds']
            ?? $this->config['r2']['presign_expiry_seconds']
            ?? 900);
        return $this->r2->presignGet((string)$row['object_key'], $expires);
    }

    private function publicObjectExists(string $url, int $expectedSize): bool
    {
        if (!function_exists('curl_init')) {
            // If your host lacks cURL, disable verify_public_object only after confirming uploads manually.
            return false;
        }

        $ch = curl_init($url);
        curl_setopt_array($ch, [
            CURLOPT_NOBODY => true,
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_TIMEOUT => 12,
            CURLOPT_CONNECTTIMEOUT => 5,
            CURLOPT_USERAGENT => 'LeoMiniGamesHub/1.0',
        ]);
        curl_exec($ch);
        $status = (int)curl_getinfo($ch, CURLINFO_RESPONSE_CODE);
        if (defined('CURLINFO_CONTENT_LENGTH_DOWNLOAD_T')) {
            $length = (int)curl_getinfo($ch, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T);
        } else {
            $length = (int)curl_getinfo($ch, CURLINFO_CONTENT_LENGTH_DOWNLOAD);
        }
        curl_close($ch);

        if ($status < 200 || $status >= 400) {
            return false;
        }
        return $length <= 0 || $length === $expectedSize;
    }
}
