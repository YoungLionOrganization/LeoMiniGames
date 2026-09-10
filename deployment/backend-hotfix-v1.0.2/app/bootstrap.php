<?php
declare(strict_types=1);

$root = dirname(__DIR__);
$configFile = $root . '/config/config.php';
if (!is_file($configFile)) {
    http_response_code(500);
    header('Content-Type: text/plain; charset=utf-8');
    exit("LeoMiniGames Hub is not configured. Copy config/config.example.php to config/config.php.\n");
}

$config = require $configFile;

require_once $root . '/app/Http.php';
require_once $root . '/app/Database.php';
require_once $root . '/app/R2Signer.php';
require_once $root . '/app/Validation.php';
require_once $root . '/app/ModRepository.php';
require_once $root . '/app/ApiController.php';

try {
    $pdo = Database::connect($config);
} catch (Throwable $e) {
    error_log('LeoMiniGames Hub DB connection failed: ' . $e->getMessage());
    Http::json(['error' => ['code' => 'database_unavailable', 'message' => 'Database connection failed.']], 503);
}
