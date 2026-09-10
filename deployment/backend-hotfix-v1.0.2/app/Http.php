<?php
declare(strict_types=1);

final class Http
{
    public static function json(array $payload, int $status = 200): never
    {
        http_response_code($status);
        header('Content-Type: application/json; charset=utf-8');
        header('Cache-Control: no-store');
        echo json_encode($payload, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
        exit;
    }

    public static function bodyJson(): array
    {
        $raw = file_get_contents('php://input');
        if ($raw === false || trim($raw) === '') {
            return [];
        }

        try {
            $value = json_decode($raw, true, 64, JSON_THROW_ON_ERROR);
        } catch (JsonException) {
            self::json(['error' => ['code' => 'invalid_json', 'message' => 'Request body must be valid JSON.']], 400);
        }

        if (!is_array($value)) {
            self::json(['error' => ['code' => 'invalid_json', 'message' => 'JSON body must be an object.']], 400);
        }
        return $value;
    }

    public static function bearerToken(): string
    {
        $header = $_SERVER['HTTP_AUTHORIZATION'] ?? '';
        if (!preg_match('/^Bearer\s+(.+)$/i', $header, $matches)) {
            return '';
        }
        return trim($matches[1]);
    }

    public static function requireAdmin(array $config): void
    {
        $token = self::bearerToken();
        $hash = (string)($config['security']['admin_token_hash'] ?? '');
        if ($token === '' || $hash === '' || !password_verify($token, $hash)) {
            self::json(['error' => ['code' => 'unauthorized', 'message' => 'Valid admin bearer token required.']], 401);
        }
    }

    public static function noContent(int $status = 204): never
    {
        http_response_code($status);
        exit;
    }
}
