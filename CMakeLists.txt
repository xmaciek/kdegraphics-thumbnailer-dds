cmake_minimum_required( VERSION 3.24 )
project( kdegraphics-thumbnailer-dds )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( QT_MIN_VERSION "6.7.0" )
set( KF_MIN_VERSION "6.1.0" )

set( KDE_COMPILERSETTINGS_LEVEL "5.82" )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
find_package( ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE )
set( CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include( KDEInstallDirs )
include( KDECMakeSettings )
include( KDECompilerSettings NO_POLICY_SCOPE )

find_package( Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui )
find_package( KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS KIO )
find_package( KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS Archive )
add_definitions( -DQT_USE_QSTRINGBUILDER )

kcoreaddons_add_plugin( ddsthumbnail INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/thumbcreator" )

target_sources( ddsthumbnail PRIVATE
    ddsthumbnail.cpp
    bc7.hpp
)

target_compile_options( ddsthumbnail PRIVATE
    -Wno-multichar
)

target_link_libraries( ddsthumbnail
    KF${QT_MAJOR_VERSION}::KIOGui
    Qt::Gui
)

add_custom_target( nuke COMMAND rm -rv "$ENV{HOME}/.cache/thumbnails/*" )

install( TARGETS ddsthumbnail DESTINATION ${KDE_INSTALL_PLUGINDIR} )
