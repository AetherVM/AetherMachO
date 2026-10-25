# AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
# Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
# SPDX-License-Identifier: APSL Version 2.0
# See APPLE_LICENSE file in the root directory for full license text.

enable_language(ASM)

add_library(MachOLoader STATIC
  MachOLoader.cpp
  common/AAREncoder.cpp
  common/CachePatching.cpp
  common/MetadataVisitor.cpp
  common/ObjCVisitor.cpp
  common/PropertyList.cpp
  common/ProcessAtlas.cpp
  common/SwiftVisitor.cpp
  dyld/RemoteNotificationResponder.cpp
  common/CRC32c.cpp
  common/Diagnostics.cpp
  common/DyldSharedCache.cpp
  common/FileManager.cpp
  common/MachOAnalyzer.cpp
  common/MachOFile.cpp
  common/MachOLayout.cpp
  common/MachOLoaded.cpp
  common/MurmurHash.cpp
  common/OptimizerSwift.cpp
  common/PerfectHash.cpp
  common/TargetPolicy.cpp
  common/Utilities.cpp
  dyld/DyldAPIs.cpp
  dyld/DyldDelegates.cpp
  dyld/DyldProcessConfig.cpp
  dyld/DyldRuntimeState.cpp
  dyld/JustInTimeLoader.cpp
  dyld/Loader.cpp
  dyld/PrebuiltLoader.cpp
  dyld/PrebuiltObjC.cpp
  dyld/PrebuiltSwift.cpp
  dyld/PremappedLoader.cpp
  dyld/Tracing.cpp
  dyld/dyldMain.cpp
  libdyld/LibSystemHelpers.cpp
  libdyld/ThreadLocalVariables.cpp
  libdyld/threadLocalHelpers.s
  lsl/Allocator.cpp
  lsl/PVLEInt64.cpp
  lsl/ProtectedStack.s
  mach_o/Architecture.cpp
  mach_o/Archive.cpp
  mach_o/BindOpcodes.cpp
  mach_o/ChainedFixups.cpp
  mach_o/CompactUnwind.cpp
  mach_o/DataInCode.cpp
  mach_o/DwarfDebug.cpp
  mach_o/Error.cpp
  mach_o/ExportsTrie.cpp
  mach_o/Fixups.cpp
  mach_o/FunctionStarts.cpp
  mach_o/FunctionVariants.cpp
  mach_o/GradedArchitectures.cpp
  mach_o/Image.cpp
  mach_o/Instructions.cpp
  mach_o/LazyLoadDylib.cpp
  mach_o/LoggingStub.cpp
  mach_o/Misc.cpp
  mach_o/NListSymbolTable.cpp
  mach_o/ObjC.cpp
  mach_o/Platform.cpp
  mach_o/Policy.cpp
  mach_o/RebaseOpcodes.cpp
  mach_o/SplitSeg.cpp
  mach_o/Symbol.cpp
  mach_o/Universal.cpp
  mach_o/UnsafeHeader.cpp
  mach_o/Version32.cpp
  mach_o/Version64.cpp
)

# The assembly sources use the C preprocessor.
set_source_files_properties(
  lsl/ProtectedStack.s
  libdyld/threadLocalHelpers.s
  PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp"
)

# Defines sThreadLocalVariables, and the thread exit handler that uses it.
set_source_files_properties(libdyld/ThreadLocalVariables.cpp
  PROPERTIES COMPILE_DEFINITIONS BUILDING_LIBDYLD=1
)

# Skips the lazy-load trampoline, which needs the full libdyld.
set_source_files_properties(libdyld/threadLocalHelpers.s
  PROPERTIES COMPILE_DEFINITIONS BUILDING_UNIT_TESTS=1
)

target_compile_definitions(MachOLoader PRIVATE
  TARGET_OS_EXCLAVEKIT=0
  BUILDING_DYLD=1
)

target_include_directories(MachOLoader PRIVATE
  cache_builder
  common
  dyld
  include
  include/mach-o
  libdyld
  lsl
  mach_o
  mach_o_writer
)
