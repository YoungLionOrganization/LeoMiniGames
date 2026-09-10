<?php
declare(strict_types=1);

if (PHP_SAPI !== 'cli') {
    fwrite(STDERR, "Run this tool from cPanel Terminal/SSH.\n");
    exit(1);
}

fwrite(STDOUT, "Enter a long random admin token: ");
$token = trim((string)fgets(STDIN));
if (strlen($token) < 24) {
    fwrite(STDERR, "Token must be at least 24 characters. Prefer 40+ random characters.\n");
    exit(1);
}
fwrite(STDOUT, password_hash($token, PASSWORD_DEFAULT) . PHP_EOL);
