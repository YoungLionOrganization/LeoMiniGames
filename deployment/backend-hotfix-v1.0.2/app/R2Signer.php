<?php
declare(strict_types=1);

final class R2Signer
{
    public function __construct(
        private readonly string $accountId,
        private readonly string $accessKeyId,
        private readonly string $secretAccessKey,
        private readonly string $bucket
    ) {
    }

    public function presignPut(string $objectKey, int $expires = 900): string
    {
        return $this->presign('PUT', $objectKey, $expires);
    }

    public function presignGet(string $objectKey, int $expires = 900): string
    {
        return $this->presign('GET', $objectKey, $expires);
    }

    public function publicUrl(string $publicBaseUrl, string $objectKey): string
    {
        return rtrim($publicBaseUrl, '/') . '/' . $this->encodePath($objectKey);
    }

    private function presign(string $method, string $objectKey, int $expires): string
    {
        $expires = max(1, min(604800, $expires));
        $method = strtoupper($method);
        if ($method !== 'GET' && $method !== 'PUT') {
            throw new InvalidArgumentException('Unsupported R2 presign method.');
        }

        $host = $this->bucket . '.' . $this->accountId . '.r2.cloudflarestorage.com';
        $date = gmdate('Ymd');
        $amzDate = gmdate('Ymd\THis\Z');
        $region = 'auto';
        $service = 's3';
        $scope = $date . '/' . $region . '/' . $service . '/aws4_request';

        $canonicalUri = '/' . $this->encodePath($objectKey);
        $params = [
            'X-Amz-Algorithm' => 'AWS4-HMAC-SHA256',
            'X-Amz-Credential' => $this->accessKeyId . '/' . $scope,
            'X-Amz-Date' => $amzDate,
            'X-Amz-Expires' => (string)$expires,
            'X-Amz-SignedHeaders' => 'host',
        ];
        ksort($params, SORT_STRING);
        $canonicalQuery = $this->query($params);

        $canonicalHeaders = 'host:' . $host . "\n";
        $canonicalRequest =
            $method . "\n" .
            $canonicalUri . "\n" .
            $canonicalQuery . "\n" .
            $canonicalHeaders . "\n" .
            "host\n" .
            "UNSIGNED-PAYLOAD";

        $stringToSign =
            "AWS4-HMAC-SHA256\n" .
            $amzDate . "\n" .
            $scope . "\n" .
            hash('sha256', $canonicalRequest);

        $kDate = hash_hmac('sha256', $date, 'AWS4' . $this->secretAccessKey, true);
        $kRegion = hash_hmac('sha256', $region, $kDate, true);
        $kService = hash_hmac('sha256', $service, $kRegion, true);
        $kSigning = hash_hmac('sha256', 'aws4_request', $kService, true);
        $signature = hash_hmac('sha256', $stringToSign, $kSigning);

        return 'https://' . $host . $canonicalUri . '?' .
            $canonicalQuery . '&X-Amz-Signature=' . $signature;
    }

    private function encodePath(string $path): string
    {
        $parts = explode('/', $path);
        return implode('/', array_map(static fn(string $part): string => rawurlencode($part), $parts));
    }

    private function query(array $params): string
    {
        $pairs = [];
        foreach ($params as $key => $value) {
            $pairs[] = rawurlencode((string)$key) . '=' . rawurlencode((string)$value);
        }
        return implode('&', $pairs);
    }
}
