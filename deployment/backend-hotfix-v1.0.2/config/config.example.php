<?php
declare(strict_types=1);

return [
    'app' => [
        'base_url' => 'https://leominigames.younglion.xyz',
        'environment' => 'production',
        'max_mod_bytes' => 67108864, // 64 MiB; must match the app safety limit.
        'verify_public_object' => true,
    ],

    'database' => [
        'host' => 'localhost',
        'port' => 3306,
        'name' => 'CPANELUSER_leominigames',
        'user' => 'CPANELUSER_leomods',
        'password' => 'CHANGE_ME',
        'charset' => 'utf8mb4',
    ],

    'r2' => [
        'account_id' => 'CLOUDFLARE_ACCOUNT_ID',
        'access_key_id' => 'R2_ACCESS_KEY_ID',
        'secret_access_key' => 'R2_SECRET_ACCESS_KEY',
        'bucket' => 'younglion-leominigames-mods',

        // For early testing this can be your https://pub-....r2.dev URL.
        // Before public launch, use an R2 custom domain, e.g.
        // https://mods-cdn.younglion.xyz
        'public_base_url' => 'https://YOUR_PUBLIC_R2_HOST',
        'presign_expiry_seconds' => 900,
    ],

    'security' => [
        // Generate with: php tools/hash_token.php
        // Paste only the PASSWORD HASH here, never the raw admin token.
        'admin_token_hash' => '$2y$12$REPLACE_THIS_WITH_A_REAL_PASSWORD_HASH',
    ],
];
