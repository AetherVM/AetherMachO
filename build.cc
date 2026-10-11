// AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
// Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
// SPDX-License-Identifier: Apache License, Version 2.0
// See LICENSE file in the root directory for full license text.

// Usage: build.cc -os=<mac|ios> [-type=<Debug|Release>]

#include <icpp.hpp>

namespace {

constexpr int echo_command = 0;

template <typename... Args>
bool command(std::format_string<Args...> format, Args &&...args) {
  auto cmd = std::vformat(format.get(), std::make_format_args(args...));
  std::println("{}", cmd);
  return echo_command ? true : std::system(cmd.data()) == 0;
}

struct Builder {
  std::string_view type_ = "Release";
  std::string_view os_;
  fs::path proj_root_;
  fs::path build_root_;
  fs::path aebi_install_;
  fs::path aevm_install_;
  fs::path ios_toolchain_;

  Builder(int argc, const char *argv[]) {
    if (argc == 1)
      usage();

    for (int i = 1; i < argc; ++i) {
      std::string_view arg{argv[i]};
      if (arg.starts_with("-type="))
        type_ = arg.substr(6);
      else if (arg.starts_with("-os="))
        os_ = arg.substr(4);
    }

    if (os_ != "mac" && os_ != "ios") {
      std::println("The supported target os should be either mac or ios.");
      usage();
    }
    proj_root_ = fs::absolute(argv[0]).parent_path();
  }

  void usage() {
    std::println("Usage: build.cc -os=<mac|ios> [-type=<Debug|Release>]");
    std::exit(0);
  }

  bool prepare() {
    auto aebi_root = proj_root_.parent_path() / "AetherBinary";
    aebi_install_ = aebi_root / std::format("build-{}{}/install",
                                            os_ == "ios" ? "ios-" : "", type_);
    if (!fs::exists(aebi_install_)) {
      std::println(
          R"(The following path should exist, you can clone and build https://github.com/AetherVM/AetherBinary to generate it:
    {})",
          aebi_install_.string());
      return false;
    }

    if (os_ == "ios")
      ios_toolchain_ = aebi_root / "third/ios-cmake/ios.toolchain.cmake";

    auto aevm_root = proj_root_.parent_path() / "AetherVM";
    aevm_install_ = aevm_root / std::format("{}build-{}/install",
                                            os_ == "ios" ? "ios/" : "", type_);
    if (!fs::exists(aevm_install_)) {
      std::println(
          R"(The following path should exist, you can clone and build https://github.com/AetherVM/AetherVM to generate it:
    {})",
          aevm_install_.string());
      return false;
    }

    build_root_ = proj_root_ / std::format("build-{}-{}", os_, type_);
    return true;
  }

  bool init() {
    if (!prepare())
      return false;

    auto ninja = build_root_ / "build.ninja";
    if (fs::exists(ninja))
      return true;

    std::string args{"-G Ninja "};
    if (os_ == "ios") {
      args +=
          std::format("-DCMAKE_TOOLCHAIN_FILE={} -DCMAKE_CROSSCOMPILING=TRUE "
                      "-DCMAKE_MACOSX_BUNDLE=NO -DPLATFORM=OS64 "
                      "-DDEPLOYMENT_TARGET=16.5 -Wno-deprecated ",
                      ios_toolchain_.string());
    }
    return command("cmake -DCMAKE_PREFIX_PATH=\"{};{}\" -DCMAKE_BUILD_TYPE={} "
                   "-DCMAKE_INSTALL_PREFIX={}/install -S {} -B {} {}",
                   aebi_install_.string(), aevm_install_.string(), type_,
                   build_root_.string(), proj_root_.string(),
                   build_root_.string(), args);
  }

  bool build() {
    for (auto &action : {"build", "install"}) {
      if (!command("cmake --{} {}", action, build_root_.string()))
        return false;
    }
    return true;
  }
};

} // namespace

int main(int argc, const char *argv[]) {
  Builder builder{argc, argv};
  return builder.init() && builder.build();
}
