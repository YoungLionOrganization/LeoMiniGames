.pragma library
function stringValue(v, fallback) { return (typeof v === "string" && v.length) ? v : (fallback || "") }
function numberValue(v, fallback) { var n=Number(v); return isFinite(n) ? n : (fallback === undefined ? 0 : fallback) }
function colorValue(v, fallback) { return (typeof v === "string" && v.length) ? v : (fallback || "#ac8f75") }
function urlValue(v) { return (typeof v === "string" && v.length) ? v : "" }
