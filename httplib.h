#ifndef MINI_HTTPLIB_H
#define MINI_HTTPLIB_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> params;

    bool has_param(const std::string& key) const {
        return params.find(key) != params.end();
    }

    std::string get_param_value(const std::string& key) const {
        std::map<std::string, std::string>::const_iterator it = params.find(key);
        if (it == params.end()) return std::string();
        return it->second;
    }
};

struct Response {
    int status;
    std::string body;
    std::string content_type;

    Response() : status(200), content_type("text/plain") {}

    void set_content(const std::string& value, const char* type) {
        body = value;
        content_type = type ? type : "text/plain";
    }
};

class Server {
public:
    typedef std::function<void(const Request&, Response&)> Handler;

    Server() : mount_point_(""), running_(false) {}

    void Get(const std::string& path, Handler handler) {
        get_handlers_[path] = handler;
    }

    void Post(const std::string& path, Handler handler) {
        post_handlers_[path] = handler;
    }

    bool set_mount_point(const std::string& mount, const std::string& dir) {
        mount_point_ = mount;
        root_dir_ = dir;
        std::ifstream test((root_dir_ + "/index.html").c_str(), std::ios::binary);
        return test.good();
    }

    bool listen(const char* host, int port) {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

        SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));

        sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<unsigned short>(port));

        if (host == NULL || std::string(host) == "0.0.0.0") {
            address.sin_addr.s_addr = htonl(INADDR_ANY);
        } else if (address.sin_addr.s_addr = inet_addr(host),
                   address.sin_addr.s_addr == INADDR_NONE) {
            closesocket(server_socket);
            WSACleanup();
            return false;
        }

        if (bind(server_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
            closesocket(server_socket);
            WSACleanup();
            return false;
        }

        if (::listen(server_socket, 10) == SOCKET_ERROR) {
            closesocket(server_socket);
            WSACleanup();
            return false;
        }

        running_ = true;
        while (running_) {
            SOCKET client = accept(server_socket, NULL, NULL);
            if (client == INVALID_SOCKET) break;
            handle_client(client);
            closesocket(client);
        }

        closesocket(server_socket);
        WSACleanup();
        return true;
#else
        (void)host; (void)port;
        return false;
#endif
    }

private:
    std::map<std::string, Handler> get_handlers_;
    std::map<std::string, Handler> post_handlers_;
    std::string mount_point_;
    std::string root_dir_;
    bool running_;

    static std::string url_decode(const std::string& input) {
        std::string out;
        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '%' && i + 2 < input.size()) {
                char hex[3] = { input[i + 1], input[i + 2], '\0' };
                char* end = NULL;
                long value = std::strtol(hex, &end, 16);
                if (end && *end == '\0') {
                    out.push_back(static_cast<char>(value));
                    i += 2;
                } else out.push_back(input[i]);
            } else if (input[i] == '+') out.push_back(' ');
            else out.push_back(input[i]);
        }
        return out;
    }

    static void parse_params(const std::string& query, Request& req) {
        size_t start = 0;
        while (start < query.size()) {
            size_t amp = query.find('&', start);
            if (amp == std::string::npos) amp = query.size();
            std::string pair = query.substr(start, amp - start);
            size_t eq = pair.find('=');
            if (eq == std::string::npos) {
                req.params[url_decode(pair)] = "";
            } else {
                req.params[url_decode(pair.substr(0, eq))] = url_decode(pair.substr(eq + 1));
            }
            start = amp + 1;
        }
    }

    static std::string mime_type(const std::string& path) {
        size_t dot = path.find_last_of('.');
        if (dot == std::string::npos) return "application/octet-stream";
        std::string ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](char c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        });
        if (ext == "html" || ext == "htm") return "text/html; charset=UTF-8";
        if (ext == "css") return "text/css; charset=UTF-8";
        if (ext == "js") return "application/javascript; charset=UTF-8";
        if (ext == "json") return "application/json; charset=UTF-8";
        if (ext == "png") return "image/png";
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "svg") return "image/svg+xml";
        if (ext == "ico") return "image/x-icon";
        return "application/octet-stream";
    }

    static std::string status_text(int status) {
        if (status == 200) return "OK";
        if (status == 400) return "Bad Request";
        if (status == 404) return "Not Found";
        if (status == 500) return "Internal Server Error";
        return "Error";
    }

    static bool read_file(const std::string& path, std::string& data) {
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) return false;
        std::ostringstream buffer;
        buffer << file.rdbuf();
        data = buffer.str();
        return true;
    }

    void send_response(SOCKET client, const Response& res) {
        std::ostringstream out;
        out << "HTTP/1.1 " << res.status << " " << status_text(res.status) << "\r\n";
        out << "Content-Type: " << res.content_type << "\r\n";
        out << "Content-Length: " << res.body.size() << "\r\n";
        out << "Connection: close\r\n";
        out << "Access-Control-Allow-Origin: *\r\n";
        out << "\r\n";
        out << res.body;
        std::string packet = out.str();
        const char* data = packet.c_str();
        int remaining = static_cast<int>(packet.size());
        while (remaining > 0) {
            int sent = send(client, data, remaining, 0);
            if (sent <= 0) break;
            data += sent;
            remaining -= sent;
        }
    }

    void handle_client(SOCKET client) {
        std::string request_data;
        char buffer[8192];
        int received;

        while (request_data.find("\r\n\r\n") == std::string::npos && request_data.size() < 1024 * 1024) {
            received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0) return;
            request_data.append(buffer, received);
        }

        size_t header_end = request_data.find("\r\n\r\n");
        if (header_end == std::string::npos) return;

        std::string headers = request_data.substr(0, header_end);
        std::string body = request_data.substr(header_end + 4);

        std::istringstream lines(headers);
        std::string request_line;
        std::getline(lines, request_line);
        if (!request_line.empty() && request_line[request_line.size() - 1] == '\r') request_line.erase(request_line.size() - 1);

        std::istringstream first(request_line);
        Request req;
        first >> req.method;
        std::string target;
        first >> target;

        size_t qpos = target.find('?');
        std::string query;
        if (qpos == std::string::npos) req.path = target;
        else {
            req.path = target.substr(0, qpos);
            query = target.substr(qpos + 1);
        }
        parse_params(query, req);

        size_t content_length = 0;
        std::string header_line;
        while (std::getline(lines, header_line)) {
            if (!header_line.empty() && header_line[header_line.size() - 1] == '\r') header_line.erase(header_line.size() - 1);
            std::string lower = header_line;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](char c) {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            });
            if (lower.find("content-length:") == 0) {
                std::string value = header_line.substr(header_line.find(':') + 1);
                content_length = static_cast<size_t>(std::strtoul(value.c_str(), NULL, 10));
            }
        }

        while (body.size() < content_length && body.size() < 1024 * 1024) {
            received = recv(client, buffer, sizeof(buffer), 0);
            if (received <= 0) break;
            body.append(buffer, received);
        }
        req.body = body;

        Response res;
        Handler handler;

        if (req.method == "GET") {
            std::map<std::string, Handler>::iterator it = get_handlers_.find(req.path);
            if (it != get_handlers_.end()) handler = it->second;
        } else if (req.method == "POST") {
            std::map<std::string, Handler>::iterator it = post_handlers_.find(req.path);
            if (it != post_handlers_.end()) handler = it->second;
        }

        if (handler) {
            handler(req, res);
            send_response(client, res);
            return;
        }

        if (req.method == "GET" && !root_dir_.empty()) {
            std::string relative = req.path;
            if (relative.empty() || relative == "/") relative = "/index.html";
            if (relative.find("..") != std::string::npos) {
                res.status = 400;
                res.set_content("Bad path", "text/plain; charset=UTF-8");
                send_response(client, res);
                return;
            }
            std::string full = root_dir_ + relative;
            std::string file_data;
            if (read_file(full, file_data)) {
                res.set_content(file_data, mime_type(full).c_str());
            } else {
                res.status = 404;
                res.set_content("404 - File not found", "text/plain; charset=UTF-8");
            }
            send_response(client, res);
            return;
        }

        res.status = 404;
        res.set_content("404 - Endpoint not found", "text/plain; charset=UTF-8");
        send_response(client, res);
    }
};

} // namespace httplib

#endif
