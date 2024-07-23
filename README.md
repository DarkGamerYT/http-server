```cpp
#include "HttpServer.h"

HttpServer httpServer;
int main()
{
	httpServer.get("/", [](Request req, Response res) {
		res.send("Hello, world!");
	});

	httpServer.listen(3000);
};
```

## Building
- Install [CMake](https://cmake.org/download)
- Run `build.bat` to build this project.
  
If built successfully, a static library with the name `web-server.lib` should be found inside `build/Release` directory.
