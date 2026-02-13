#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "LinuxDiagnosticsService.h"
#include "LinuxClipboardService.h"
#include "LinuxFileSystemService.h"
#include "LinuxPathService.h"
#include "LinuxProcessService.h"

int main() {
	using namespace npp::platform;

	LinuxPathService pathService;
	LinuxFileSystemService fsService;
	LinuxProcessService processService;
	LinuxClipboardService clipboardService;
	LinuxDiagnosticsService diagnosticsService(pathService, fsService);

	auto config = pathService.GetPath(PathScope::kConfig);
	auto data = pathService.GetPath(PathScope::kData);
	auto cache = pathService.GetPath(PathScope::kCache);
	if (!config.ok() || !data.ok() || !cache.ok()) {
		std::cerr << "path lookups failed\n";
		return 1;
	}

	auto tempRoot = pathService.GetAppPath(PathScope::kTemp, "npp-linux-smoke");
	if (!tempRoot.ok()) {
		std::cerr << "temp app path failed: " << tempRoot.status.message << "\n";
		return 2;
	}
	if (!fsService.CreateDirectories(*tempRoot.value).ok()) {
		std::cerr << "create temp directory failed\n";
		return 3;
	}
	setenv("XDG_STATE_HOME", tempRoot.value->c_str(), 1);

	std::filesystem::path smokeFile = std::filesystem::path(*tempRoot.value) / "sample.txt";
	WriteFileOptions writeOptions;
	writeOptions.atomic = true;
	writeOptions.createParentDirs = true;
	if (!fsService.WriteTextFile(smokeFile.string(), "hello-linux", writeOptions).ok()) {
		std::cerr << "write file failed\n";
		return 4;
	}

	auto content = fsService.ReadTextFile(smokeFile.string());
	if (!content.ok() || *content.value != "hello-linux") {
		std::cerr << "read file failed: " << content.status.message << "\n";
		return 5;
	}

	auto crashDir = diagnosticsService.EnsureCrashDirectory("notepad-plus-plus-linux");
	if (!crashDir.ok()) {
		std::cerr << "ensure crash dir failed: " << crashDir.status.message << "\n";
		return 6;
	}
	if (!diagnosticsService.WriteDiagnostic(
			"notepad-plus-plus-linux", "crash", "smoke.log", "crash-diagnostic").ok()) {
		std::cerr << "write diagnostic failed\n";
		return 7;
	}

	ProcessSpec runSpec;
	runSpec.program = "/bin/true";
	auto runResult = processService.Run(runSpec, std::chrono::seconds(2));
	if (!runResult.ok() || runResult.value->exitCode != 0) {
		std::cerr << "run process failed\n";
		return 8;
	}

	ProcessSpec spawnSpec;
	spawnSpec.program = "/bin/sleep";
	spawnSpec.args = {"2"};
	auto child = processService.Spawn(spawnSpec);
	if (!child.ok()) {
		std::cerr << "spawn failed: " << child.status.message << "\n";
		return 9;
	}
	if (!processService.Terminate(*child.value).ok()) {
		std::cerr << "terminate failed\n";
		return 10;
	}

	if (!clipboardService.SupportsBuffer(ClipboardBuffer::kClipboard) ||
		!clipboardService.SupportsBuffer(ClipboardBuffer::kPrimarySelection)) {
		std::cerr << "clipboard buffer support mismatch\n";
		return 11;
	}
	if (!clipboardService.SetText("clip-text").ok()) {
		std::cerr << "set clipboard text failed\n";
		return 12;
	}
	auto clipText = clipboardService.GetText();
	if (!clipText.ok() || *clipText.value != "clip-text") {
		std::cerr << "get clipboard text failed\n";
		return 13;
	}
	if (!clipboardService.SetText("primary-text", ClipboardBuffer::kPrimarySelection).ok()) {
		std::cerr << "set primary selection text failed\n";
		return 14;
	}
	auto primaryText = clipboardService.GetText(ClipboardBuffer::kPrimarySelection);
	if (!primaryText.ok() || *primaryText.value != "primary-text") {
		std::cerr << "get primary selection text failed\n";
		return 15;
	}
	if (!clipboardService.Clear().ok() || !clipboardService.Clear(ClipboardBuffer::kPrimarySelection).ok()) {
		std::cerr << "clear clipboard buffers failed\n";
		return 16;
	}

	return 0;
}
