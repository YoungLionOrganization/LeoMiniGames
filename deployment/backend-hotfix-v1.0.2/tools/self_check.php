<?php
declare(strict_types=1);

$root = dirname(__DIR__);
$checks = [
    'PHP >= 8.1' => version_compare(PHP_VERSION, '8.1.0', '>='),
    'PDO extension' => extension_loaded('pdo'),
    'PDO MySQL extension' => extension_loaded('pdo_mysql'),
    'JSON extension' => extension_loaded('json'),
    'OpenSSL extension' => extension_loaded('openssl'),
    'cURL extension (recommended)' => extension_loaded('curl'),
    'mbstring extension (recommended)' => extension_loaded('mbstring'),
    'public/.htaccess exists' => is_file($root . '/public/.htaccess'),
    'database/schema.sql exists' => is_file($root . '/database/schema.sql'),
    'config/config.php exists' => is_file($root . '/config/config.php'),
];

$failedRequired = false;
foreach ($checks as $label => $ok) {
    $required = !str_contains($label, '(recommended)');
    printf("[%s] %s\n", $ok ? 'OK' : ($required ? 'FAIL' : 'WARN'), $label);
    if (!$ok && $required) {
        $failedRequired = true;
    }
}

if (is_file($root . '/config/config.php')) {
    $config = require $root . '/config/config.php';
    $placeholders = [
        'DB password' => (string)($config['database']['password'] ?? ''),
        'R2 account ID' => (string)($config['r2']['account_id'] ?? ''),
        'R2 access key' => (string)($config['r2']['access_key_id'] ?? ''),
        'R2 secret key' => (string)($config['r2']['secret_access_key'] ?? ''),
        'R2 public URL' => (string)($config['r2']['public_base_url'] ?? ''),
        'Admin token hash' => (string)($config['security']['admin_token_hash'] ?? ''),
    ];
    foreach ($placeholders as $label => $value) {
        $looksPlaceholder = $value === '' || str_contains($value, 'CHANGE_ME') || str_contains($value, 'CLOUDFLARE_') || str_contains($value, 'R2_') || str_contains($value, 'YOUR_') || str_contains($value, 'REPLACE_THIS');
        printf("[%s] %s configured\n", $looksPlaceholder ? 'WARN' : 'OK', $label);
    }
}

exit($failedRequired ? 1 : 0);
