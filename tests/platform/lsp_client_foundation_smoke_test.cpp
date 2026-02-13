#include <iostream>
#include <string>

#include "LinuxLspClientService.h"
#include "LinuxProcessService.h"

int main() {
    using namespace npp::platform;

    LinuxProcessService processService;
    LinuxLspClientService lspService(&processService);

    LspServerConfig server;
    server.languageId = "python";
    server.command = "/bin/sleep";
    server.args = {"15"};
    const Status registerStatus = lspService.RegisterServer(server);
    if (!registerStatus.ok()) {
        std::cerr << "register LSP server failed: " << registerStatus.message << "\n";
        return 1;
    }

    LspSessionSpec sessionSpec;
    sessionSpec.languageId = "python";
    sessionSpec.workspacePathUtf8 = "/tmp";
    sessionSpec.documentPathUtf8 = "/tmp/sample.py";

    const auto session = lspService.StartSession(sessionSpec);
    if (!session.ok()) {
        std::cerr << "start LSP session failed: " << session.status.message << "\n";
        return 2;
    }

    if (lspService.ActiveSessionCount() != 1 || !lspService.IsSessionActive(*session.value)) {
        std::cerr << "unexpected LSP session state after start\n";
        return 3;
    }

    const Status stopStatus = lspService.StopSession(*session.value);
    if (!stopStatus.ok()) {
        std::cerr << "stop LSP session failed: " << stopStatus.message << "\n";
        return 4;
    }

    if (lspService.ActiveSessionCount() != 0 || lspService.IsSessionActive(*session.value)) {
        std::cerr << "unexpected LSP session state after stop\n";
        return 5;
    }

    LspSessionSpec missingSpec;
    missingSpec.languageId = "javascript";
    missingSpec.workspacePathUtf8 = "/tmp";
    missingSpec.documentPathUtf8 = "/tmp/a.js";
    const auto missingSession = lspService.StartSession(missingSpec);
    if (missingSession.ok()) {
        std::cerr << "start session for unknown language should fail\n";
        return 6;
    }

    const Status stopAllStatus = lspService.StopAllSessions();
    if (!stopAllStatus.ok()) {
        std::cerr << "stop all LSP sessions failed: " << stopAllStatus.message << "\n";
        return 7;
    }

    return 0;
}
