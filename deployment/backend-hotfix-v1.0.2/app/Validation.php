<?php
declare(strict_types=1);

final class Validation
{
    public static function modId(string $value): string
    {
        $value = strtolower(trim($value));
        if (!preg_match('/^[a-z0-9][a-z0-9_.-]{1,63}$/', $value)) {
            Http::json(['error' => ['code' => 'invalid_id', 'message' => 'Invalid mod id.']], 422);
        }
        return $value;
    }

    public static function version(string $value): string
    {
        $value = trim($value);
        if (!preg_match('/^[0-9]+(?:\.[0-9]+){0,3}(?:[-+][A-Za-z0-9.-]+)?$/', $value)) {
            Http::json(['error' => ['code' => 'invalid_version', 'message' => 'Invalid version.']], 422);
        }
        return $value;
    }

    public static function sha256(string $value): string
    {
        $value = strtolower(trim($value));
        if (!preg_match('/^[a-f0-9]{64}$/', $value)) {
            Http::json(['error' => ['code' => 'invalid_sha256', 'message' => 'SHA-256 must be 64 lowercase hex characters.']], 422);
        }
        return $value;
    }

    public static function entry(string $value): string
    {
        $value = trim($value);
        if ($value === '' || str_starts_with($value, '/') || str_contains($value, '..') || !str_ends_with($value, '.qml')) {
            Http::json(['error' => ['code' => 'invalid_entry', 'message' => 'Entry must be a safe relative .qml path.']], 422);
        }
        return $value;
    }

    public static function text(mixed $value, int $max, string $field, bool $allowEmpty = false): string
    {
        $text = trim((string)$value);
        $length = function_exists('mb_strlen') ? mb_strlen($text, 'UTF-8') : strlen($text);
        if ((!$allowEmpty && $text === '') || $length > $max) {
            Http::json(['error' => ['code' => 'invalid_' . $field, 'message' => "Invalid {$field}."]], 422);
        }
        return $text;
    }
}
