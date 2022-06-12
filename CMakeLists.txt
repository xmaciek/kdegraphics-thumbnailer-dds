cmake_minimum_required( VERSION 3.16 )
project( kdegraphics-thumbnailer-dds )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( QT_MIN_VERSION "5.15.0" )
set( KF5_MIN_VERSION "5.90.0" )
set( KDE_COMPILERSETTINGS_LEVEL "5.82" )
find_package( ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE )
set( CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include( ECMOptionalAddSubdirectory )
include( KDEInstallDirs )
include( KDECMakeSettings )
include( KDECompilerSettings NO_POLICY_SCOPE )
include( FeatureSummary )

find_package( Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui )
find_package( KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS KIO )
add_definitions( -DQT_USE_QSTRINGBUILDER )

add_library( ddsthumbnail MODULE )

target_sources( ddsthumbnail PRIVATE
    ddsthumbnail.cpp
)

target_link_libraries( ddsthumbnail
    KF5::KIOWidgets
    Qt::Gui
)

install( TARGETS ddsthumbnail DESTINATION ${KDE_INSTALL_PLUGINDIR} )
#install( TARGETS ddsthumbnail DESTINATION "/usr/lib/qt/plugins/" )
install( FILES ddsthumbnail.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR} )
