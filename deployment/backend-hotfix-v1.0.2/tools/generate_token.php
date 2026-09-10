<?php
declare(strict_types=1);

if (PHP_SAPI !== 'cli') {
    exit(1);
}
$token = bin2hex(random_bytes(32));
echo "RAW ADMIN TOKEN (save it somewhere safe):\n{$token}\n\n";
echo "HASH FOR config/config.php:\n" . password_hash($token, PASSWORD_DEFAULT) . "\n";
