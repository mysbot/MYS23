## GitHub Copilot Chat

- Extension Version: 0.24.0 (prod)
- VS Code: vscode/1.97.0
- OS: Windows

## Network

User Settings:
```json
  "github.copilot.advanced.debug.useElectronFetcher": true,
  "github.copilot.advanced.debug.useNodeFetcher": false,
  "github.copilot.advanced.debug.useNodeFetchFetcher": true
```

Environment Variables:
- NO_PROXY=127.0.0.1

Connecting to https://api.github.com:
- DNS ipv4 Lookup: 20.205.243.168 (93 ms)
- DNS ipv6 Lookup: Error (43 ms): getaddrinfo ENOTFOUND api.github.com
- Proxy URL: http://127.0.0.1:7878 (5 ms)
- Proxy Connection: 200 Connection established (1 ms)
- Electron fetch (configured): HTTP 403 (900 ms)
- Node.js https: HTTP 403 (800 ms)
- Node.js fetch: HTTP 403 (807 ms)
- Helix fetch: HTTP 200 (714 ms)

Connecting to https://api.individual.githubcopilot.com/_ping:
- DNS ipv4 Lookup: 140.82.113.21 (46 ms)
- DNS ipv6 Lookup: Error (42 ms): getaddrinfo ENOTFOUND api.individual.githubcopilot.com
- Proxy URL: http://127.0.0.1:7878 (9 ms)
- Proxy Connection: 200 Connection established (1 ms)
- Electron fetch (configured): HTTP 200 (1289 ms)
- Node.js https: HTTP 200 (1229 ms)
- Node.js fetch: HTTP 200 (1238 ms)
- Helix fetch: HTTP 200 (938 ms)

## Documentation

In corporate networks: [Troubleshooting firewall settings for GitHub Copilot](https://docs.github.com/en/copilot/troubleshooting-github-copilot/troubleshooting-firewall-settings-for-github-copilot).