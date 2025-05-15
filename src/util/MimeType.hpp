#ifndef MIMETYPE_HPP
#define MIMETYPE_HPP

#include <string>
#include <unordered_map>
#include <filesystem>

class MimeType
{
public:
    static std::string getMimeType(const std::filesystem::path& path)
    {
        auto extension = path.extension().string();
        if (!s_MimeTypeMap.contains(extension))
            return "application/octet-stream";

        return s_MimeTypeMap[extension];
    };

private:
    inline static std::unordered_map<std::string, std::string> s_MimeTypeMap = {
        { ".aac", "audio/aac" },
        { ".abw", "application/x-abiword" },
        { ".apng", "image/apng" },
        { ".arc", "application/x-freearc" },
        { ".avif", "image/avif" },
        { ".avi", "video/x-msvideo" },
        { ".azw", "application/vnd.amazon.ebook" },
        { ".bin", "application/octet-stream" },
        { ".bz", "application/x-bzip" },
        { ".bz2", "application/x-bzip2" },
        { ".cda", "application/x-cdf" },
        { ".csh", "application/x-csh" },
        { ".css", "text/css" },
        { ".csv", "text/csv" },
        { ".doc", "application/msword" },
        { ".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
        { ".eot", "application/vnd.ms-fontobject" },
        { ".epub", "application/epub+zip" },
        { ".gz", "application/x-gzip" },
        { ".gif", "image/gif" },
        { ".htm", "text/html" },
        { ".html", "text/html" },
        { ".ico", "image/vnd.microsoft.icon" },
        { ".ics", "text/calendar" },
        { ".jar", "application/java-archive" },
        { ".jpeg", "image/jpeg" },
        { ".jpg", "image/jpeg" },
        { ".js", "text/javascript" },
        { ".json", "application/json" },
        { ".jsonld", "application/ld+json" },
        { ".md", "text/markdown" },
        { ".mid", "audio/x-midi" },
        { ".midi", "audio/x-midi" },
        { ".mjs", "text/javascript" },
        { ".mp3", "audio/mpeg" },
        { ".mp4", "video/mp4" },
        { ".mpeg", "video/mpeg" },
        { ".mpkg", "application/vnd.apple.installer+xml" },
        { ".odp", "application/vnd.oasis.opendocument.presentation" },
        { ".ods", "application/vnd.oasis.opendocument.spreadsheet" },
        { ".odt", "application/vnd.oasis.opendocument.text" },
        { ".oga", "audio/ogg" },
        { ".ogv", "video/ogg" },
        { ".ogx", "application/ogg" },
        { ".opus", "audio/ogg" },
        { ".otf", "font/otf" },
        { ".png", "image/png" },
        { ".pdf", "application/pdf" },
        { ".php", "application/x-httpd-php" },
        { ".ppt", "application/vnd.ms-powerpoint" },
        { ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
        { ".rar", "application/vnd.rar" },
        { ".rtf", "application/rtf" },
        { ".sh", "application/x-sh" },
        { ".svg", "image/svg+xml" },
        { ".tar", "application/x-tar" },
        { ".tif", "image/tiff" },
        { ".tiff", "image/tiff" },
        { ".ts", "video/mp2t" },
        { ".ttf", "font/ttf" },
        { ".txt", "text/plain" },
        { ".vsd", "application/vnd.visio" },
        { ".wav", "audio/wav" },
        { ".weba", "audio/webm" },
        { ".webm", "video/webm" },
        { ".webp", "image/webp" },
        { ".woff", "font/woff" },
        { ".woff2", "font/woff2" },
        { ".xhtml", "application/xhtml+xml" },
        { ".woff2", "application/vnd.ms-excel" },
        { ".xls", "application/vnd.ms-excel" },
        { ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
        { ".xml", "application/xml" },
        { ".xul", "application/vnd.mozilla.xul+xml" },
        { ".zip", "application/zip" },
        { ".7z", "application/x-7z-compressed" }
    };
};

#endif // !MIMETYPE_HPP
