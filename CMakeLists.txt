cmake_minimum_required (VERSION 4.3.0 FATAL_ERROR)

set (CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set (CMAKE_SUPPRESS_REGENERATION ON)
#set (CMAKE_SKIP_INSTALL_RULES ON)
set (CMAKE_VERBOSE_MAKEFILE OFF)
set (CMAKE_XCODE_GENERATE_SCHEME ON)
set (CMAKE_XCODE_ATTRIBUTE_SHOW_ENVIRONMENT_BUILD_SETTINGS NO)
set (CMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE NO)
set (CMAKE_CONFIGURATION_TYPES Debug Release Development)
set_property (GLOBAL PROPERTY DEBUG_CONFIGURATIONS "Debug")

if (WIN32)
	set (CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /Oy /GF /GS- /Gy /fp:fast /DNDEBUG /Zi")
	set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:REF /OPT:ICF /nodefaultlib:msvcrt")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
endif()

file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION)

project (
	ultraView
	VERSION ${VERSION}
	LANGUAGES CXX C
	HOMEPAGE_URL "https://ultrasid.com/")

Include (FetchContent)

include (CMakeDependentOption)

# Development = Release plus dev-only features (naked Data folder, inspector);
# custom configs start with empty flags, so inherit the Release set wholesale
set (CMAKE_C_FLAGS_DEVELOPMENT "${CMAKE_C_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_RELEASE}")
set (CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set (CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")

set_property (DIRECTORY APPEND PROPERTY LABELS ultraView)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ultraView)

set (CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_C_STANDARD 17)
set (CMAKE_C_STANDARD_REQUIRED ON)
set (CMAKE_C_EXTENSIONS OFF)
set (CMAKE_OBJCXX_STANDARD 23)
set (CMAKE_OBJCXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set (CMAKE_MINSIZEREL_POSTFIX -rm)
set (CMAKE_RELWITHDEBINFO_POSTFIX -rd)
set (CMAKE_OPTIMIZE_DEPENDENCIES OFF)

set (BUILD_SHARED_LIBS OFF)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)
set_property (GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER utility)
set_property (GLOBAL PROPERTY REPORT_UNDEFINED_PROPERTIES "${CMAKE_BINARY_DIR}/undefined_properties.log")

set_property (DIRECTORY APPEND PROPERTY LABELS External)

# JUCE
set (JUCE_MODULES_ONLY OFF)
set (JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)
set (JUCE_BUILD_EXTRAS OFF)
set (JUCE_BUILD_EXAMPLES OFF)

# Soft pin on develop: the fixes land there months before a tagged release.
# The patch step resets the patched file before each apply. Drop the patch
# and this hook once the fix has landed upstream.
FetchContent_Declare (JUCE
					GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
					GIT_TAG origin/develop
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/JUCE
					PATCH_COMMAND git checkout -- modules/juce_gui_basics/windows/juce_ComponentPeer.cpp
					COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/Tools/juce-handleFocusGain.patch)
FetchContent_MakeAvailable (JUCE)

set_property (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/JUCE" APPEND PROPERTY LABELS JUCE)

# Binary Data (icons)
if(WIN32)
	set (icon_path_big "${CMAKE_CURRENT_LIST_DIR}/icons/windows_big.png")
	set (icon_path_small "${CMAKE_CURRENT_LIST_DIR}/icons/windows_small.png")
elseif(APPLE)
	set (icon_path_big "${CMAKE_CURRENT_LIST_DIR}/icons/macos15_big.png")
	set (icon_path_small "${CMAKE_CURRENT_LIST_DIR}/icons/macos15_small.png")
endif()

# Gin modules
foreach(module_name IN ITEMS gin gin_dsp gin_simd gin_graphics gin_gui)
	juce_add_module ( "${CMAKE_CURRENT_LIST_DIR}/modules/Gin/modules/${module_name}" )
	set_property (TARGET "${module_name}" APPEND PROPERTY LABELS Gin)
endforeach()

# Lime modules
foreach ( module_name IN ITEMS lime_Logger lime_ShaderToyComponent lime_YamlConfig lime_Webcam lime_CRTEmulation )
	juce_add_module ( "${CMAKE_CURRENT_LIST_DIR}/modules/lime/modules/${module_name}" )
	set_property (TARGET "${module_name}" APPEND PROPERTY LABELS Lime)
endforeach()

# Melatonin Inspector: dev tooling, Development config only, and Windows only,
# the sole dev platform (Xcode cannot exclude an interface module per config)
if (WIN32)
	FetchContent_Declare (melatonin_inspector
						GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
						GIT_TAG origin/main
						GIT_SHALLOW TRUE
						SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector)
	FetchContent_MakeAvailable (melatonin_inspector)
endif ()

# Melatonin Blur
FetchContent_Declare (melatonin_blur
					GIT_REPOSITORY https://github.com/sudara/melatonin_blur.git
					GIT_TAG origin/main
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_blur)
FetchContent_MakeAvailable (melatonin_blur)

# fmt replaces std::format (features land overnight instead of every three
# years); hard pin on a release, bumped deliberately
set (FMT_INSTALL OFF)
FetchContent_Declare (fmt
					GIT_REPOSITORY https://github.com/fmtlib/fmt.git
					GIT_TAG 12.2.0
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/fmt)
FetchContent_MakeAvailable (fmt)

# boost::regex, standalone and header-only (BOOST_REGEX_STANDALONE): no other
# boost libs, nothing to build. SOURCE_SUBDIR points nowhere so its own CMake
# never runs; hard pin on a release, bumped deliberately
FetchContent_Declare (boost_regex
					GIT_REPOSITORY https://github.com/boostorg/regex.git
					GIT_TAG boost-1.88.0
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost_regex
					SOURCE_SUBDIR nonexistent)
FetchContent_MakeAvailable (boost_regex)
if (WIN32)
	# no C++20 modules - clang-cl has no /scanDependencies. The inherited
	# /fp:fast would strip the inf/nan handling fmt's float paths rely on
	set_target_properties (fmt PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_SCAN_FOR_MODULES OFF)
	target_compile_options (fmt PRIVATE /clang:-fno-finite-math-only)
endif ()

# Silence compiler warnings inside FetchContent dependencies — their code, their
# warnings. Skips INTERFACE targets (nothing is compiled for those).
if(MSVC OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  # /W0, not /w: the VS generator maps /W0 to the vcxproj WarningLevel element,
  # while /w lands in AdditionalOptions and collides with MSBuild's default
  # warning level (cl warning D9025)
  set(_dep_no_warnings /W0)
else()
  set(_dep_no_warnings -w)
endif()
foreach(_t fmt melatonin_inspector melatonin_blur)
  if(TARGET ${_t})
    get_target_property(_type ${_t} TYPE)
    if(NOT _type STREQUAL "INTERFACE_LIBRARY")
      get_target_property(_t_opts ${_t} COMPILE_OPTIONS)
      if(_t_opts)
        list(FILTER _t_opts EXCLUDE REGEX "^[-/][Ww]")
        set_target_properties(${_t} PROPERTIES COMPILE_OPTIONS "${_t_opts}")
      endif()
      target_compile_options(${_t} PRIVATE ${_dep_no_warnings})
    endif()
  endif()
endforeach()

# JUCE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

juce_add_gui_app (ultraView
				  PRODUCT_NAME ultraView
				  CAMERA_PERMISSION_ENABLED YES
				  CAMERA_PERMISSION_TEXT "User reflection"
				  VERSION "${ultraView_VERSION}"
  				  COMPANY_NAME "Michael Hartmann"
				  COMPANY_WEBSITE "https://ultrasid.com/"
				  ICON_BIG "${icon_path_big}"
				  ICON_SMALL "${icon_path_small}"
				  BUNDLE_ID com.toddlerboy.ultraview)

# JUCE's version-resource command only runs when the rc is missing (its only
# declared dependency is the icon), so a VERSION bump in an existing tree
# would keep shipping the old numbers; evict a stale rc so the build remakes it
if (WIN32)
	set (version_rc "${CMAKE_BINARY_DIR}/ultraView_artefacts/JuceLibraryCode/ultraView_resources.rc")
	if (EXISTS "${version_rc}" AND "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" IS_NEWER_THAN "${version_rc}")
		file (REMOVE "${version_rc}")
	endif ()
endif ()

file (GLOB_RECURSE source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)

target_sources (ultraView PRIVATE ${source_files})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX Source FILES ${source_files})

# The About screen's icon is the canonical app icon, synced from icons/ so it
# ships in the pak without a second committed copy drifting out of date
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/icons/windows_big.png
				${CMAKE_CURRENT_SOURCE_DIR}/Data/UI/png/about-icon.png COPYONLY)

target_include_directories (ultraView PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source ${boost_regex_SOURCE_DIR}/include)

target_link_libraries (ultraView PRIVATE

						gin
						gin_dsp
						gin_simd
						gin_graphics
						gin_gui

						lime_Logger
						lime_ShaderToyComponent
						lime_YamlConfig
						lime_Webcam
						lime_CRTEmulation

						juce::juce_audio_basics
						juce::juce_audio_devices
						juce::juce_audio_formats
						juce::juce_audio_plugin_client
						juce::juce_audio_processors
						juce::juce_audio_utils
						juce::juce_core
						juce::juce_cryptography
						juce::juce_data_structures
						juce::juce_dsp
						juce::juce_events
						juce::juce_graphics
						juce::juce_gui_basics
						juce::juce_gui_extra
						juce::juce_opengl

    					juce::juce_recommended_config_flags

						melatonin_blur

						fmt::fmt
					)

juce_generate_juce_header (ultraView)

target_compile_definitions (ultraView PRIVATE
								JUCE_COREGRAPHICS_DRAW_ASYNC=1
								JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING=1
								JUCE_MODAL_LOOPS_PERMITTED=0
								JUCE_WEB_BROWSER=0
								JUCE_USE_FLAC=0
								JUCE_USE_CURL=1
								JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
								JUCE_USE_MP3AUDIOFORMAT=0
								JUCE_USE_LAME_AUDIO_FORMAT=0
								JUCE_USE_WINDOWS_MEDIA_FORMAT=0
								JUCE_USE_CAMERA=0
								JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR=1
								DONT_SET_USING_JUCE_NAMESPACE=1
								_CRT_SECURE_NO_WARNINGS
								GIN_HAS_SIMD=0
								BOOST_REGEX_STANDALONE
								$<$<CONFIG:Development>:ULTRA_DEVELOPMENT=1>
								# Debug/Development read the naked repo Data folder, baked in
								# so the exe runs from anywhere, with the ultra-shared Data
								# tree as fallback root; Release is pak-only
								$<$<NOT:$<CONFIG:Release>>:ULTRA_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data">
								$<$<NOT:$<CONFIG:Release>>:ULTRA_SHARED_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Source/ultra-shared/Data">
							)

if (APPLE)
	set_target_properties(ultraView PROPERTIES
		XCODE_ATTRIBUTE_INFOPLIST_KEY_NSCameraUsageDescription "Add users reflection to CRT screen"
		XCODE_ATTRIBUTE_INFOPLIST_KEY_NSLocalNetworkUsageDescription "ultraView receives the video and audio stream from your C64 Ultimate over the local network"
		XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME OFF
		XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] ON
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] ON
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Development] ON)
	target_compile_options(ultraView PRIVATE -Wno-missing-braces -ffast-math -fno-finite-math-only -ffp-contract=fast -funsafe-math-optimizations)
	target_link_libraries(ultraView PUBLIC "-framework AVFoundation" "-framework CoreMedia" "-framework SystemConfiguration")

	# The whole data set ships inside the bundle as Resources/Data.pak, where
	# datasource reads it: images stored (they carry their own compression),
	# the rest weakest deflate. Release only (Debug/Development read the repo
	# Data folder); the non-Release commands degrade to argument-eating `true`.
	# Runs before any code-signing. Source-only files (bitmap PSDs, submodule
	# .git pointers) stay out. The ultra-shared Data tree packs first, so app
	# entries would win a name clash (matching the naked-mode precedence); it
	# is tiny and text-only, one weakest-deflate pass
	add_custom_command(TARGET ultraView POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/ultra-shared/Data
		COMMAND $<IF:$<CONFIG:Release>,rm,true> -rf $<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data $<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data.pak
		COMMAND $<IF:$<CONFIG:Release>,zip,true> -q -1 -r $<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data.pak . -x "*/!src/*" ".*" "*/.*" "*/Thumbs.db" "Thumbs.db"
		VERBATIM)
	add_custom_command(TARGET ultraView POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Data
		COMMAND $<IF:$<CONFIG:Release>,zip,true> -q -0 -r $<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data.pak . -i "*.png" "*.jpg" -x "*/!src/*" ".*" "*/.*"
		COMMAND $<IF:$<CONFIG:Release>,zip,true> -q -1 -r $<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data.pak . -x "*.png" "*.jpg" "*/!src/*" ".*" "*/.*" "*/Thumbs.db" "Thumbs.db"
		VERBATIM)
	# Completeness: every live file made it into the pak. A name clash between
	# the two trees would collapse into one entry and trip the count
	add_custom_command(TARGET ultraView POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND $<IF:$<CONFIG:Release>,sh,true> -c "expected=$(find Source/ultra-shared/Data Data -type f ! -path '*/!src/*' ! -path '*/.*' ! -name '.*' ! -name 'Thumbs.db' | wc -l); actual=$(zipinfo -1 '$<TARGET_BUNDLE_CONTENT_DIR:ultraView>/Resources/Data.pak' | grep -vc '/$'); [ \"$expected\" -eq \"$actual\" ] || { echo \"Data.pak entry count mismatch: $actual packed, $expected on disk\"; exit 1; }"
		VERBATIM)
endif()

if (WIN32)
	target_compile_options(ultraView PRIVATE -Wno-missing-braces /clang:-ffast-math /clang:-fno-finite-math-only /clang:-msse4.2 /clang:-mfma /clang:-ffp-contract=fast /clang:-funsafe-math-optimizations -showFilenames)
endif()

if (WIN32)
	set_target_properties(ultraView PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
	# The debugger starts in the repo root, where the naked dev Data lives
	set_target_properties(ultraView PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
	set_property(TARGET ultraView APPEND_STRING PROPERTY LINK_FLAGS_DEBUG " /INCREMENTAL:NO")
	set_target_properties(ultraView PROPERTIES LINK_FLAGS "/ignore:4099")
	target_link_libraries (ultraView PRIVATE mfplat mf mfuuid Mfreadwrite Shlwapi iphlpapi)
	# The inspector compiles and links into Development only; ULTRA_INSPECTOR
	# gates its include, member and key handler
	target_link_libraries (ultraView PRIVATE $<$<CONFIG:Development>:melatonin_inspector>)
	target_compile_definitions (ultraView PRIVATE $<$<CONFIG:Development>:ULTRA_INSPECTOR=1>)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries (ultraView PRIVATE curl)
	target_compile_options(ultraView PRIVATE $<$<CONFIG:Debug>:-g -O0>)
endif()

if (WIN32)
	install (TARGETS ultraView DESTINATION "Program Files")
elseif (APPLE)
	install (TARGETS ultraView DESTINATION "Applications")
elseif (UNIX)
	install (TARGETS ultraView DESTINATION bin)
	install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Data/"
			 DESTINATION share/ultraView
			 PATTERN "!src" EXCLUDE)

	set (CPACK_GENERATOR DEB)
	set (CPACK_PACKAGE_NAME "ultraview")
	set (CPACK_PACKAGE_VERSION "${ultraView_VERSION}")
	set (CPACK_PACKAGE_CONTACT "Michael Hartmann <info@ultrasid.com>")
	set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "C64 Ultimate stream viewer")
	set (CPACK_DEBIAN_PACKAGE_SECTION "sound")
	set (CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libjack-jackd2-0, libfreetype6, libcurl4, libx11-6, libxcomposite1, libxcursor1, libxext6, libxinerama1, libxrandr2, libxrender1")
	set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
	include (CPack)
endif ()
