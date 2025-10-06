# MultiChat

# Todo
- Fix formWin so it doesn't need a constructor
- Fix control flow so networkStart() creates an error so chat can redirect

- Fix switchToChat so it doesn't need returns under it. I.e. follows normal flow

- Fix Signal handler for kill server (have it read in epoll loop then die)
- Fix random segfault when server reads malformed packet from log
