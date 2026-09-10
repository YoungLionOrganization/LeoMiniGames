<?php
declare(strict_types=1);

final class ModRepository
{
    public function __construct(private readonly PDO $pdo)
    {
    }

    public function listPublished(string $query, string $category, string $sort, int $limit, int $offset): array
    {
        $where = ["status = 'published'"];
        $params = [];

        if ($query !== '') {
            $where[] = '(name LIKE :q OR description LIKE :q OR author LIKE :q OR id LIKE :q)';
            $params[':q'] = '%' . $query . '%';
        }
        if ($category !== '') {
            $where[] = 'category = :category';
            $params[':category'] = $category;
        }

        $order = match ($sort) {
            'downloads' => 'downloads DESC, updated_at DESC',
            'name' => 'name ASC',
            default => 'updated_at DESC',
        };

        $sql = 'SELECT * FROM mods WHERE ' . implode(' AND ', $where) .
            ' ORDER BY ' . $order . ' LIMIT :limit OFFSET :offset';
        $stmt = $this->pdo->prepare($sql);
        foreach ($params as $key => $value) {
            $stmt->bindValue($key, $value, PDO::PARAM_STR);
        }
        $stmt->bindValue(':limit', $limit, PDO::PARAM_INT);
        $stmt->bindValue(':offset', $offset, PDO::PARAM_INT);
        $stmt->execute();
        return $stmt->fetchAll();
    }

    public function countPublished(string $query, string $category): int
    {
        $where = ["status = 'published'"];
        $params = [];
        if ($query !== '') {
            $where[] = '(name LIKE :q OR description LIKE :q OR author LIKE :q OR id LIKE :q)';
            $params[':q'] = '%' . $query . '%';
        }
        if ($category !== '') {
            $where[] = 'category = :category';
            $params[':category'] = $category;
        }
        $stmt = $this->pdo->prepare('SELECT COUNT(*) FROM mods WHERE ' . implode(' AND ', $where));
        $stmt->execute($params);
        return (int)$stmt->fetchColumn();
    }

    public function findPublished(string $id): ?array
    {
        $stmt = $this->pdo->prepare("SELECT * FROM mods WHERE id = :id AND status = 'published' LIMIT 1");
        $stmt->execute([':id' => $id]);
        $row = $stmt->fetch();
        return $row ?: null;
    }

    public function incrementDownloads(string $id): void
    {
        $stmt = $this->pdo->prepare('UPDATE mods SET downloads = downloads + 1 WHERE id = :id');
        $stmt->execute([':id' => $id]);
    }

    public function upsert(array $mod): void
    {
        $sql = <<<'SQL'
INSERT INTO mods (
    id, name, description, author, version, category, icon,
    min_app_version, package_format, entry_path, object_key,
    sha256, size_bytes, status, published_at
) VALUES (
    :id, :name, :description, :author, :version, :category, :icon,
    :min_app_version, :package_format, :entry_path, :object_key,
    :sha256, :size_bytes, 'published', CURRENT_TIMESTAMP
)
ON DUPLICATE KEY UPDATE
    name = VALUES(name),
    description = VALUES(description),
    author = VALUES(author),
    version = VALUES(version),
    category = VALUES(category),
    icon = VALUES(icon),
    min_app_version = VALUES(min_app_version),
    package_format = VALUES(package_format),
    entry_path = VALUES(entry_path),
    object_key = VALUES(object_key),
    sha256 = VALUES(sha256),
    size_bytes = VALUES(size_bytes),
    status = 'published',
    published_at = CURRENT_TIMESTAMP
SQL;
        $stmt = $this->pdo->prepare($sql);
        $stmt->execute([
            ':id' => $mod['id'],
            ':name' => $mod['name'],
            ':description' => $mod['description'],
            ':author' => $mod['author'],
            ':version' => $mod['version'],
            ':category' => $mod['category'],
            ':icon' => $mod['icon'],
            ':min_app_version' => $mod['min_app_version'],
            ':package_format' => $mod['package_format'],
            ':entry_path' => $mod['entry'],
            ':object_key' => $mod['object_key'],
            ':sha256' => $mod['sha256'],
            ':size_bytes' => $mod['size_bytes'],
        ]);
    }

    public function disable(string $id): bool
    {
        $stmt = $this->pdo->prepare("UPDATE mods SET status = 'disabled' WHERE id = :id");
        $stmt->execute([':id' => $id]);
        return $stmt->rowCount() > 0;
    }
}
