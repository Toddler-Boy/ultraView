cmake_minimum_required (VERSION 4.3.0 FATAL_ERROR)

set (CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set (CMAKE_SUPPRESS_REGENERATION ON)
#set (CMAKE_SKIP_INSTALL_RULES ON)
set (CMAKE_VERBOSE_MAKEFILE OFF)
set (CMAKE_XCODE_GENERATE_SCHEME ON)
set (CMAKE_XCODE_ATTRIBUTE_SHOW_ENVIRONMENT_BUILD_SETTINGS NO)
set (CMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE NO)
set (CMAKE_CONFIGURATION_TYPES Debug Release)
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

set_property (DIRECTORY APPEND PROPERTY LABELS ultraView)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ultraView)

set (CMAKE_OSX_DEPLOYMENT_TARGET 14.0)

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

add_subdirectory (modules/JUCE)

set_property (DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/modules/JUCE" APPEND PROPERTY LABELS JUCE)

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

# Melatonin Inspector
FetchContent_Declare (melatonin_inspector
					GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
					GIT_TAG origin/main
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector)
FetchContent_MakeAvailable (melatonin_inspector)

# Melatonin Blur
FetchContent_Declare (melatonin_blur
					GIT_REPOSITORY https://github.com/sudara/melatonin_blur.git
					GIT_TAG origin/main
					GIT_SHALLOW TRUE
					SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_blur)
FetchContent_MakeAvailable (melatonin_blur)

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

file (GLOB_RECURSE source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/Source/*.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)

target_sources (ultraView PRIVATE ${source_files})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX Source FILES ${source_files})

target_include_directories (ultraView PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

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

						melatonin_inspector
						melatonin_blur
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
							)

if (APPLE)
	set_target_properties(ultraView PROPERTIES
		XCODE_ATTRIBUTE_INFOPLIST_KEY_NSCameraUsageDescription "Add users face to CRT screen"
		XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME OFF
		XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] ON
		XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] ON)
	target_compile_options(ultraView PRIVATE -Wno-missing-braces -ffast-math -fno-finite-math-only -ffp-contract=fast -funsafe-math-optimizations)
	target_link_libraries(ultraView PUBLIC "-framework AVFoundation" "-framework CoreMedia")
endif()

if (WIN32)
	target_compile_options(ultraView PRIVATE -Wno-missing-braces /clang:-ffast-math /clang:-fno-finite-math-only /clang:-msse4.2 /clang:-mfma /clang:-ffp-contract=fast /clang:-funsafe-math-optimizations -showFilenames)
endif()

if (WIN32)
	set_target_properties(ultraView PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
	set_property(TARGET ultraView APPEND_STRING PROPERTY LINK_FLAGS_DEBUG " /INCREMENTAL:NO")
	set_target_properties(ultraView PROPERTIES LINK_FLAGS "/ignore:4099")
	target_link_libraries (ultraView PRIVATE mfplat mf mfuuid Mfreadwrite Shlwapi iphlpapi)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries (ultraView PRIVATE curl)
	target_compile_options(ultraView PRIVATE $<$<CONFIG:Debug>:-g -O0>)
endif()

if(WIN32)
	set (dest "Program Files")
else()
	set (dest "Applications")
endif()

install (TARGETS ultraView DESTINATION "${dest}")
