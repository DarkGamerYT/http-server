<div align="center">

  # http-server
  A simple HTTP server made using C++.
</div>

## Library Usage Examples
Refer to [src-examples](./src-examples) for more examples
```cpp
#include "HttpServer.hpp"

HttpServer g_Server;
int main(int argc, char* argv[])
{
    g_Server.use("/", HttpMethod::GET, [](HttpRequest request, HttpResponse& response, const NextFn& next) {
        response.setHeader("Content-Type", "text/plain");
        response.send("Hello, world!");
    });

    g_Server.listen(8080);
};
```

<img src=".github/img/http.screenshot.png" />

## Building
- Install [CMake](https://cmake.org/download)
- Run `build.bat`, or `build.sh` to build this project.
  
If built successfully, a static library with the name `web-server.lib` should be found inside `build/Release` directory.

## Features
- [x] WebSockets
- [ ] Caching
- [ ] SSL Support

## License
This project is open-source under the **MIT License**. Feel free to modify and contribute!  

## Contributing
```shell
# Fork the repository
git clone https://github.com/DarkGamerYT/http-server.git

# Create a new branch
git checkout -b feature-branch

# Commit your changes
git commit -m "Added new feature"

# Push to GitHub
git push origin feature-branch

# Submit a Pull Request
```
