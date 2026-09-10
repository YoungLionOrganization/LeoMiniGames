<?php
declare(strict_types=1);

require dirname(__DIR__) . '/app/bootstrap.php';

$method = strtoupper($_SERVER['REQUEST_METHOD'] ?? 'GET');
$path = parse_url($_SERVER['REQUEST_URI'] ?? '/', PHP_URL_PATH) ?: '/';
$path = '/' . ltrim($path, '/');

// API
$api = new ApiController($config, $pdo);

if ($method === 'GET' && $path === '/api/v1/health') {
    $api->health();
}
if ($method === 'GET' && $path === '/api/v1/mods') {
    $api->listMods();
}
if ($method === 'GET' && preg_match('#^/api/v1/mods/([^/]+)$#', $path, $m)) {
    $api->getMod(rawurldecode($m[1]));
}
if ($method === 'GET' && preg_match('#^/api/v1/mods/([^/]+)/download$#', $path, $m)) {
    $api->download(rawurldecode($m[1]));
}
if ($method === 'POST' && $path === '/api/v1/admin/upload-url') {
    $api->createUploadUrl();
}
if ($method === 'POST' && $path === '/api/v1/admin/mods') {
    $api->publishMod();
}
if ($method === 'DELETE' && preg_match('#^/api/v1/admin/mods/([^/]+)$#', $path, $m)) {
    $api->disableMod(rawurldecode($m[1]));
}

// Pages
if ($method === 'GET' && ($path === '/' || $path === '/index.html')) {
    header('Content-Type: text/html; charset=utf-8');
    readfile(__DIR__ . '/site.html');
    exit;
}
if ($method === 'GET' && ($path === '/admin' || $path === '/admin/')) {
    header('Content-Type: text/html; charset=utf-8');
    header('X-Robots-Tag: noindex, nofollow');
    readfile(__DIR__ . '/admin.html');
    exit;
}

Http::json(['error' => ['code' => 'not_found', 'message' => 'Route not found.']], 404);
